#!/usr/bin/env python3

import json
import os
import sys

# Print the CGI response headers
print("Content-Type: application/json")
print()

# Directory containing images (change this to your actual images directory)
image_directory = "/home/glacroix/Documents/pushed-webserv/html/gallery-images" #str(os.getenv('HOME')) + "/p"

# Base URL for accessing images (adjust host/port and path as needed)
base_url = "http://localhost:2525/images/"

# Supported image file extensions
image_extensions = {".png", ".jpg", ".jpeg", ".gif", ".webp"}

images = [
]

# Ensure the directory exists to avoid errors
if not os.path.isdir(image_directory):
    # If the directory doesn't exist, return an empty list or an error message
    response = { "images": [] }
    print(json.dumps(response))
    sys.exit(0)

# List the files in the image directory and filter by supported extensions
for filename in os.listdir(image_directory):
    #print(filename);
    ext = os.path.splitext(filename)[1].lower()
    if ext in image_extensions:
        images.append(base_url + filename)

# Create a JSON response
response = {"images": images}

# Print the JSON response
print(json.dumps(response))

