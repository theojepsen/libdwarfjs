#include "DwarfDIE.h"
#include "common.h"

namespace libdwarfjs {

Nan::Persistent<v8::FunctionTemplate> DwarfDIE::constructor_template;

DwarfDIE::DwarfDIE() : offset(0), dwarfDbg(NULL) {
}

DwarfDIE::~DwarfDIE() {
  dwarf_dealloc(dwarfDbg->dbg, die, DW_DLA_DIE);
}

void DwarfDIE::Init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DwarfDIE").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  Nan::SetPrototypeMethod(tpl, "getOffset", getOffset);
  Nan::SetPrototypeMethod(tpl, "getName", getName);
  Nan::SetPrototypeMethod(tpl, "getTagName", getTagName);
  Nan::SetPrototypeMethod(tpl, "getAttributes", getAttributes);
  Nan::SetPrototypeMethod(tpl, "getChildren", getChildren);
  Nan::SetPrototypeMethod(tpl, "getPCRange", getPCRange);

  constructor_template.Reset(tpl);
  exports->Set(Nan::New("DwarfDIE").ToLocalChecked(), tpl->GetFunction());
}

void DwarfDIE::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.IsConstructCall()) {
    DwarfDIE* obj = new DwarfDIE();
    obj->cu = Nan::ObjectWrap::Unwrap<DwarfCUHeader>(info[0]->ToObject());
    obj->dwarfDbg = obj->cu->dwarfDbg;
    obj->offset = info[1]->NumberValue();
    obj->loadDie();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    v8::Local<v8::Value> argv[2] = { info[0], info[1] };
    v8::Local<v8::Function> cons = Nan::New(constructor_template)->GetFunction();
    info.GetReturnValue().Set(cons->NewInstance(2, argv));
  }
}

v8::Local<v8::Object> DwarfDIE::NewInstance(v8::Local<v8::Object> cuObj, uint64_t offset) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Value> argv[2] = { cuObj, Nan::New<v8::Number>(offset) };
    v8::Local<v8::Function> cons = Nan::New(constructor_template)->GetFunction();
    return scope.Escape(cons->NewInstance(2, argv));
}

void DwarfDIE::getCUHeader(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfDIE* obj = ObjectWrap::Unwrap<DwarfDIE>(info.Holder());
  info.GetReturnValue().Set(obj->cu->handle());
}

void DwarfDIE::getOffset(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfDIE* obj = ObjectWrap::Unwrap<DwarfDIE>(info.Holder());
  info.GetReturnValue().Set(Nan::New<v8::Number>(obj->offset));
}

void DwarfDIE::getChildren(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Dwarf_Off child_die_offset;
  Dwarf_Die next_child_die, child_die;
  v8::Local<v8::Array> children = Nan::New<v8::Array>();
  DwarfDIE* obj = ObjectWrap::Unwrap<DwarfDIE>(info.Holder());

  int rc = dwarf_child(obj->die, &child_die, &obj->dwarfDbg->err);
  if (rc == DW_DLV_ERROR) {
    dwarf_dealloc(obj->dwarfDbg->dbg, child_die, DW_DLA_DIE);
    return throwDwarf("Failed to get first child die of CU DIE", obj->dwarfDbg->err);
  }
  else if (rc == DW_DLV_OK) {
    int i = 0;
    while (1) {
      rc = dwarf_dieoffset(child_die, &child_die_offset, &obj->dwarfDbg->err);
      if (rc == DW_DLV_ERROR) {
        dwarf_dealloc(obj->dwarfDbg->dbg, child_die, DW_DLA_DIE);
        return throwDwarf("child_die_offset", obj->dwarfDbg->err);
      }
      children->Set(i++, obj->dwarfDbg->loadDIE(obj->cu->handle(), (uint64_t)child_die_offset));


      rc = dwarf_siblingof(obj->dwarfDbg->dbg, child_die, &next_child_die, &obj->dwarfDbg->err);
      dwarf_dealloc(obj->dwarfDbg->dbg, child_die, DW_DLA_DIE);
      if (rc == DW_DLV_ERROR)
        return throwDwarf("Failed to get sibling of child die)", obj->dwarfDbg->err);
      else if (rc == DW_DLV_NO_ENTRY)
        break;

      child_die = next_child_die;
    }
  }

  info.GetReturnValue().Set(children);
}

