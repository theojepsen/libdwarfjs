#include "DwarfDbg.h"
#include "common.h"
#include <unistd.h>

namespace libdwarfjs {

Nan::Persistent<v8::Function> DwarfDbg::constructor;

DwarfDbg::DwarfDbg(v8::Local<v8::String> f) : dbg(0), filename(f), prog_fd(-1) {
  if ((prog_fd = open(*v8::String::Utf8Value(f), O_RDONLY)) < 0) {
    throwErr("open");
    return;
  }

  if (dwarf_init(prog_fd, DW_DLC_READ, 0, 0, &dbg, &err) != DW_DLV_OK) {
    throwDwarf("Failed DWARF initialization", err);
  }
}


void DwarfDbg::getCUHeaders(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Nan::EscapableHandleScope scope;
  DwarfDbg* obj = ObjectWrap::Unwrap<DwarfDbg>(info.Holder());

  Dwarf_Half cu_version;
  Dwarf_Off this_cu_offset, next_cu_offset;
  int rc;

  // Iterate through CU Headers
  v8::Local<v8::Array> cu_arr = Nan::New<v8::Array>();
  int i = 0;
  this_cu_offset = 0;
  while (1) {
    rc = dwarf_next_cu_header_b(obj->dbg, NULL, &cu_version, NULL,
        NULL, NULL, NULL, &next_cu_offset, &obj->err);

    if (rc == DW_DLV_ERROR)
      return throwDwarf("Error reading DWARF cu header", obj->err);
    if (rc == DW_DLV_NO_ENTRY)
      break;

    cu_arr->Set(i++, DwarfCUHeader::NewInstance(info.Holder(), (uint64_t)this_cu_offset, (uint16_t)cu_version));
    this_cu_offset = next_cu_offset;
  }

  info.GetReturnValue().Set(cu_arr);
}

DwarfDbg::~DwarfDbg() {
    if (dwarf_finish(dbg, &err) != DW_DLV_OK) {
      throwDwarf("Failed DWARF finalization", err);
    }
    close(prog_fd);
}

void DwarfDbg::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DwarfDbg").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "getCUHeaders", getCUHeaders);
  Nan::SetPrototypeMethod(tpl, "getLocList", getLocList);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("DwarfDbg").ToLocalChecked(), tpl->GetFunction());
}

void DwarfDbg::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.IsConstructCall()) {
    // Invoked as constructor: `new DwarfDbg(...)`
    DwarfDbg* obj = new DwarfDbg(info[0]->ToString());
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function `DwarfDbg(...)`, turn into construct call.
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = { info[0] };
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

void DwarfDbg::getLocList(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfDbg* obj = ObjectWrap::Unwrap<DwarfDbg>(info.Holder());
  int rc;
  Dwarf_Unsigned off, listOff, len, next;
  Dwarf_Addr hipc, lopc;
  Dwarf_Ptr data;
  Dwarf_Locdesc *llbuf;
  Dwarf_Signed lcnt;

  v8::Local<v8::Array> locdescs = Nan::New<v8::Array>();

  off = 0;
  listOff = 0; // offset of a whole list
  int i = 0;
  while ((rc = dwarf_get_loclist_entry(obj->dbg, off, &hipc, &lopc,
      &data, &len, &next, &obj->err)) == DW_DLV_OK) {
    if (lopc == 0 && hipc == 0) {
      listOff = next;
    }
    else {
      rc = dwarf_loclist_from_expr(obj->dbg, data,
          len, &llbuf, &lcnt, &obj->err);
      v8::Local<v8::Array> l = parseLocList(obj->dbg, &obj->err, llbuf, lcnt);
      v8::Local<v8::Object> loc = Nan::New<v8::Object>();
      loc->Set(Nan::New("offset").ToLocalChecked(), Nan::New<v8::Number>(listOff));
      loc->Set(Nan::New("begin").ToLocalChecked(), Nan::New<v8::Number>(lopc));
      loc->Set(Nan::New("end").ToLocalChecked(), Nan::New<v8::Number>(hipc));
      loc->Set(Nan::New("expr").ToLocalChecked(), l->Get(0));
      locdescs->Set(i++, loc);
    }
    off = next;
  }
  if (rc == DW_DLV_ERROR)
    return throwDwarf("dwarf_get_loclist_entry", obj->err);

  info.GetReturnValue().Set(locdescs);
}

v8::Local<v8::Object> DwarfDbg::loadDIE(v8::Local<v8::Object> cuObj, uint64_t offset) {
  if (dieMap.find(offset) == dieMap.end()) {
    v8::Local<v8::Object> dieObj = DwarfDIE::NewInstance(cuObj, offset);
    dieMap[offset] = Nan::ObjectWrap::Unwrap<DwarfDIE>(dieObj);
  }
  return dieMap[offset]->handle();
}

} // namespace libdwarfjs
