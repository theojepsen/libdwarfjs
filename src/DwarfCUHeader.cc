#include "DwarfCUHeader.h"
#include "DwarfDIE.h"
#include "common.h"

namespace libdwarfjs {

Nan::Persistent<v8::FunctionTemplate> DwarfCUHeader::constructor_template;

DwarfCUHeader::DwarfCUHeader() : dwarfDbg(NULL), dwversion(0), headerOffset(0) {
}

DwarfCUHeader::~DwarfCUHeader() {
}

void DwarfCUHeader::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DwarfCUHeader").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "getHeaderOffset", getHeaderOffset);
  Nan::SetPrototypeMethod(tpl, "getDIE", getDIE);

  constructor_template.Reset(tpl);
  exports->Set(Nan::New("DwarfCUHeader").ToLocalChecked(), tpl->GetFunction());
}

void DwarfCUHeader::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.IsConstructCall()) {
    DwarfCUHeader* obj = new DwarfCUHeader();
    obj->dwarfDbgObj = info[0]->ToObject();
    obj->dwarfDbg = Nan::ObjectWrap::Unwrap<DwarfDbg>(info[0]->ToObject());
    obj->headerOffset = info[1]->NumberValue();
    obj->dwversion = info[2]->NumberValue();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Value> argv[3] = { info[0], info[1], info[2] };
    v8::Local<v8::Function> cons = Nan::New(constructor_template)->GetFunction();
    info.GetReturnValue().Set(cons->NewInstance(3, argv));
  }
}

v8::Local<v8::Object> DwarfCUHeader::NewInstance(v8::Local<v8::Object> ddbg, uint64_t headerOffset, Dwarf_Half version) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Value> argv[3] = { ddbg, Nan::New<v8::Number>(headerOffset), Nan::New<v8::Number>(version) };
    v8::Local<v8::Function> cons = Nan::New(constructor_template)->GetFunction();
    return scope.Escape(cons->NewInstance(3, argv));
}

void DwarfCUHeader::getHeaderOffset(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfCUHeader* obj = ObjectWrap::Unwrap<DwarfCUHeader>(info.Holder());
  info.GetReturnValue().Set(Nan::New<v8::Number>(obj->headerOffset));
}

void DwarfCUHeader::getDIE(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfCUHeader* obj = ObjectWrap::Unwrap<DwarfCUHeader>(info.Holder());
  Dwarf_Die cu_die;
  Dwarf_Off cu_die_offset;

  if (dwarf_get_cu_die_offset_given_cu_header_offset(obj->dwarfDbg->dbg,
        obj->headerOffset, &cu_die_offset, &obj->dwarfDbg->err) != DW_DLV_OK)
    return throwDwarf("Failed to get CU DIE offset", obj->dwarfDbg->err);
  if (dwarf_offdie(obj->dwarfDbg->dbg, cu_die_offset,
        &cu_die, &obj->dwarfDbg->err) != DW_DLV_OK)
    return throwDwarf("Failed to get CU DIE", obj->dwarfDbg->err);

  dwarf_dealloc(obj->dwarfDbg->dbg, cu_die, DW_DLA_DIE);
  info.GetReturnValue().Set(obj->dwarfDbg->loadDIE(info.Holder(), (uint64_t)cu_die_offset));
}

} // namespace libdwarfjs