void DwarfDIE::getName(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfDIE* obj = ObjectWrap::Unwrap<DwarfDIE>(info.Holder());
  char* die_name = 0;
  int rc = dwarf_diename(obj->die, &die_name, &obj->dwarfDbg->err);
  if (rc == DW_DLV_ERROR)
    return throwDwarf("Error in dwarf_diename", obj->dwarfDbg->err);
  if (rc == DW_DLV_OK)
    info.GetReturnValue().Set(Nan::New(die_name).ToLocalChecked());
}

void DwarfDIE::getTagName(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfDIE* obj = ObjectWrap::Unwrap<DwarfDIE>(info.Holder());
  Dwarf_Half tag;
  if (dwarf_tag(obj->die, &tag, &obj->dwarfDbg->err) != DW_DLV_OK)
    return throwDwarf("dwarf_tag", obj->dwarfDbg->err);
  const char* tag_name;
  if (dwarf_get_TAG_name(tag, &tag_name) != DW_DLV_OK)
    return throwDwarf("dwarf_get_TAG_name", obj->dwarfDbg->err);
  info.GetReturnValue().Set(Nan::New(tag_name).ToLocalChecked());
}

void DwarfDIE::getPCRange(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DwarfDIE* obj = ObjectWrap::Unwrap<DwarfDIE>(info.Holder());
  Dwarf_Addr hi, lo;
  v8::Local<v8::Object> range = Nan::New<v8::Object>();
  int rc;

  rc = dwarf_highpc(obj->die, &hi, &obj->dwarfDbg->err);
  if (rc == DW_DLV_ERROR)
    return throwDwarf("dwarf_tag", obj->dwarfDbg->err);
  if (rc == DW_DLV_NO_ENTRY)
    range->Set(Nan::New("hi").ToLocalChecked(), Nan::Null());
  else
    range->Set(Nan::New("hi").ToLocalChecked(), Nan::New<v8::Number>(hi));

  rc = dwarf_lowpc(obj->die, &lo, &obj->dwarfDbg->err);
  if (rc == DW_DLV_ERROR)
    return throwDwarf("dwarf_tag", obj->dwarfDbg->err);
  if (rc == DW_DLV_NO_ENTRY)
    range->Set(Nan::New("lo").ToLocalChecked(), Nan::Null());
  else
    range->Set(Nan::New("lo").ToLocalChecked(), Nan::New<v8::Number>(lo));

  info.GetReturnValue().Set(range);
}

