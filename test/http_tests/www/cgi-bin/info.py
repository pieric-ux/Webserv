#!/usr/bin/env python3
import os,sys
n=int(os.environ.get("CONTENT_LENGTH") or 0)
b=sys.stdin.read(n) if n else ""
print("Content-Type: text/plain");print()
print("METHOD="+str(os.environ.get("REQUEST_METHOD")))
print("QUERY="+str(os.environ.get("QUERY_STRING")))
print("CLEN="+str(os.environ.get("CONTENT_LENGTH")))
print("BODY="+b)
