#include "DwarfDbg.h"
#include "DwarfCUHeader.h"
#include "DwarfDIE.h"

namespace libdwarfjs {

void InitAll(v8::Local<v8::Object> exports) {
  DwarfDbg::Init(exports);
  DwarfCUHeader::Init(exports);
  DwarfDIE::Init(exports);
}

NODE_MODULE(libdwarfjs, InitAll)

} // namespace libdwarfjs
