project(
  'buoh',
  'c',
  license: 'GPL-2.0-or-later',
  version: '0.8.3',
  default_options: [
    'b_ndebug=if-release',
  ],
)

gnome = import('gnome')
i18n = import('i18n')
data_dir = join_paths(meson.current_source_dir(), 'data')
po_dir = join_paths(meson.current_source_dir(), 'po')

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
icondir = join_paths(datadir, 'icons', 'hicolor')
localedir = join_paths(prefix, get_option('localedir'))
pkgdatadir = join_paths(datadir, meson.project_name())
schemadir = join_paths(datadir, 'glib-2.0', 'schemas')
comicsdir = join_paths(pkgdatadir, 'comics')

# Dependencies
gtk = dependency('gtk+-3.0', version: '>= 3.22.0')
glib = dependency('glib-2.0', version: '>= 2.70.0')
libsoup = dependency('libsoup-2.4', version: '>= 2.4.0')
libxml2 = dependency('libxml-2.0', version: '>= 2.4.0')

python3 = find_program('python3')
appstream_util = find_program('appstream-util', required: false)
desktop_file_validate = find_program('desktop-file-validate', required: false)
gtk_builder_tool = find_program('gtk-builder-tool', required: false)
xmllint = find_program('xmllint', required: false)
xsltproc = find_program('xsltproc', required: false)

buoh_deps = [
  gtk,
  glib,
  libsoup,
  libxml2,
]

if appstream_util.found()
  meson.add_dist_script(
    python3,
    join_paths(meson.current_source_dir(), 'build-aux', 'dist-news.py'),
    appstream_util.full_path(),
  )
endif

# Post-install scripts
meson.add_install_script(
  python3,
  join_paths(meson.source_root(), 'build-aux', 'post-install.py'),
  icondir,
  schemadir,
)

# Subfolders
subdir('src')
subdir('data')
subdir('po')
