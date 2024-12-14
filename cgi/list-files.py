#!/usr/bin/python3

#!/usr/bin/python3

import os
import cgi

print("Content-Type: text/html")
print()

path = "/home/glacroix/Documents/pushed-webserv/uploaded-files/"
try:
    files = os.listdir(path)
    if len(cgi.FieldStorage()) > 0:
        file_name = cgi.FieldStorage().keys()[0]
        file_path = os.path.join(path, file_name)
        if os.path.isfile(file_path):
            with open(file_path, 'r') as file:
                print("<h2>File Contents:</h2>")
                print("<pre>")
                print(file.read())
                print("</pre>")
        else:
            print("File not found")
    else:
        print("<h2>Files in Directory:</h2>")
        print("<ul>")
        for file in files:
            print("<li><a href='" + path "/" + file + "'>" + file + "</a></li>")
        print("</ul>")
except FileNotFoundError:
    print("Directory not found")
except PermissionError:
    print("Permission denied")

