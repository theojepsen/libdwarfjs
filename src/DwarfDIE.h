#ifndef LIBDWARFJS_DWARFDIE_H
#define LIBDWARFJS_DWARFDIE_H

#include <nan.h>
#include "libdwarf.h"
#include "DwarfCUHeader.h"
#include "DwarfDbg.h"

class DwarfDbg;
class DwarfCUHeader;

namespace libdwarfjs {

class DwarfDIE : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  explicit DwarfDIE();

  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Object> d, uint64_t offset);

 private:
  ~DwarfDIE();

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getCUHeader(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getOffset(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getChildren(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getName(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getTagName(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getAttributes(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getPCRange(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
  Nan::Persistent<v8::String> filename;


  void loadDie();
  uint64_t offset;

  DwarfCUHeader *cu;
  DwarfDbg *dwarfDbg;
  Dwarf_Die die;
  static NAN_METHOD(NewInstance);

};

} // namespace libdwarfjs
#endif // LIBDWARFJS_DWARFDIE_H
