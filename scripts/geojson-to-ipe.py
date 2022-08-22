#!/usr/bin/python3

import json
import sys

if len(sys.argv) != 2:
	print('Usage: geojson-to-ipe.py <geojson_file>', file=sys.stderr)
	sys.exit(1)

geojson_file_name = sys.argv[1]

with open(geojson_file_name) as geojson_file:
	geojson = json.load(geojson_file)

if geojson['type'] != 'FeatureCollection':
	print('Error: geojson file is not a FeatureCollection', file=sys.stderr)
	sys.exit(1)

print("""<?xml version="1.0"?>
<!DOCTYPE ipe SYSTEM "ipe.dtd">
<ipe version="70218">
<page>""")

for feature in geojson['features']:
	if feature['type'] != 'Feature':
		continue
	if feature['properties']['ADM0_ISO'] == '-99':
		continue
	if feature['geometry']['type'] != 'MultiPolygon':
		continue
	print('{} -> {}'.format(feature['properties']['ADM0_ISO'], feature['properties']['NAME_LONG']), file=sys.stderr)
	polygons = feature['geometry']['coordinates']
	print('<path>')
	for polygon in polygons:
		for ring in polygon:
			first = True
			for coordinate in ring:
				if first:
					print('{} {} m'.format(coordinate[0], coordinate[1]))
					first = False
				else:
					print('{} {} l'.format(coordinate[0], coordinate[1]))
			print('h')
	print('</path>')

print("""</page>
</ipe>""")
