project('cowlang', ['c', 'cpp'], version: '1.0')

inc_dirs_global = include_directories('include/')
prefix_library_path=[get_option('prefix')+'/lib/x86_64-linux-gnu', '/usr/local/lib/x86_64-linux-gnu']

# Third party libraries
subdir('./thirdparty/btc')
subdir('./thirdparty/snappy')
subdir('./thirdparty/bitstream')
subdir('./thirdparty/pybind11')
subdir('./thirdparty/libpypa')
subdir('./src/compiler')
subdir('./src/interpreter')
subdir('./src/modules')
subdir('./test')

gtest = dependency('gtest')
boost = dependency('boost', modules : ['serialization'])


compile_args = ['-Wall', '-std=c++17']

# FIXME: Pypa causes a lot of warnings
#compile_args += ['-Wextra', '-Werror']

if get_option('use_geo') == true
    compile_args = compile_args + ['-DUSE_GEO']
endif

cpp = meson.get_compiler('cpp')

libcowlang = static_library('libcowlang', [module_cpp_files, compiler_cpp_files, interpreter_cpp_files], include_directories: [btcinc, inc_dir_pypy, inc_dirs_global, bitstream_local_incdir, pybind_inc, inc_dir_snappy], dependencies: [pypa_dep], install: true, cpp_args: compile_args, name_prefix: '')
readline = cpp.find_library('readline', required: true)


shell_cpp_files = files(
    'src/shell/main.cpp')
slib_cpp_files = files(
    'src/shell/library.cpp')
slibtest_cpp_files = files(
    'src/shell/library_tester.cpp', 'src/shell/library.cpp')



subdir('doc')

install_subdir('include/cowlang', install_dir : 'include')
tests = executable('cowlang-test', test_cpp_files, dependencies: [gtest, pypa_dep], link_with: [btclib, libcowlang, snappy_lib], include_directories: [btcinc, inc_dir_pypy, inc_dirs_global, bitstream_local_incdir, pybind_inc,inc_dir_snappy])
clang_tidy_checks = '-hicpp-signed-bitwise,-readability-implicit-bool-conversion,-cppcoreguidelines-pro-*,-clang-diagnostic*,-llvm-include-order'

# Now, build the command line interpreter
contractpython = executable('contractpython', [module_cpp_files, compiler_cpp_files, interpreter_cpp_files, shell_cpp_files], dependencies: [gtest, pypa_dep, readline, boost], link_with: [btclib, snappy_lib], include_directories: [btcinc, inc_dir_pypy, inc_dirs_global, bitstream_local_incdir, pybind_inc, inc_dir_snappy])

# .. and the shared library
contractpythonlib = static_library('contractpython', [module_cpp_files, compiler_cpp_files, interpreter_cpp_files, slib_cpp_files], dependencies: [gtest, pypa_dep, readline, boost], link_with: [btclib, snappy_lib], include_directories: [btcinc, inc_dir_pypy, inc_dirs_global, bitstream_local_incdir, pybind_inc, inc_dir_snappy])

# .. and the shared library
contractpythonlibtest = executable('librarytester', [module_cpp_files, compiler_cpp_files, interpreter_cpp_files, slibtest_cpp_files], dependencies: [gtest, pypa_dep, readline, boost], link_with: [btclib, contractpythonlib, snappy_lib], include_directories: [btcinc, inc_dir_pypy, inc_dirs_global, bitstream_local_incdir, pybind_inc, inc_dir_snappy])

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
