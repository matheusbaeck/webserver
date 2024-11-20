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

    # Extract the first two key-value pairs dynamically, or provide defaults if missing
    keys = list(params.keys())
    value1 = params.get(keys[0], 'Someone').capitalize() if len(keys) > 0 else 'Someone'
    value2 = params.get(keys[1], '').capitalize() if len(keys) > 1 else ''

    # Format the output based on available values
    if value1 and value2:
        greeting = f"What's up {value1} {value2}!!!"
    elif value1:
        greeting = f"What's up {value1}!!!"
    else:
        greeting = "What's up!!!"

    # Output the HTTP headers
    print("Content-Type: text/html")
    print()  # Blank line to separate headers from body

    # Output the HTML content
    print("<html><body>")
    print(f"<h1>{greeting}</h1>")
    print("</body></html>")

if __name__ == "__main__":
    main()

