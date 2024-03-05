#!/usr/bin/python3
"""Output certificate file in header file format """

import sys

file_to_open = sys.argv[1]

with open(file_to_open, 'r', encoding="utf8") as cert_file:
    cert_lines = cert_file.readlines()

    for line_num in range(len(cert_lines)-1):
        print("\""+cert_lines[line_num].strip()+"\\n\"\\")
    print("\""+cert_lines[len(cert_lines)-1].strip()+"\\n\"")
