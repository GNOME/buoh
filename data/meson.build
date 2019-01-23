ui_files = files(
  'ui/add-comic-dialog.ui',
  'ui/menus.ui',
  'ui/comic-list.ui',
  'ui/properties-dialog.ui',
  'ui/view.ui',
  'ui/view-comic.ui',
  'ui/view-message.ui',
  'ui/window.ui',
)
if gtk_builder_tool.found() and xsltproc.found()
  foreach ui : ui_files
    # gtk-builder-tool validate does not support custom widgets
    ui_for_validation = configure_file(
      input: ui,
      output: '@PLAINNAME@',
      command: [
        xsltproc,
        join_paths(meson.source_root(), 'build-aux', 'ui-pre-validation.xsl'),
        '@INPUT@',
      ],
      capture: true,
    )

    test(
      'Validating @0@'.format(ui),
      gtk_builder_tool,
      args: ['validate', ui_for_validation],
      workdir: meson.current_build_dir(),
    )
  endforeach
endif

desktop = 'org.gnome.buoh.desktop'
i18n.merge_file(
  desktop,
  type: 'desktop',
  input: desktop + '.in',
  output: desktop,
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(datadir, 'applications'),
)
if desktop_file_validate.found()
  test(
    'Validating @0@'.format(desktop),
    desktop_file_validate,
    args: [desktop],
    workdir: meson.current_build_dir(),
  )
endif

comics = 'comics.xml'
install_data(
  comics,
  install_dir: comicsdir,
)
if xmllint.found()
  test(
    'Validating @0@'.format(comics),
    xmllint,
    args: ['--noout', '--schema', 'comic_list.xsd', comics],
    workdir: meson.current_source_dir(),
  )
endif

gnome.mkenums(
  'org.gnome.buoh.enums.xml',
  sources: settings_headers,
  comments: '<!-- @comment@ -->',
  fhead: '<schemalist>',
  vhead: '  <@type@ id="org.gnome.buoh.@EnumName@">',
  vprod: '    <value nick="@valuenick@" value="@valuenum@"/>',
  vtail: '  </@type@>',
  ftail: '</schemalist>',
  install_header: true,
  install_dir: schemadir,
)

install_data(
  'org.gnome.buoh.gschema.xml',
  install_dir: schemadir,
)

subdir('icons')
