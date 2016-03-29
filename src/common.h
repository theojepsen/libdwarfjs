#ifndef LIBDWARFJS_COMMON_H
#define LIBDWARFJS_COMMON_H

#include <nan.h>
#include "libdwarf.h"
#include "dwarf.h"


namespace libdwarfjs {

void throwErr(const char* fmt, ...);
void throwDwarf(const char* msg, Dwarf_Error err);
v8::Local<v8::Array> parseLocList(Dwarf_Debug dbg, Dwarf_Error *err, Dwarf_Locdesc *llbuf, Dwarf_Signed lcnt);

} // namespace libdwarfjs
#endif // LIBDWARFJS_COMMON_H
