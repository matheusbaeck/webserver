#!/usr/bin/env python3

import json
import os

# Set the content type to application/json
print("Content-Type: application/json")
print()  # End of headers

# Create a list of image URLs (you can modify this to fetch from a database or filesystem)

image_urls = [
        "http://localhost:2525/images/42madrid.png",
        "http://localhost:2525/images/chill-guy.jpg",
]

# Create a JSON response
response = {
    "images": image_urls
}

# Print the JSON response
print(json.dumps(response))
