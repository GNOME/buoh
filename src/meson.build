settings_headers = files(
  'buoh-settings.h',
)
enum_headers = settings_headers + files(
  'buoh-view.h',
)

sources = files(
  'buoh-add-comic-dialog.c',
  'buoh-application.c',
  'buoh-comic-cache.c',
  'buoh-comic-list.c',
  'buoh-comic-loader.c',
  'buoh-comic-manager-date.c',
  'buoh-comic-manager.c',
  'buoh-comic.c',
  'buoh-properties-dialog.c',
  'buoh-view-comic.c',
  'buoh-view-message.c',
  'buoh-view.c',
  'buoh-window.c',
  'buoh.vapi',
  'main.vala',
)

headers = settings_headers + files(
  'buoh-add-comic-dialog.h',
  'buoh-application.h',
  'buoh-comic-cache.h',
  'buoh-comic-list.h',
  'buoh-comic-loader.h',
  'buoh-comic-manager-date.h',
  'buoh-comic-manager.h',
  'buoh-comic.h',
  'buoh-properties-dialog.h',
  'buoh-settings.h',
  'buoh-view-comic.h',
  'buoh-view-message.h',
  'buoh-view.h',
  'buoh-window.h',
)

sources += gnome.compile_resources(
  'buoh-resources',
  join_paths(data_dir, 'org.gnome.buoh.gresource.xml'),
  source_dir: [
    data_dir,
  ],
  c_name: 'buoh',
  export: true,
)

sources += gnome.mkenums_simple(
  'buoh-enums',
  sources: enum_headers,
)

cflags = [
  '-DCOMICS_DIR="@0@"'.format(comicsdir),
  '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
  '-DVERSION="@0@"'.format(meson.project_version()),
  '-DLOCALE_DIR="@0@"'.format(localedir),
]

noinst_lib = shared_library(
  'noinst',
  headers,
  install: false,
)

gir = gnome.generate_gir(
  noinst_lib,
  sources: headers,
  nsversion: '1.0',
  namespace: 'Buoh',
  identifier_prefix: 'Buoh',
  dependencies: buoh_deps,
  extra_args: [
    '--header-only',
  ],
  symbol_prefix: 'buoh',
)

vapi = gnome.generate_vapi(
  'buoh',
  sources: gir[0],
)

buoh_deps += vapi

buoh = executable(
  meson.project_name(),
  sources,
  dependencies: buoh_deps,
  c_args: cflags,
  install: true,
)
