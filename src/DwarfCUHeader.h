#ifndef LIBDWARFJS_DWARFCUHEADER_H
#define LIBDWARFJS_DWARFCUHEADER_H

#include <nan.h>
#include "libdwarf.h"
#include "DwarfCUHeader.h"
#include "DwarfDbg.h"

class DwarfDbg;

namespace libdwarfjs {

class DwarfCUHeader : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  explicit DwarfCUHeader();

  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Object> d, uint64_t headerOffset, uint16_t version);
  DwarfDbg *dwarfDbg;
  Dwarf_Half dwversion;
  uint64_t headerOffset;

 private:
  ~DwarfCUHeader();

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getHeaderOffset(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getDIE(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
  Nan::Persistent<v8::String> filename;


  v8::Local<v8::Object> dwarfDbgObj;
  static NAN_METHOD(NewInstance);

};

} // namespace libdwarfjs
#endif // LIBDWARFJS_DWARFCUHEADER_H
