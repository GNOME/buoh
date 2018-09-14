if get_option('gtk_doc')
  version_conf = configuration_data()
  version_conf.set('BUOH_VERSION', meson.project_version())
  buoh_docs = configure_file(
    input: 'buoh-docs.xml.in',
    output: 'buoh-docs.xml',
    configuration: version_conf,
  )

  gnome.gtkdoc(
    'buoh',
    main_xml: join_paths(meson.current_build_dir(), 'buoh-docs.xml'),
    namespace: 'buoh',
    mode: 'none',
    src_dir: join_paths(meson.source_root(), 'src'),
    dependencies: buoh,
    fixxref_args: [
      '--html-dir=' + docpath,
    ],
    install: true,
  )
endif
