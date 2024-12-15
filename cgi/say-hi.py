#!/usr/bin/env python3


import sys

# Write standard HTTP response to stdout
print("Content-Type: text/html")
print("")
print("<html><body>")
print("<p>Hello World!</p>")
print("</body></html>")

# Write debug information or errors to stderr
sys.stderr.write("Debug: CGI script executed successfully.\n")
sys.stderr.flush()  # Ensure the message is flushed to stderr
