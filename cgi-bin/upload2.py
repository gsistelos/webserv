#!/usr/bin/python3

import os, sys

# Determine the content length from the HTTP headers
content_length = int(os.environ.get("CONTENT_LENGTH", 0))

if content_length > 0:
    # Read the binary data from stdin
    file_data = sys.stdin.buffer.read(content_length)

    # Handle the uploaded file
    if file_data:
        if not os.path.exists("cgi-bin/uploads/"):
            os.mkdir("cgi-bin/uploads/")

        # You can generate a unique filename or use the original filename
        # For simplicity, let's assume the filename is passed in a query parameter
        filename = os.environ.get("QUERY_STRING", "uploaded_file")

        with open("cgi-bin/uploads/file.txt", "wb") as uploaded_file:
            uploaded_file.write(file_data)

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
else:
    response = b"<html>"
    response += b"<p>No content length provided</p>"
    response += b"</html>"

    sys.stdout.buffer.write(b"HTTP/1.1 400 Bad Request")
    sys.stdout.buffer.write(b"Content-Length: " + str(len(response)).encode("utf-8") + b"\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(response)
