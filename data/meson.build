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
if (desktop_file_validate.found())
  test('Validating ' + desktop,
    desktop_file_validate,
    args: [desktop],
    workdir: meson.current_build_dir(),
  )
endif

install_data(
  'comics.xml',
  install_dir: comicsdir,
)

appdata = 'org.gnome.buoh.appdata.xml'
i18n.merge_file(
  appdata,
  input: appdata + '.in',
  output: appdata,
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(datadir, 'metainfo'),
)
if (appstream_util.found())
  test(
    'Validating ' + appdata,
    appstream_util,
    args: ['validate', '--nonet', appdata],
    workdir: meson.current_build_dir(),
  )
endif

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
