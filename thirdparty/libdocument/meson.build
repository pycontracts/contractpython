inc_dirs = include_directories('include/', '../bitstream/include', '../pybind11/include')
json_inc = include_directories('include/')

gtest = dependency('gtest')

compile_args = ['-std=c++17', '-Wextra', '-Wno-implicit-exception-spec-mismatch', '-Werror'] # Remove me once clang issue is fixed in SGX SDK

if get_option('use_geo') == true
    compile_args = compile_args + ['-DUSE_GEO']
endif

subdir('src')

doc = static_library('document', cpp_files, include_directories: inc_dirs, install: false, cpp_args: compile_args)
json_dep = declare_dependency(link_with: doc)

