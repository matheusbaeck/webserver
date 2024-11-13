#!/usr/bin/env python3

def parse_query_string(query_string):
    params = {}
    if query_string:
        pairs = query_string.split('&')
        for pair in pairs:
            key, value = pair.split('=')
            params[key] = value
    return params

def main():
    # Get the query string from the environment
    import os
    query_string = os.environ.get('QUERY_STRING', '')

    # Parse the query string
    params = parse_query_string(query_string)

    # Output the HTTP headers
    print("Content-Type: text/html")
    print()  # Blank line to separate headers from body

    # Output the HTML content
    print("<html><body>")
    print("<h1>Received Parameters</h1>")
    print("<ul>")
    for key, value in params.items():
        print(f"<li>{key}: {value}</li>")
    print("</ul>")
    print("</body></html>")

if __name__ == "__main__":
    main()

