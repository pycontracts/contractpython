project('cowlang', 'cpp', version: '1.0')

inc_dirs_global = include_directories(get_option('prefix') + '/include', '/usr/local/include', 'include/')
prefix_library_path=[get_option('prefix')+'/lib/x86_64-linux-gnu', get_option('prefix')+'/lib', '/usr/local/lib', '/usr/local/lib/x86_64-linux-gnu']

# Third party libraries
subdir('./thirdparty/bitstream')
subdir('./thirdparty/libdocument')
subdir('./thirdparty/pybind11')
subdir('./thirdparty/libpypa')

subdir('./src/compiler')
subdir('./src/interpreter')
subdir('./src/shell')
subdir('./test')

gtest = dependency('gtest')

compile_args = ['-Wall', '-std=c++17']

# FIXME: Pypa causes a lot of warnings
#compile_args += ['-Wextra', '-Werror']

if get_option('use_geo') == true
    compile_args = compile_args + ['-DUSE_GEO']
endif

cpp = meson.get_compiler('cpp')
py3_ext = import('python3')
py3_dep = dependency('python3')

if py3_ext.language_version() == '3.5'
    python_dir = 'lib/python3.5/dist-packages'
elif py3_ext.language_version() == '3.6'
    python_dir = 'lib/python3.6/dist-packages'
else
    error('Unknown python version')
endif

libcowlang = static_library('libcowlang', [compiler_cpp_files, interpreter_cpp_files], include_directories: [inc_dir_pypy, inc_dirs_global, json_inc, bitstream_local_incdir, pybind_inc], dependencies: [json_dep, pypa_dep], install: true, cpp_args: compile_args, name_prefix: '')

cpp = meson.get_compiler('cpp')
readline = cpp.find_library('readline', required: true)


py3_ext.extension_module('cowlang', 'src/python_bindings.cpp',
    dependencies: [py3_dep], install: true, include_directories: [inc_dir_pypy, inc_dirs_global, json_inc, bitstream_local_incdir, pybind_inc],
    cpp_args: compile_args, install_dir: python_dir, link_with: [libcowlang]
)


subdir('doc')

install_subdir('include/cowlang', install_dir : 'include')
tests = executable('cowlang-test', test_cpp_files, dependencies: [gtest, json_dep, pypa_dep], link_with: [libcowlang], include_directories: [inc_dir_pypy, inc_dirs_global, json_inc, bitstream_local_incdir, pybind_inc])
clang_tidy_checks = '-hicpp-signed-bitwise,-readability-implicit-bool-conversion,-cppcoreguidelines-pro-*,-clang-diagnostic*,-llvm-include-order'

# Now, build the command line interpreter
contractpython = executable('contractpython', shell_cpp_files, dependencies: [gtest, json_dep, pypa_dep, readline], link_with: [libcowlang], include_directories: [inc_dir_pypy, inc_dirs_global, json_inc, bitstream_local_incdir, pybind_inc])

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
