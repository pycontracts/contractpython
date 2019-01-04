  if python_module_extension, 'MATCHES', 'pypy'
    # add_custom_target('cpptest')
    # Dummy target on PyPy. Embedding is not supported.
    _suppress_unused_variable_warning = '${DOWNLOAD_CATCH}'
    # return()
  endif
  Catch_dep = dependency('Catch')
  if 'CATCH_FOUND'
    # message(['STATUS', 'Building interpreter tests using Catch v${CATCH_VERSION}'])
  else
    # message(['STATUS', 'Catch not detected. Interpreter tests will be skipped. Install Catch headers', ' manually or use `cmake -DDOWNLOAD_CATCH=1` to fetch them automatically.'])
    # return()
  endif
  test_embed_exe = executable('test_embed', 'catch.cpp', 'test_interpreter.cpp')
  # target_include_directories(['test_embed', 'PRIVATE', catch_include_dir])
  # pybind11_enable_warnings('test_embed')
  if 'NOT', 'CMAKE_VERSION', 'VERSION_LESS', '3.0'
    # target_link_libraries(['test_embed', 'PRIVATE', 'pybind11::embed'])
  else
    # target_include_directories(['test_embed', 'PRIVATE', pybind11_include_dir, python_include_dirs])
    # target_compile_options(['test_embed', 'PRIVATE', pybind11_cpp_standard])
    # target_link_libraries(['test_embed', 'PRIVATE', python_libraries])
  endif
  Threads_dep = dependency('Threads')
  # target_link_libraries(['test_embed', 'PUBLIC', cmake_thread_libs_init])
  # add_custom_target(['cpptest', 'COMMAND', '$<TARGET_FILE:test_embed>', 'WORKING_DIRECTORY', cmake_current_source_dir])
  # pybind11_add_module(['external_module', 'THIN_LTO', 'external_module.cpp'])
  # set_target_properties(['external_module', 'PROPERTIES', 'LIBRARY_OUTPUT_DIRECTORY', cmake_current_source_dir])
  # add_dependencies(['cpptest', 'external_module'])
  # add_dependencies(['check', 'cpptest'])
