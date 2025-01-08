#!/usr/bin/python3

import json
import os
import sys

print("Content-Type: application/json")
print()

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
upload_directory = os.path.join(project_root, 'uploaded-files')

base_url = "http://localhost:2525/upload/"

file_extensions = { 
    ".txt", ".doc", ".docx", ".pdf", ".odt", ".rtf", ".md",
    ".xls", ".xlsx", ".csv", ".ods",
    ".ppt", ".pptx", ".odp",
    ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".svg",
    ".mp3", ".wav", ".aac", ".flac", ".ogg",
    ".mp4", ".mkv", ".avi", ".mov", ".wmv",
    ".zip", ".rar", ".tar", ".gz", ".7z",
    ".exe", ".bat", ".sh", ".app", ".jar",
    ".html", ".htm", ".css", ".js", ".php", ".asp",
    ".sql", ".db", ".mdb", ".accdb", ".sqlite",
    ".c", ".cpp", ".py", ".java", ".rb",
    ".dll", ".sys", ".ini", ".log",
    ".iso", ".dmg", ".epub", ".torrent"
}

files  = []


if not os.path.isdir(upload_directory):
    # If the directory doesn't exist, return an empty list or an error message
    response = { "files": [] }
    print(json.dumps(response))
    sys.exit(0)

# List the files in the image directory and filter by supported extensions
for filename in os.listdir(upload_directory):
    #print(filename);
    ext = os.path.splitext(filename)[1].lower()
    if ext in file_extensions:
        files.append(base_url + filename)

# Create a JSON response
response = {"files": files}

# Print the JSON response
print(json.dumps(response))

