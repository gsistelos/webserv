#!/usr/bin/python3

import cgi
import os

form = cgi.FieldStorage()

# Get filename here
fileitem = form['filename']

# Test if the file was uploaded
if fileitem.filename:
    open(os.getcwd() + '/cgi-bin/tmp/' +
         os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
    message = 'The file "' + \
        os.path.basename(fileitem.filename) + \
        '" was uploaded to ' + os.getcwd() + '/cgi-bin/tmp'
else:
    message = 'Uploading Failed'

# Response to client
print("Content-Type: text/html\n")
print("<html>")
print("<body>")
print("<p>", message, "</p>")
print("<body>")
print("<html>")
