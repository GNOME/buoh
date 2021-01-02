#!/usr/bin/env python3
import datetime
import sys
import xml.etree.ElementTree as ET
import requests

if len(sys.argv) < 2:
    sys.exit('usage: check-comic-links.py <comics.xml>')

tree = ET.parse(sys.argv[1])
comic_list = tree.getroot()

errors = False

for comic in comic_list:
    comic_class = comic.attrib['class']
    if comic_class == 'date':
        first_uri = datetime.datetime.strptime(comic.attrib['first'], '%Y-%m-%d').strftime(comic.attrib['generic_uri'])
        response = requests.get(first_uri, allow_redirects=False)
        if response.status_code != 200:
            title = comic.attrib['title']
            print(f'Comic “{title}” returned {response.status_code} for {first_uri}', file=sys.stderr)
            errors = True

if errors:
    sys.exit(1)
