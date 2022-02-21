import os
from pathlib import Path
import subprocess
import sys

if len(sys.argv) < 2:
  sys.exit("usage: dist-news.py <appstream_util>")

appstream_util = sys.argv[1]

print('Generating NEWS file…')
appstream_path = Path(os.environ['MESON_PROJECT_SOURCE_ROOT']) / 'data/org.gnome.buoh.appdata.xml.in'
news = subprocess.check_output([appstream_util, 'appdata-to-news', appstream_path])

news_path = Path(os.environ['MESON_PROJECT_DIST_ROOT']) / 'NEWS'
with open(news_path , 'wb') as news_file:
    news_file.write(news)
