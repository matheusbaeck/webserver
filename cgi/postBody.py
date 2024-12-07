#!/usr/bin/env python3
import sys
import os
import json
from urllib.parse import parse_qs

# Determine content type
content_type = os.environ.get('CONTENT_TYPE', '')

# Read all input data from stdin
input_data = sys.stdin.read()

# Print response header (HTML)
print("Content-Type: text/html\n")

print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("    <meta charset='UTF-8' />")
print("    <meta name='viewport' content='width=device-width, initial-scale=1.0' />")
print("    <title>POST Data Received</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; padding: 20px; background: #fafafa; }")
print("        h1 { color: #333; }")
print("        pre { background: #f0f0f0; padding: 10px; border-radius: 4px; }")
print("        .key { font-weight: bold; color: #007BFF; }")
print("    </style>")
print("</head>")
print("<body>")
print("<h1>POST Data Received</h1>")

if 'application/json' in content_type.lower():
    # Input data should be JSON
    try:
        data = json.loads(input_data)
        print("<h2>Parsed JSON:</h2>")
        print("<pre>")
        # Pretty-print JSON
        print(json.dumps(data, indent=4))
        print("</pre>")
    except json.JSONDecodeError:
        print("<p><strong>Error:</strong> Unable to parse JSON data.</p>")
        print("<h2>Raw Input:</h2>")
        print(f"<pre>{input_data}</pre>")
elif 'application/x-www-form-urlencoded' in content_type.lower():
    # Input data is form data
    fields = parse_qs(input_data)
    print("<h2>Parsed Form Data:</h2>")
    if fields:
        for key, values in fields.items():
            print(f"<p><span class='key'>{key}:</span> {', '.join(values)}</p>")
    else:
        print("<p>No fields were parsed.</p>")
    print("<h2>Raw Input:</h2>")
    print(f"<pre>{input_data}</pre>")
else:
    # Unknown or unsupported content type, just print the raw input
    print("<h2>Raw Input (Unrecognized Content Type):</h2>")
    print(f"<pre>{input_data}</pre>")

print("</body>")
print("</html>")