void DwarfDIE::getAttributes(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  int rc;
  DwarfDIE* obj = ObjectWrap::Unwrap<DwarfDIE>(info.Holder());
  v8::Local<v8::Array> attributes = Nan::New<v8::Array>();
  Dwarf_Attribute* atlist;
  Dwarf_Half attrcode, attrform;
  const char* attrname;
  const char* formname;
  Dwarf_Form_Class formclass;
  Dwarf_Signed attrcount, a;

  rc = dwarf_attrlist(obj->die, &atlist, &attrcount, &obj->dwarfDbg->err);
  if (rc == DW_DLV_NO_ENTRY)
    attrcount = 0;
  else if (rc != DW_DLV_OK)
    return throwDwarf("dwarf_attrlist", obj->dwarfDbg->err);

  for (a = 0; a < attrcount; a++) {
    if (dwarf_whatattr(atlist[a], &attrcode, &obj->dwarfDbg->err) != DW_DLV_OK)
      return throwDwarf("dwarf_whatattr", obj->dwarfDbg->err);
    if (dwarf_whatform(atlist[a], &attrform, &obj->dwarfDbg->err) != DW_DLV_OK)
      return throwDwarf("dwarf_whatform", obj->dwarfDbg->err);
    if (dwarf_get_AT_name(attrcode, &attrname) != DW_DLV_OK)
      return throwDwarf("dwarf_get_AT_name", obj->dwarfDbg->err);
    if (dwarf_get_FORM_name(attrform, &formname) != DW_DLV_OK)
      return throwDwarf("dwarf_get_FORM_name", obj->dwarfDbg->err);
    formclass = dwarf_get_form_class(obj->cu->dwversion, attrcode, obj->cu->headerOffset, attrform);


    v8::Local<v8::Object> attr = Nan::New<v8::Object>();
    attr->Set(Nan::New("code").ToLocalChecked(), Nan::New(attrcode));
    attr->Set(Nan::New("name").ToLocalChecked(), Nan::New(attrname).ToLocalChecked());
    attr->Set(Nan::New("form").ToLocalChecked(), Nan::New(formname).ToLocalChecked());

    if (attrcode == DW_AT_location) {
      Dwarf_Locdesc *llbuf;
      Dwarf_Signed lcnt;
      v8::Local<v8::Array> locdescs = Nan::New<v8::Array>();

      rc = dwarf_loclist(atlist[a], &llbuf, &lcnt, &obj->dwarfDbg->err);
      if (rc == DW_DLV_ERROR)
        return throwDwarf("dwarf_loclist", obj->dwarfDbg->err);
      if (rc != DW_DLV_NO_ENTRY) {
        locdescs = parseLocList(obj->dwarfDbg->dbg, &obj->dwarfDbg->err, llbuf, lcnt);
      }
      attr->Set(Nan::New("value").ToLocalChecked(), locdescs);
    }
    else if (formclass == DW_FORM_CLASS_CONSTANT) {
      Dwarf_Unsigned u;
      if (dwarf_formudata(atlist[a], &u, &obj->dwarfDbg->err) != DW_DLV_OK)
        return throwDwarf("dwarf_formudata", obj->dwarfDbg->err);
      attr->Set(Nan::New("value").ToLocalChecked(), Nan::New<v8::Number>(u));
    }
    else if (formclass == DW_FORM_CLASS_REFERENCE) {
      Dwarf_Off off;
      if (dwarf_global_formref(atlist[a], &off, &obj->dwarfDbg->err) != DW_DLV_OK)
        return throwDwarf("dwarf_formref", obj->dwarfDbg->err);
      //attr->Set(Nan::New("value").ToLocalChecked(), DwarfDIE::NewInstance(obj->cu->handle(), (uint64_t)off));
      attr->Set(Nan::New("value").ToLocalChecked(), obj->dwarfDbg->loadDIE(obj->cu->handle(), (uint64_t)off));
    }
    else if (formclass == DW_FORM_CLASS_ADDRESS) {
      Dwarf_Addr addr;
      if (dwarf_formaddr(atlist[a], &addr, &obj->dwarfDbg->err) != DW_DLV_OK)
        return throwDwarf("dwarf_formaddr", obj->dwarfDbg->err);
      attr->Set(Nan::New("value").ToLocalChecked(), Nan::New<v8::Number>(addr));
    }
    else if (formclass == DW_FORM_CLASS_FLAG) {
      Dwarf_Bool b;
      if (dwarf_formflag(atlist[a], &b, &obj->dwarfDbg->err) != DW_DLV_OK)
        return throwDwarf("dwarf_formudata", obj->dwarfDbg->err);
      attr->Set(Nan::New("value").ToLocalChecked(), Nan::New<v8::Boolean>(b));
    }
    else if (formclass == DW_FORM_CLASS_STRING) {
      char *attr_str = 0;
      if (dwarf_formstring(atlist[a], &attr_str, &obj->dwarfDbg->err) != DW_DLV_OK)
        return throwDwarf("dwarf_formstring", obj->dwarfDbg->err);
      attr->Set(Nan::New("value").ToLocalChecked(), Nan::New(attr_str).ToLocalChecked());
    }

    attributes->Set(a, attr);
    dwarf_dealloc(obj->dwarfDbg->dbg, atlist[a], DW_DLA_ATTR);
  }
  dwarf_dealloc(obj->dwarfDbg->dbg, atlist, DW_DLA_LIST);

  info.GetReturnValue().Set(attributes);
}


void DwarfDIE::loadDie() {
  if (dwarf_offdie(dwarfDbg->dbg, offset,
        &die, &dwarfDbg->err) != DW_DLV_OK)
    return throwDwarf("DwarfDIE::loadDie(): dwarf_offdie", dwarfDbg->err);
}

} // namespace libdwarfjs
