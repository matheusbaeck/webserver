#!/usr/bin/env python3

import os
import sys
import cgi
import json

def save_uploaded_file(field_data, upload_dir):
    """Save the uploaded file to the specified directory."""
    filename = field_data['filename']
    content = field_data['content']
    file_path = os.path.join(upload_dir, filename)

    with open(file_path, "wb") as f:
        f.write(content)

    return file_path

def parse_multipart_form_data(content_type, body):
    """Parse multipart form data."""
    if not content_type.startswith("multipart/form-data"):
        raise ValueError("Content-Type must be 'multipart/form-data'")

    boundary = content_type.split("boundary=")[-1]
    if not boundary:
        raise ValueError("Boundary not found in Content-Type")

    boundary = f"--{boundary}".encode()
    end_boundary = f"{boundary}--".encode()

    parts = body.split(boundary)
    parsed_data = {}

    for part in parts:
        if not part.strip() or part == b"--" or part == b"--\r\n":
            continue

        try:
            headers, content = part.split(b"\r\n\r\n", 1)
        except ValueError:
            continue

        headers = headers.decode().split("\r\n")
        content = content.rstrip(b"\r\n")

        disposition = [h for h in headers if h.startswith("Content-Disposition")]
        if not disposition:
            continue

        disposition = disposition[0]
        field_name = None
        filename = None
        for item in disposition.split(";"):
            if "name=" in item:
                field_name = item.split("=", 1)[-1].strip('"')
            if "filename=" in item:
                filename = item.split("=", 1)[-1].strip('"')

        if field_name:
            if filename:
                parsed_data[field_name] = {
                    "filename": filename,
                    "content": content
                }
            else:
                parsed_data[field_name] = content.decode()

    return parsed_data

# CGI Script Starts Here
try:
    content_type = os.environ.get("CONTENT_TYPE", "")
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))

    body = sys.stdin.read(content_length).encode()
    parsed_data = parse_multipart_form_data(content_type, body)

    UPLOAD_DIR = "/home/glacroix/Documents/pushed-webserv/html/gallery-images"  # Change this to your upload directory
    os.makedirs(UPLOAD_DIR, exist_ok=True)

    uploaded_files = []
    for field_name, field_data in parsed_data.items():
        if isinstance(field_data, dict) and "filename" in field_data:
            file_path = save_uploaded_file(field_data, UPLOAD_DIR)
            uploaded_files.append(file_path)

    # Return a JSON response
    print("Content-Type: application/json\n")
    print(json.dumps({"status": "success", "files": uploaded_files}))

except Exception as e:
    # Return an error response
    print("Content-Type: application/json\n")
    print(json.dumps({"status": "error", "message": str(e)}))

