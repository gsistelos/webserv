#!/usr/bin/env python3

# Import modules for CGI handling
import cgi, cgitb, sys

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
first_name = form.getvalue('first_name')
last_name = form.getvalue('last_name')

content = "<html>"
content += "<head>"
content += "<title>Success!</title>"
content += "</head>"
content += "<body>"
content += "<h2>Hello %s %s, your data has been registered!</h2>" % (first_name, last_name)
content += "</body>"
content += "</html>"

print("HTTP/1.1 200 OK")
print ("Content-type:text/html")
print ("Content-Length: " + str(len(content)))
print()
print(content)