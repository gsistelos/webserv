#!/usr/bin/python3
import cgi
import os
import cgitb
cgitb.enable()
form = cgi.FieldStorage()
fileitem = form['filename']
if fileitem.filename:
    fn = os.path.basename(fileitem.filename)
    open('/tmp/' + fn, 'wb').write(fileitem.file.read())
    message = 'The file "' + fn + '" was uploaded successfully'

else:
    message = 'No file was uploaded'

print("Content-Type: text/html\r\n")
print("<html>")
print("<body>")
print("<p>Koe</p>")
print("</body>")
print("</html>")
