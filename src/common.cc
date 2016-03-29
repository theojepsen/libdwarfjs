#include "common.h"
#include <stdio.h>
#include <cstdarg>


namespace libdwarfjs {

void throwErr(const char* fmt, ...) {
  va_list args;
  char s[256];

  va_start(args, fmt);
  vsprintf(s, fmt, args);
  va_end(args);

  Nan::ThrowError(s);
}

void throwDwarf(const char* msg, Dwarf_Error err) {
  throwErr("%s: %s", msg, dwarf_errmsg(err));
}

v8::Local<v8::Array> parseLocList(Dwarf_Debug dbg, Dwarf_Error *err, Dwarf_Locdesc *llbuf, Dwarf_Signed lcnt) {
  Dwarf_Signed i, j;
  v8::Local<v8::Array> locdescs = Nan::New<v8::Array>();
  const char* atomname;
  for (i = 0; i < lcnt; i++) {
    v8::Local<v8::Object> locdesc = Nan::New<v8::Object>();
    locdesc->Set(Nan::New("lopc").ToLocalChecked(), Nan::New<v8::Number>((unsigned)llbuf[i].ld_lopc));
    locdesc->Set(Nan::New("hipc").ToLocalChecked(), Nan::New<v8::Number>((unsigned)llbuf[i].ld_hipc));

    v8::Local<v8::Array> locs = Nan::New<v8::Array>();
    for (j = 0; j < llbuf[i].ld_cents; j++) {
      v8::Local<v8::Object> loc = Nan::New<v8::Object>();
      loc->Set(Nan::New("atomCode").ToLocalChecked(), Nan::New<v8::Number>(llbuf[i].ld_s[j].lr_atom));
      if (dwarf_get_OP_name(llbuf[i].ld_s[j].lr_atom, &atomname) == DW_DLV_OK)
        loc->Set(Nan::New("atom").ToLocalChecked(), Nan::New(atomname).ToLocalChecked());
      switch (llbuf[i].ld_s[j].lr_atom) {
        case DW_OP_fbreg:
          loc->Set(Nan::New("number").ToLocalChecked(), Nan::New<v8::Number>((signed long)llbuf[i].ld_s[j].lr_number));
          break;
        default:
          loc->Set(Nan::New("number").ToLocalChecked(), Nan::New<v8::Number>((unsigned)llbuf[i].ld_s[j].lr_number));
      }
      loc->Set(Nan::New("number2").ToLocalChecked(), Nan::New<v8::Number>((unsigned)llbuf[i].ld_s[j].lr_number2));
      loc->Set(Nan::New("offset").ToLocalChecked(), Nan::New<v8::Number>((unsigned)llbuf[i].ld_s[j].lr_offset));
      locs->Set(j, loc);
    }
    locdesc->Set(Nan::New("locations").ToLocalChecked(), locs);

    locdescs->Set(i, locdesc);

    dwarf_dealloc(dbg, llbuf[i].ld_s, DW_DLA_LOC_BLOCK);
  }
  dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);

  return locdescs;
}


} // namespace libdwarfjs
