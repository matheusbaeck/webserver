#!/usr/bin/env python3

import cgi
import os
import json
import sys

# Define the path to the comments file
comments_file = '/home/glacroix/Documents/pushed-webserv/comments/db.txt'

# Function to read comments from the file
def read_comments():
    if os.path.exists(comments_file):
        with open(comments_file, 'r') as f:
            return f.readlines()
    return []

# Function to write a comment to the file
def write_comment(comment):
    with open(comments_file, 'a') as f:
        f.write(comment + '\n')

# Handle the CGI request
print("Content-Type: application/json")
print()

if os.environ['REQUEST_METHOD'] == 'POST':
    # Parse the form data
    form = cgi.FieldStorage()
    comment = form.getvalue('comment')
    if comment:
        write_comment(comment.strip())
    print(json.dumps({"status": "success"}))
else:
    # Default to GET: Return the list of comments
    comments = read_comments()
    print(json.dumps(comments))

