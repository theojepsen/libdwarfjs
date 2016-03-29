#ifndef LIBDWARFJS_DWARFDBG_H
#define LIBDWARFJS_DWARFDBG_H

#include <nan.h>
#include "libdwarf.h"
#include "DwarfCUHeader.h"
#include "DwarfDIE.h"
#include <map>

class DwarfDIE;

namespace libdwarfjs {

class DwarfDbg : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  Dwarf_Debug dbg;
  Dwarf_Error err;

  v8::Local<v8::Object> loadDIE(v8::Local<v8::Object> cuObj, uint64_t offset);

 private:
  explicit DwarfDbg(v8::Local<v8::String> filename);
  ~DwarfDbg();

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getCUHeaders(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void getLocList(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static Nan::Persistent<v8::Function> constructor;
  Nan::Persistent<v8::String> filename;

  std::map<uint64_t, DwarfDIE* > dieMap;

  int prog_fd;
};

} // namespace libdwarfjs
#endif // LIBDWARFJS_DWARFDBG_H
