{
  "targets": [
    {
      "target_name": "libdwarfjs",
      "sources": [
        "src/libdwarfjs.cc",
        "src/common.cc",
        "src/DwarfDbg.cc",
        "src/DwarfCUHeader.cc",
        "src/DwarfDIE.cc"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "./vendor/libdwarf/"
      ],
      "dependencies": [
        "./vendor/libdwarf/libdwarf.gyp:libdwarf",
        "./vendor/libelf/libelf.gyp:libelf"
      ],
    }
  ]
}
