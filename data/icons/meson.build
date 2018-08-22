icon_sizes = [
  '16x16',
  '22x22',
  '24x24',
  '32x32',
  '48x48',
  '64x64',
]

foreach icon_size : icon_sizes
  install_data(
    join_paths(icon_size, 'buoh.png'),
    install_dir: join_paths(datadir, 'icons', 'hicolor', icon_size, 'apps'),
  )
endforeach
