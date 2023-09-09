#!/usr/bin/python3

import cgi
import os

os.environ["REQUEST_METHOD"] = "POST"

form = cgi.FieldStorage()

# Get filename here
fileitem = form['filename']

# Test if the file was uploaded
if fileitem.filename:
    # Check if directory exists
    if not os.path.exists(os.getcwd() + '/cgi-bin/tmp'):
        os.makedirs(os.getcwd() + '/cgi-bin/tmp')

    # Create and write to file
    open(os.getcwd() + '/cgi-bin/tmp/' +
         os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
    message = 'The file "' + \
        os.path.basename(fileitem.filename) + \
        '" was uploaded to ' + os.getcwd() + '/cgi-bin/tmp'
else:
    message = 'Uploading Failed'

response = "<html>\n"
response += "<p>" + message + "</p>\n"
response += "</html>\n"

# Response to client
print("HTTP/1.1 200 OK")
print("Content-Type: text/html")
print("Content-Length: ", len(message))
print()
print(response)
