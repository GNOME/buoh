ui_files = files(
  'ui/add-comic-dialog.ui',
  'ui/properties-dialog.ui',
  'ui/view-comic.ui',
  'ui/view-message.ui',
)
if gtk_builder_tool.found()
  foreach ui : ui_files
    test(
      'Validating @0@'.format(ui),
      gtk_builder_tool,
      args: ['validate', ui],
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

install_data(
  'comics.xml',
  install_dir: comicsdir,
)

gnome.mkenums(
  'org.gnome.buoh.enums.xml',
  sources: enum_headers,
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
