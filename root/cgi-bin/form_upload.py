#!/usr/bin/python3

import os, cgi, sys

# body = sys.stdin.buffer.read()

# print("CGI BODY: ", body.decode('utf-8'), file=sys.stderr)

form = cgi.FieldStorage()

form_data = form["filename"]

if form_data.filename:
    if not os.path.exists("uploads"):
        os.mkdir("uploads")

    file_content = form_data.file.read()

    open("uploads/" + form_data.filename, "wb").write(file_content)

    sys.stdout.buffer.write(b"HTTP/1.1 200 OK\r\n")
    sys.stdout.buffer.write(b"Content-Length: " + str(len(file_content)).encode("utf-8") + b"\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(file_content)
else:
    response = "<html>"
    response += "<p>No file was uploaded</p>"
    response += "</html>"

    sys.stdout.buffer.write(b"HTTP/1.1 400 Bad Request")
    sys.stdout.buffer.write(b"Content-Length: " + str(len(response)).encode("utf-8") + b"\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(response)
