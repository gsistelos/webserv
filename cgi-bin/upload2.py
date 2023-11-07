#!/usr/bin/python3

import os, sys

file_data = sys.stdin.buffer.read()

if file_data:
    if not os.path.exists("cgi-bin/uploads/"):
           os.mkdir("cgi-bin/uploads/")

    open("cgi-bin/uploads/file.txt", "wb").write(file_data)

    response = b"<html>"
    response += b"<p>Your file has been uploaded successfully!</p>"
    response += b"</html>"

    sys.stdout.buffer.write(b"HTTP/1.1 200 OK\r\n")
    sys.stdout.buffer.write(b"Content-Length: " + str(len(response)).encode("utf-8") + b"\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(response)
else:
    response = b"<html>"
    response += b"<p>No file was uploaded</p>"
    response += b"</html>"

    sys.stdout.buffer.write(b"HTTP/1.1 400 Bad Request")
    sys.stdout.buffer.write(b"Content-Length: " + str(len(response)).encode("utf-8") + b"\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(response)