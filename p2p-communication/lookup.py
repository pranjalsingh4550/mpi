#!/usr/bin/python3
# estimates time taken by a node from info-<rank>.txt
# toy estimates - did not use this finally
from sys import argv
import csv

if (len(argv) != 3):
    print ("Usage: $ python3 lookup.py <input file> <log file>")
    print("argv is", argv)
    exit()

def create_lookup_dict():
    d={}
    count = 0
    f = open (argv[1], 'r')
    m = csv.reader(f)
    for line in m:
        size=int (line[0])
        if not size in d.keys():
            d[size] = (float(line[1]))
        count += 1
    f.close()
    # d is initialised to key-value pairs for sizes 1k to 200k
    return d;

def lookup(size):
    d = create_lookup_dict ()
    size = (size + 999) // 1000
    size = size * 1000 # Ceiling
    return d[size]

def read_file():
    d = create_lookup_dict()
    file_name = argv[2]
    f = open(file_name)
    t = f.readline ()
    t = f.readline()
    rank = t[0]
    node = t[1]

    time = 0
    while t:
        if '*' in t:
            break
        # print(t)
        u = t.split()
        destrank = int(u[3])

        time += lookup(int(u[4]))
        # print ("adding size=",int(u[4]), lookup(int(u[4])))
        t = f.readline()


    f.close()
    print(time)

read_file()
