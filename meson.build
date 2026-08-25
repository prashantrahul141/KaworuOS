project(
    'KaworuOS',
    'c',
    default_options: ['c_std=gnu23'],
)

fs = import('fs')
project_root = meson.project_source_root()

# parse kconfig -----------------
kconfig_file = project_root / '.config'

if not fs.exists(kconfig_file)
    warning(
        'Kconfig config file not found. Run `make defconfig` or `make debugconfig` before building.',
    )
    kconfig_result = ''
else
    python = find_program('python')
    kconfig_result = run_command(
        python,
        project_root / 'scripts/kconfig-to-meson.py',
        kconfig_file,
        check: true,
    ).stdout()
endif


kconfig = {}
foreach line : kconfig_result.strip().split('\n')
    if line.strip() == ''
        continue
    endif

    parts = line.split('=')
    if parts.length() != 2
        error('Invalid Kconfig output (value contains "="): ' + line)
    endif

    key = parts[0].strip()
    value = parts[1].strip()
    kconfig += {key: value}
endforeach

# compiler args -----------------
common_warning_flags = ['-Wno-unused-command-line-argument']
warning_flags = [
    '-Wall',
    '-Wextra',
    '-Wpedantic',
    '-Wconversion',
    '-Wsign-conversion',
    '-Wimplicit-int-conversion',
    '-Wshorten-64-to-32',
    '-Wpointer-to-int-cast',
    '-Wint-to-pointer-cast',
    '-Wshadow',
    '-Wundef',
    '-Wcast-qual',
    '-Wcast-align',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes',
    '-Wmissing-declarations',
    '-Wswitch-enum',
    '-Wswitch-default',
    '-Wnull-dereference',
    '-Wunreachable-code',
    '-Wvla',
    '-Walloca',
    '-Wwrite-strings',
    '-Wformat=2',
    '-Wcast-function-type',
    '-Wconditional-uninitialized',
    '-Wcomma',
    '-Wextra-semi-stmt',
    '-Wno-language-extension-token',
    '-Wno-gnu-empty-struct',
    '-Wno-gnu-zero-variadic-macro-arguments',
] + common_warning_flags

# warning as errors?
if kconfig.get('CONFIG_WARNING_AS_ERROR', 'false') == 'true'
    warning_flags += ['-Werror']
endif

# common across all translation units
common_flags = []

# debugging?
if kconfig.get('CONFIG_DEBUG_SYMBOLS', 'false') == 'true'
    common_flags += ['-ggdb3']
endif

# optimize?
if kconfig.get('CONFIG_OPTIMIZATIONS_LOW', 'false') == 'true'
    common_flags += ['-O1']
elif kconfig.get('CONFIG_OPTIMIZATIONS_MEDIUM', 'false') == 'true'
    common_flags += ['-O2', '-flto=thin']
elif kconfig.get('CONFIG_OPTIMIZATIONS_HIGH', 'false') == 'true'
    common_flags += ['-O2', '-flto=full']
elif kconfig.get('CONFIG_OPTIMIZATIONS_MAX', 'false') == 'true'
    common_flags += ['-O3', '-flto=full']
else
    common_flags += ['-O0']
endif

# common across all c translation units
common_c_flags = ['-ffreestanding', '-nostdlib']

# used in kernel c sources only
kernel_c_flags = common_flags + common_c_flags + warning_flags

# used in thirdparty c sources only
thirdparty_c_flags = common_flags + common_c_flags + common_warning_flags

# user program files
user_c_flags = common_flags + common_c_flags + warning_flags

# include directories -----------------
include_dirs = [
    '.',
    './drivers/',
    './kernel/',
    './kernel/arch/',
    './lib/',
    './thirdparty/limine-protocol/include/',
    './thirdparty/Flanterm/src/',
    './thirdparty/libfdt/',
]

# sources -----------------
kernel_c_sources = []
kernel_asm_sources = []
thirdparty_c_sources = []
user_c_sources = []
user_asm_sources = []

subdir('kernel')
subdir('drivers')
subdir('lib')
subdir('thirdparty')
subdir('user')

# static libs -----------------
kaworu_lib = static_library(
    'kaworu',
    kernel_c_sources + kernel_asm_sources,
    include_directories: include_dirs,
    c_args: kernel_c_flags,
    install: false,
)

thirdparty_lib = static_library(
    'thirdparty',
    thirdparty_c_sources,
    include_directories: include_dirs,
    c_args: thirdparty_c_flags,
    install: false,
)

user_lib = executable(
    'userinit',
    user_c_sources,
    include_directories: include_dirs,
    c_args: user_c_flags,
    link_args: [
        '-fuse-ld=lld',
        '-nostdlib',
        '-nodefaultlibs',
        '-nostartfiles',
        '-Wl,-T,' + project_root / 'user/user.ld',
    ],
    install: false,
)

# generate elf -----------
kernel_elf = executable(
    'kaworu.elf',
    'kernel/init.c',
    link_whole: [
        kaworu_lib,
        thirdparty_lib,
    ],
    include_directories: include_dirs,
    c_args: kernel_c_flags,
    link_args: [
        '-fuse-ld=lld',
        '-nostdlib',
        '-nodefaultlibs',
        '-nostartfiles',
        '-Wl,-T,' + project_root / 'kernel/kernel.ld',
    ],
    install: false,
)
