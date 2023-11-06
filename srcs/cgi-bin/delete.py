#!/usr/bin/env python3

# Import modules for CGI handling
import sys, os, cgi

form = cgi.FieldStorage()

filename = form.getvalue('filename')

try:
    os.remove("uploads/" + filename)
    response = b"<html>"
    response += b"<p>File was successfully deleted</p>"
    response += b"</html>"

    sys.stdout.buffer.write(b"HTTP/1.1 200 OK\r\n")
    sys.stdout.buffer.write(b"Content-Length: " + str(len(response)).encode("utf-8") + b"\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(response)
except OSError:
    response = b"<html>"
    response += b"<p>File could not be deleted</p>"
    response += b"</html>"

    sys.stdout.buffer.write(b"HTTP/1.1 400 Bad Request\r\n")
    sys.stdout.buffer.write(b"Content-Length: " + str(len(response)).encode("utf-8") + b"\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(response)