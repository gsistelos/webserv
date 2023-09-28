#!/usr/bin/env python3

import os
import sys

# Read the filename from the query string
query_string = os.environ.get('QUERY_STRING', '')
filename = query_string.split('=')[1]

# Check if the file exists in the uploads directory
filepath = os.path.join('uploads/', filename)
if not os.path.isfile(filepath):
    print('File not found')
    sys.exit(0)

# Read the contents of the file
with open(filepath, 'r') as f:
    content = f.read()

# Return the contents of the file as a response
print('HTTP/1.1 200 OK')
print('Content-Type: text/plain')
print()
print(content)