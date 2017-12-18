datadir = join_paths(get_option('datadir'), 'doc', 'spede')

doxygen = find_program('doxygen', required : false)
if not doxygen.found()
  error(' doxygen not found.')
endif

cdata = configuration_data()
cdata.set('VERSION', meson.project_version())
cdata.set('TOP_SRCDIR', meson.source_root())
cdata.set('TOP_BUILDDIR', meson.build_root())

doxyfile = configure_file(input: 'Doxyfile.in',
                          output: 'Doxyfile',
                          configuration: cdata,
                          install: false)

html_target = custom_target('docs',
                            input: doxyfile,
                            output: 'html',
                            command: [doxygen, doxyfile],
                            build_by_default: false)
