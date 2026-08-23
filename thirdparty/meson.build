# fdt
foreach file : [
    'fdt.c',
    'fdt_ro.c',
    'fdt_wip.c',
    'fdt_sw.c',
    'fdt_rw.c',
    'fdt_strerror.c',
    'fdt_empty_tree.c',
    'fdt_addresses.c',
    'fdt_overlay.c',
    'fdt_check.c',
]
    thirdparty_c_sources += files(project_root / 'thirdparty' / 'libfdt' / file)
endforeach

# flanterm
foreach file : ['flanterm.c', 'flanterm_backends/fb.c']
    thirdparty_c_sources += files(project_root / 'thirdparty' / 'Flanterm'/ 'src' / file)
endforeach
