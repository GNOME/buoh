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
  'main.c',
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

headers = files(
  'buoh-add-comic-dialog.h',
  'buoh-application.h',
  'buoh-comic-cache.h',
  'buoh-comic.h',
  'buoh-comic-list.h',
  'buoh-comic-loader.h',
  'buoh-comic-manager-date.h',
  'buoh-comic-manager.h',
  'buoh-properties-dialog.h',
  'buoh-settings.h',
  'buoh-view-comic.h',
  'buoh-view.h',
  'buoh-view-message.h',
  'buoh-window.h',
)

cflags = [
  '-DCOMICS_DIR="@0@"'.format(comicsdir),
  '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
  '-DVERSION="@0@"'.format(meson.project_version()),
  '-DLOCALE_DIR="@0@"'.format(localedir),
]

if introspection_enabled
  cflags += '-DENABLE_INTROSPECTION'
endif

link_args = []

if introspection_enabled
  # Required for GIRepository to be able to dump symbols.
  link_args += '-Wl,--export-dynamic'
endif

buoh = executable(
  meson.project_name(),
  sources,
  dependencies: buoh_deps,
  c_args: cflags,
  link_args: link_args,
  install: true,
)

if introspection_enabled
  buoh_gir = gnome.generate_gir(
    buoh,
    sources: [
      sources,
      headers,
    ],
    namespace: 'Buoh',
    nsversion: '0',
    includes: [
      'Gtk-3.0',
    ],
  )
endif
