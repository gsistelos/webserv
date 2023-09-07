#!/usr/bin/python3

import os, cgi

form = cgi.FieldStorage()

form_data = form["filename"]

if form_data.filename:
    if not os.path.exists("uploads"):
        os.mkdir("uploads")

    open("uploads/" + form_data.filename, "wb").write(form_data.file.read())

    response = "<html>"
    response += "<p>File " + form_data.filename + " was uploaded to " + "uploads/" + "</p>"
    response += "</html>"

    print("HTTP/1.1 200 OK")
    print("Content-Length: " + str(len(response)))
    print()
    print(response)
else:
    response = "<html>"
    response += "<p>No file was uploaded</p>"
    response += "</html>"

    print("HTTP/1.1 400 Bad Request")
    print("Content-Length: " + str(len(response)))
    print()
    print(response)
