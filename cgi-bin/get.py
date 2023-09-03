#!/usr/bin/python3

import os
import sys

# Set environment variables
os.environ["REQUEST_METHOD"] = "GET"

# Get the file path from the environment variable
file_path = os.environ["REQUEST_URI"]

# Print to stderr
print(file_path, file=sys.stderr)

# Open and read the HTML file
try:
    with open(file_path, "r") as file:
        file_content = file.read()
except FileNotFoundError:
    # Handle the case where the file does not exist
    file_content = "File not found."

# Response to client
print(file_content)
