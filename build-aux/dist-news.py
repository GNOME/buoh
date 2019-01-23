import subprocess
import sys

if len(sys.argv) < 2:
  sys.exit("usage: dist-news.py <appstream_util>")

appstream_util = sys.argv[1]

print('Generating NEWS file…')
subprocess.call([appstream_util, 'appdata-to-news', 'data/org.gnome.buoh.appdata.xml.in'])
