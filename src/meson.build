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

libbuoh = static_library(
  'buoh',
  sources + headers,
  dependencies: buoh_deps,
  c_args: cflags,
  install: false,
)

gir = gnome.generate_gir(
  libbuoh,
  sources: sources + headers,
  includes: ['Gio-2.0', 'GLib-2.0', 'GdkPixbuf-2.0', 'Gtk-3.0'],
  nsversion: '1.0',
  namespace: 'Buoh',
  identifier_prefix: 'Buoh',
  header: 'buoh.h',
  dependencies: buoh_deps,
  extra_args: [
    '--warn-all',
  ],
  symbol_prefix: 'buoh',
  install: false,
)

vala_flags = ['--gresourcesdir=' + join_paths(meson.current_source_dir(), '..', 'data')]

buoh_deps += gnome.generate_vapi(
  'buoh',
  sources: gir[0],
  packages: ['gio-2.0', 'glib-2.0', 'gdk-pixbuf-2.0', 'gtk+-3.0'],
)

libbuoh_dep = declare_dependency(link_with: libbuoh, dependencies: buoh_deps)

buoh = executable(
  meson.project_name(),
  'main.vala',
  'buoh.vapi',
  dependencies: libbuoh_dep,
  c_args: cflags,
  vala_args: vala_flags,
  install: true,
)
