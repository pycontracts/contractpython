inc_dir_pypy = include_directories('/usr/local/include', 'src/', get_option('prefix')+'/include', '../bitstream/include','../libdocument/include')

compile_args = ['-std=c++17', '-Wextra', '-Wno-implicit-exception-spec-mismatch'] # Remove me once clang issue is fixed in SGX SDK

if get_option('use_geo') == true
    compile_args = compile_args + ['-DUSE_GEO']
endif
cc = meson.get_compiler('cpp')

gmp_dep = cc.find_library('gmp')

cpp_files = files(
                 'src/pypa/ast/ast.cc',
                 'src/pypa/ast/dump.cc',
                 'src/pypa/filebuf.cc',
                 'src/pypa/lexer/lexer.cc',
                 'src/pypa/parser/parser.cc',
                 'src/pypa/parser/make_string.cc',
                 'src/pypa/parser/symbol_table.cc',
                 'src/double-conversion/src/double-conversion.cc',
                 'src/double-conversion/src/bignum-dtoa.cc',
                 'src/double-conversion/src/cached-powers.cc',
                 'src/double-conversion/src/fast-dtoa.cc',
                 'src/double-conversion/src/diy-fp.cc',
                 'src/double-conversion/src/fixed-dtoa.cc',
                 'src/double-conversion/src/strtod.cc',
                 'src/double-conversion/src/bignum.cc')

pypa = static_library('pypa', cpp_files, dependencies: [gmp_dep], include_directories: inc_dir_pypy, install: false, cpp_args: compile_args)
pypa_dep = declare_dependency(link_with: pypa)

