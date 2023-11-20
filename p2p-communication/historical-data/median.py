from statistics import median
from sys import argv
import csv

if (len(argv) != 3):
    print ("Usage: $ python3 median.py <input file> <output file>")
    print("argv is", argv)
    exit()

d={} # dict of dicts
count = 0
f = open (argv[1], 'r')
m = csv.reader(f)
for line in m:
    #csv format is source,dest,size,time
    destination=line[1]
    if not destination in d.keys():
        d[destination] = {}
    size=int (line[2])
    if not size in (d[destination]).keys():
        d[destination][size] = []
    d[destination][size].append(float(line[3]))
    count += 1

print (count)
o= open (argv[2], 'x')
for dest in d.keys():
    for key in d[dest].keys():
        d[dest][key] = median ( d[dest][key])
        fl = f"{d[dest][key]:.6f}"
        o.write (dest + '\t' + str(key) + "\t" + fl + "\n")
o.close()
f.close()
