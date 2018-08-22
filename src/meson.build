enum_headers = files('buoh-settings.h')

sources = files(
  'buoh-add-comic-dialog.c',
  'buoh.c',
  'buoh-comic.c',
  'buoh-comic-cache.c',
  'buoh-comic-list.c',
  'buoh-comic-loader.c',
  'buoh-comic-manager.c',
  'buoh-comic-manager-date.c',
  'buoh-properties-dialog.c',
  'buoh-view.c',
  'buoh-view-comic.c',
  'buoh-view-message.c',
  'buoh-window.c',
  'main.c',
)

cflags = [
  '-DUI_DIR="@0@"'.format(uidir),
  '-DCOMICS_DIR="@0@"'.format(comicsdir),
  '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
  '-DVERSION="@0@"'.format(meson.project_version()),
  '-DLOCALE_DIR="@0@"'.format(localedir),
]

buoh = executable(
  meson.project_name(),
  sources,
  dependencies: buoh_deps,
  c_args: cflags,
  install: true,
  install_dir: bindir,
)
