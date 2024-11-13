#!/usr/bin/env python3

localvars_table = '<table>'
for x in dir():
  localvars_table += '<tr><td>%s</td></tr>' % x
localvars_table += '</table>'

print("Content-Type: text/html")
print("")
print("""<html><body>
<p>Hello World! Your custom CGI script is working. Here are your current Python local variables.</p>
%s
<p>NOTE: If you want to write useful CGI script, try the Python 'cgi' module. See cgitest.py script.</p>
</body></html>""" % (localvars_table))
