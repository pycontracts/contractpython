project('cowlang', 'cpp', version: '1.0')

inc_dirs = include_directories(get_option('prefix') + '/include', '/usr/local/include', './include/')
prefix_library_path=[get_option('prefix')+'/lib/x86_64-linux-gnu', get_option('prefix')+'/lib', '/usr/local/lib', '/usr/local/lib/x86_64-linux-gnu']

subdir('./src/compiler')
subdir('./src/interpreter')
subdir('./test')

gtest = dependency('gtest')

compile_args = ['-Wall', '-std=c++17']

# FIXME: Pypa causes a lot of warnings
#compile_args += ['-Wextra', '-Werror']

if get_option('use_geo') == true
    compile_args = compile_args + ['-DUSE_GEO']
endif

cpp = meson.get_compiler('cpp')
log_dep = cpp.find_library('glog')
json_dep = cpp.find_library('document', dirs: prefix_library_path)
pypa_dep = cpp.find_library('pypa', dirs: prefix_library_path)

py3_ext = import('python3')
py3_dep = dependency('python3')

if py3_ext.language_version() == '3.5'
    python_dir = 'lib/python3.5/dist-packages'
elif py3_ext.language_version() == '3.6'
    python_dir = 'lib/python3.6/dist-packages'
else
    error('Unknown python version')
endif

libcowlang = shared_library('libcowlang', [compiler_cpp_files, interpreter_cpp_files], include_directories: inc_dirs, dependencies: [log_dep, json_dep, pypa_dep], install: true, cpp_args: compile_args, name_prefix: '')

cpp = meson.get_compiler('cpp')

sgx_sdk_dir = get_option('sgx_sdk_dir')
sgx_library_path = sgx_sdk_dir + '/lib64'

sgx_tstdc_dep = cpp.find_library('sgx_tstdc', dirs: sgx_library_path, required: false)
sgx_tstdcxx_dep = cpp.find_library('sgx_tcxx', dirs: sgx_library_path, required: false)
sgx_trts_dep = cpp.find_library('sgx_trts', dirs: sgx_library_path, required: false)

if sgx_tstdc_dep.found() and sgx_tstdcxx_dep.found() and sgx_trts_dep.found()
    message('SGX found')

    sgx_tlibc_incdir = include_directories(sgx_sdk_dir +'/include/tlibc')
    sgx_stdcxx_incdir = include_directories(sgx_sdk_dir +'/include/libcxx')
    sgx_incdir = include_directories(sgx_sdk_dir + '/include')

    sgx_compile_args = ['-DIS_ENCLAVE', '-nostdinc', '-fstack-protector', '-pie', '-UNDEBUG', '-UEDEBUG']

    cowlang_sgx = static_library('cowlang_sgx', interpreter_cpp_files,
        c_args: sgx_compile_args,
        cpp_args: ['-std=c++11', '-nostdinc++'] + sgx_compile_args,
        link_args:['-Wl,--no-undefined','-nostdlib','-nodefaultlibs','-nostartfiles','-L'+sgx_sdk_dir+'/lib64',
        '-Wl,--whole-archive','-lsgx_trts','-Wl,--no-whole-archive',
        '-Wl,--start-group','-lsgx_tstdc','-lsgx_tcxx', '-lsgx_tkey_exchange','-ldocument_sgx','-Wl,--end-group',
        ],
        include_directories: [sgx_incdir, sgx_tlibc_incdir, sgx_stdcxx_incdir, inc_dirs], install: true)
else
    message('SGX not found. Not building trusted library.')
endif

py3_ext.extension_module('cowlang', 'src/python_bindings.cpp',
    dependencies: [py3_dep], install: true, include_directories: [inc_dirs],
    cpp_args: compile_args, install_dir: python_dir, link_with: [libcowlang]
)


subdir('doc')

install_subdir('include/cowlang', install_dir : 'include')

tests = executable('cowlang-test', test_cpp_files, dependencies: [gtest, json_dep, pypa_dep], link_with: [libcowlang], include_directories: inc_dirs)

clang_tidy_checks = '-hicpp-signed-bitwise,-readability-implicit-bool-conversion,-cppcoreguidelines-pro-*,-clang-diagnostic*,-llvm-include-order'

# NOTE: gtest on ubuntu still uses deprecated functions so we can't lint the test files yet
clangtidy = find_program('clang-tidy', required: false)
if clangtidy.found()
    run_target(
        'tidy',
        command: [
            clangtidy,
            '-checks=' + clang_tidy_checks,
            '-warnings-as-errors=*',
            '-p', meson.build_root()
        ] + compiler_cpp_files + interpreter_cpp_files)
endif
test('cowlang-test', tests)
