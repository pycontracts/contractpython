  # CMakeLists.txt -- Build system for the pybind11 test suite
  #
  # Copyright (c) 2015 Wenzel Jakob <wenzel@inf.ethz.ch>
  #
  # All rights reserved. Use of this source code is governed by a
  # BSD-style license that can be found in the LICENSE file.
  if 'CMAKE_CURRENT_SOURCE_DIR', 'STREQUAL', 'CMAKE_SOURCE_DIR'
    # We're being loaded directly, i.e. not via add_subdirectory, so make this
    # work as its own project and load the pybind11Config to get the tools we need
    project('pybind11_tests', 'cpp')
    pybind11_dep = dependency('pybind11')
  endif
  if 'NOT', 'CMAKE_BUILD_TYPE', 'AND', 'NOT', 'CMAKE_CONFIGURATION_TYPES'
    # message(['STATUS', 'Setting tests build type to MinSizeRel as none was specified'])
    cmake_build_type = ['MinSizeRel', 'CACHE', 'STRING', 'Choose the type of build.', 'FORCE']
    # set_property(['CACHE', 'CMAKE_BUILD_TYPE', 'PROPERTY', 'STRINGS', 'Debug', 'Release', 'MinSizeRel', 'RelWithDebInfo'])
  endif
  # Full set of test files (you can override these; see below)
  pybind11_test_files = ['test_buffers.cpp', 'test_builtin_casters.cpp', 'test_call_policies.cpp', 'test_callbacks.cpp', 'test_chrono.cpp', 'test_class.cpp', 'test_constants_and_functions.cpp', 'test_copy_move.cpp', 'test_docstring_options.cpp', 'test_eigen.cpp', 'test_enum.cpp', 'test_eval.cpp', 'test_exceptions.cpp', 'test_factory_constructors.cpp', 'test_gil_scoped.cpp', 'test_iostream.cpp', 'test_kwargs_and_defaults.cpp', 'test_local_bindings.cpp', 'test_methods_and_attributes.cpp', 'test_modules.cpp', 'test_multiple_inheritance.cpp', 'test_numpy_array.cpp', 'test_numpy_dtypes.cpp', 'test_numpy_vectorize.cpp', 'test_opaque_types.cpp', 'test_operator_overloading.cpp', 'test_pickling.cpp', 'test_pytypes.cpp', 'test_sequences_and_iterators.cpp', 'test_smart_ptr.cpp', 'test_stl.cpp', 'test_stl_binders.cpp', 'test_tagbased_polymorphic.cpp', 'test_virtual_functions.cpp']
  # Invoking cmake with something like:
  #     cmake -DPYBIND11_TEST_OVERRIDE="test_callbacks.cpp;test_picking.cpp" ..
  # lets you override the tests that get compiled and run.  You can restore to all tests with:
  #     cmake -DPYBIND11_TEST_OVERRIDE= ..
  if 'PYBIND11_TEST_OVERRIDE'
    pybind11_test_files = pybind11_test_override
  endif
  # string(['REPLACE', '.cpp', '.py', 'PYBIND11_PYTEST_FILES', '${PYBIND11_TEST_FILES}'])
  # Contains the set of test files that require pybind11_cross_module_tests to be
  # built; if none of these are built (i.e. because TEST_OVERRIDE is used and
  # doesn't include them) the second module doesn't get built.
  pybind11_cross_module_tests = ['test_exceptions.py', 'test_local_bindings.py', 'test_stl.py', 'test_stl_binders.py']
  # Check if Eigen is available; if not, remove from PYBIND11_TEST_FILES (but
  # keep it in PYBIND11_PYTEST_FILES, so that we get the "eigen is not installed"
  # skip message).
  # list(['FIND', 'PYBIND11_TEST_FILES', 'test_eigen.cpp', 'PYBIND11_TEST_FILES_EIGEN_I'])
  if 'PYBIND11_TEST_FILES_EIGEN_I', 'GREATER', '-1'
    # Try loading via newer Eigen's Eigen3Config first (bypassing tools/FindEigen3.cmake).
    # Eigen 3.3.1+ exports a cmake 3.0+ target for handling dependency requirements, but also
    # produces a fatal error if loaded from a pre-3.0 cmake.
    if 'NOT', 'CMAKE_VERSION', 'VERSION_LESS', '3.0'
      Eigen3_dep = dependency('Eigen3')
      if 'EIGEN3_FOUND'
        if 'EIGEN3_VERSION_STRING', 'AND', 'NOT', 'EIGEN3_VERSION_STRING', 'VERSION_LESS', '3.3.1'
          pybind11_eigen_via_target = '1'
        endif
      endif
    endif
    if 'NOT', 'EIGEN3_FOUND'
      # Couldn't load via target, so fall back to allowing module mode finding, which will pick up
      # tools/FindEigen3.cmake
      Eigen3_dep = dependency('Eigen3')
    endif
    if 'EIGEN3_FOUND'
      # Eigen 3.3.1+ cmake sets EIGEN3_VERSION_STRING (and hard codes the version when installed
      # rather than looking it up in the cmake script); older versions, and the
      # tools/FindEigen3.cmake, set EIGEN3_VERSION instead.
      if 'NOT', 'EIGEN3_VERSION', 'AND', 'EIGEN3_VERSION_STRING'
        eigen3_version = eigen3_version_string
      endif
      # message(['STATUS', 'Building tests with Eigen v${EIGEN3_VERSION}'])
    else
      # list(['REMOVE_AT', 'PYBIND11_TEST_FILES', pybind11_test_files_eigen_i])
      # message(['STATUS', 'Building tests WITHOUT Eigen'])
    endif
  endif
  # Optional dependency for some tests (boost::variant is only supported with version >= 1.56)
  Boost_dep = dependency('Boost')
  # Compile with compiler warnings turned on
  # function(['pybind11_enable_warnings', 'target_name'])
  if 'MSVC'
    # target_compile_options([target_name, 'PRIVATE', '/W4'])
  else
    # target_compile_options([target_name, 'PRIVATE', '-Wall', '-Wextra', '-Wconversion', '-Wcast-qual', '-Wdeprecated'])
  endif
  if 'PYBIND11_WERROR'
    if 'MSVC'
      # target_compile_options([target_name, 'PRIVATE', '/WX'])
    else
      # target_compile_options([target_name, 'PRIVATE', '-Werror'])
    endif
  endif
  # endfunction()
  test_targets = 'pybind11_tests'
  # Build pybind11_cross_module_tests if any test_whatever.py are being built that require it
  # foreach(['t', pybind11_cross_module_tests])
  # list(['FIND', 'PYBIND11_PYTEST_FILES', t, 'i'])
  if 'i', 'GREATER', '-1'
    # list(['APPEND', 'test_targets', 'pybind11_cross_module_tests'])
    # break()
  endif
  # endforeach()
  testdir = cmake_current_source_dir
  # foreach(['target', test_targets])
  test_files = pybind11_test_files
  if 'NOT', 'target', 'STREQUAL', 'pybind11_tests'
    test_files = ''
  endif
  # Create the binding library
  # pybind11_add_module([target, 'THIN_LTO', target, '.cpp', test_files, pybind11_headers])
  # pybind11_enable_warnings(target)
  if 'MSVC'
    # target_compile_options([target, 'PRIVATE', '/utf-8'])
  endif
  if 'EIGEN3_FOUND'
    if 'PYBIND11_EIGEN_VIA_TARGET'
      # target_link_libraries([target, 'PRIVATE', 'Eigen3::Eigen'])
    else
      # target_include_directories([target, 'PRIVATE', eigen3_include_dir])
    endif
    # target_compile_definitions([target, 'PRIVATE', '-DPYBIND11_TEST_EIGEN'])
  endif
  if 'Boost_FOUND'
    # target_include_directories([target, 'PRIVATE', boost_include_dirs])
    # target_compile_definitions([target, 'PRIVATE', '-DPYBIND11_TEST_BOOST'])
  endif
  # Always write the output file directly into the 'tests' directory (even on MSVC)
  if 'NOT', 'CMAKE_LIBRARY_OUTPUT_DIRECTORY'
    # set_target_properties([target, 'PROPERTIES', 'LIBRARY_OUTPUT_DIRECTORY', testdir])
    # foreach(['config', cmake_configuration_types])
    # string(['TOUPPER', config, 'config'])
    # set_target_properties([target, 'PROPERTIES', 'LIBRARY_OUTPUT_DIRECTORY_${config}', testdir])
    # endforeach()
  endif
  # endforeach()
  # Make sure pytest is found or produce a fatal error
  if 'NOT', 'PYBIND11_PYTEST_FOUND'
    # execute_process(['COMMAND', python_executable, '-c', 'import pytest; print(pytest.__version__)', 'RESULT_VARIABLE', 'pytest_not_found', 'OUTPUT_VARIABLE', 'pytest_version', 'ERROR_QUIET'])
    if 'pytest_not_found'
      # message(['FATAL_ERROR', 'Running the tests requires pytest. Please install it manually', ' (try: ${PYTHON_EXECUTABLE} -m pip install pytest)'])
    elif 'pytest_version', 'VERSION_LESS', '3.0'
      # message(['FATAL_ERROR', 'Running the tests requires pytest >= 3.0. Found: ${pytest_version}', 'Please update it (try: ${PYTHON_EXECUTABLE} -m pip install -U pytest)'])
    endif
    pybind11_pytest_found = ['TRUE', 'CACHE', 'INTERNAL', '']
  endif
  if 'CMAKE_VERSION', 'VERSION_LESS', '3.2'
    pybind11_uses_terminal = ''
  else
    pybind11_uses_terminal = 'USES_TERMINAL'
  endif
  # A single command to compile and run the tests
  # add_custom_target(['pytest', 'COMMAND', python_executable, '-m', 'pytest', pybind11_pytest_files, 'DEPENDS', test_targets, 'WORKING_DIRECTORY', testdir, pybind11_uses_terminal])
  if 'PYBIND11_TEST_OVERRIDE'
    # add_custom_command(['TARGET', 'pytest', 'POST_BUILD', 'COMMAND', cmake_command, '-E', 'echo', 'Note: not all tests run: -DPYBIND11_TEST_OVERRIDE is in effect'])
  endif
  # Add a check target to run all the tests, starting with pytest (we add dependencies to this below)
  # add_custom_target(['check', 'DEPENDS', 'pytest'])
  # The remaining tests only apply when being built as part of the pybind11 project, but not if the
  # tests are being built independently.
  if 'NOT', 'PROJECT_NAME', 'STREQUAL', 'pybind11'
    # return()
  endif
  # Add a post-build comment to show the primary test suite .so size and, if a previous size, compare it:
  # add_custom_command(['TARGET', 'pybind11_tests', 'POST_BUILD', 'COMMAND', python_executable, project_source_dir, '/tools/libsize.py', '$<TARGET_FILE:pybind11_tests>', cmake_current_binary_dir, '/sosize-$<TARGET_FILE_NAME:pybind11_tests>.txt'])
  # Test embedding the interpreter. Provides the `cpptest` target.
  subdir('test_embed')
  # Test CMake build using functions and targets from subdirectory or installed location
  subdir('test_cmake_build')
