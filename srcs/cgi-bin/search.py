#!/usr/bin/env python3

# Import modules for CGI handling
import sys, os, cgi

form = cgi.FieldStorage()

filename = form.getvalue('filename')

if os.path.exists("uploads/" + filename):
	file_content = open("uploads/" + filename, "rb").read()

	sys.stdout.buffer.write(b"HTTP/1.1 200 OK\r\n")
	sys.stdout.buffer.write(b"Content-Length: " + str(len(file_content)).encode("utf-8") + b"\r\n")
	sys.stdout.buffer.write(b"\r\n")
	sys.stdout.buffer.write(file_content)
else:
	response = b"<html>"
	response += b"<p>No file was uploaded</p>"
	response += b"</html>"

	sys.stdout.buffer.write(b"HTTP/1.1 400 Bad Request")
	sys.stdout.buffer.write(b"Content-Length: " + str(len(response)).encode("utf-8") + b"\r\n")
	sys.stdout.buffer.write(b"\r\n")
	sys.stdout.buffer.write(response)