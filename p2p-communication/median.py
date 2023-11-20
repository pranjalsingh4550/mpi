#!/usr/bin/python3
from statistics import median
from sys import argv
import csv
# computes medians of the datapoints in <input file> for each size
# CSV format - "size,time"

if (len(argv) != 3):
    print ("Usage: $ python3 median.py <input file> <output file>")
    print("argv is", argv)
    exit()

d={}
count = 0
f = open (argv[1], 'r')
m = csv.reader(f)
for line in m:
    size=int (line[0])
    if not size in d.keys():
        d[size] = []
    d[size].append(float(line[1]))
    count += 1

print (count)
o= open (argv[2], 'x')
for key in d.keys():
    d[key] = median ( d[key])
    o.write (str(key) + "," + str(d[key])+ "\n")
o.close()
f.close()
