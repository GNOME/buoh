install_data(
  'buoh-ui.xml',
  install_dir: uidir,
)

desktop = 'buoh.desktop'
i18n.merge_file(
  desktop,
  type: 'desktop',
  input: desktop + '.in',
  output: desktop,
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(datadir, 'applications'),
)

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