if get_option('gtk_doc')
  version_conf = configuration_data()
  version_conf.set('BUOH_VERSION', meson.project_version())
  buoh_docs = configure_file(
    input: 'buoh-docs.xml.in',
    output: '@BASENAME@',
    configuration: version_conf,
  )

  glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
  glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')

  gnome.gtkdoc(
    'buoh',
    main_xml: join_paths(meson.current_build_dir(), 'buoh-docs.xml'),
    namespace: 'buoh',
    mode: 'none',
    src_dir: [
      join_paths(meson.source_root(), 'src'),
      join_paths(meson.build_root(), 'src'),
    ],
    dependencies: buoh,
    fixxref_args: [
      '--html-dir=' + docpath,
      # '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
      # '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
      # '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
      # '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gi')),
      # '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gtk3')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gdk-pixbuf')),
    ],
    install: true,
  )
endif
