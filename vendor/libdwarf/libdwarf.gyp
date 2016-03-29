{
  'targets': [
    {
      'target_name': 'libdwarf',
      'type': 'static_library',
      'sources': [
        'common.c',
        'dwarf_abbrev.c',
        'dwarf_addr_finder.c',
        'dwarf_alloc.c',
        'dwarf_arange.c',
        'dwarf_die_deliv.c',
        'dwarf_elf_access.c',
        'dwarf_error.c',
        'dwarf_form.c',
        'dwarf_frame2.c',
        'dwarf_frame3.c',
        'dwarf_frame.c',
        'dwarf_funcs.c',
        'dwarf_gdbindex.c',
        'dwarf_global.c',
        'dwarf_harmless.c',
        'dwarf_init_finish.c',
        'dwarf_leb.c',
        'dwarf_line2.c',
        'dwarf_line.c',
        'dwarf_loc.c',
        'dwarf_macro5.c',
        'dwarf_macro.c',
        'dwarf_names.c',
        'dwarf_original_elf_init.c',
        'dwarf_print_lines.c',
        'dwarf_pubtypes.c',
        'dwarf_query.c',
        'dwarf_ranges.c',
        'dwarf_sort_line.c',
        'dwarf_string.c',
        'dwarf_stubs.c',
        'dwarf_tied.c',
        'dwarf_tsearchhash.c',
        'dwarf_types.c',
        'dwarf_util.c',
        'dwarf_vars.c',
        'dwarf_weaks.c',
        'dwarf_xu_index.c',
        'dwgetopt.c',
        'gennames.c',
        'malloc_check.c',
        'pro_alloc.c',
        'pro_arange.c',
        'pro_die.c',
        'pro_encode_nm.c',
        'pro_error.c',
        'pro_expr.c',
        'pro_finish.c',
        'pro_forms.c',
        'pro_frame.c',
        'pro_funcs.c',
        'pro_init.c',
        'pro_line.c',
        'pro_macinfo.c',
        'pro_pubnames.c',
        'pro_reloc.c',
        'pro_reloc_stream.c',
        'pro_reloc_symbolic.c',
        'pro_section.c',
        'pro_types.c',
        'pro_vars.c',
        'pro_weaks.c',
      ],
      "dependencies": [
        "../libelf/libelf.gyp:libelf"
      ],
      'include_dirs': [
        './', '../libelf'
      ]
    }
  ]
}
