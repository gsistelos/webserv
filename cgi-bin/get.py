#!/usr/bin/python3

import os
import sys

# Set environment variables
os.environ["REQUEST_METHOD"] = "GET"

# Get the file path from the environment variable
file_path = os.environ["REQUEST_URI"]

# Print to stderr
print(file_path, file=sys.stderr)

if file_path == "google.com/":
    print("HTTP/1.1 301 Moved Permanently")
    print("Location: https://www.google.com/")
    print()
    sys.exit()

if file_path == "/":
    file_path = os.environ["SERVER_ROOT"] + "/index.html"
    print(file_path, file=sys.stderr)

elif file_path[0] == '/':
    file_path = os.environ["SERVER_ROOT"] + file_path
    print(file_path, file=sys.stderr)

# Open and read the HTML file
try:
    with open(file_path, "r") as file:
        file_content = file.read()
except FileNotFoundError:
    # Handle the case where the file does not exist
    file_content = "File not found."

# Response to client
print("HTTP/1.1 200 OK")
print("Content-Type: text/html")
print("Content-Length: ", len(file_content))
print()
print(file_content)
