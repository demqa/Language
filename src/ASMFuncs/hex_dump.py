#!/usr/bin/env python3
import sys

file = open(sys.argv[1], "rb")

for line in file:
    for char in line:
        print(hex(char), sep="", end=", ")
    print()
