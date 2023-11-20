#!/usr/bin/python3
# finds median for each node for 5 message sizes. writes to a file
import csv
from statistics import median
import os
from sys import argv
args = argv
if (len(args) != 1):
	print ("Usage: python3 combine.py")
	exit ()
# rank = int(args[1])
ls = os.listdir()
csvs = []
for t in ls:
	if (t.startswith("time_from_")):
		csvs.append (t)

datapoints = {} # datapoints[1024] = [0.0003,0.000234,..] # for the same rank
union = {}  # to find system-wide latency and bandwidth
for t in csvs:
	datapoints = {}
	u = t.removeprefix ("time_from_")
	u = u.partition ("_")
	node = u[0]
	if (len(u) > 2):
		rank = u[2]
	pairs = {}
	file = open (t)
	rd = csv.reader (file)
    
	median_file = open (f"median_from_{node}", 'w')
	print ("writing to", f"median_from_{node}", end = '\t')
    
	count = 0
	for line in rd:
		size=int (line[-2])
		if not size in datapoints.keys():
			datapoints[size] = []
		datapoints[size].append(float(line[-1]))
		if not size in union.keys():
			union[size] = []
		union[size].append ( float (line[-1]) )
		count += 1
	
	#now datapoints is complete. find medians
	for size in datapoints.keys():
		datapoints[size] = median (datapoints[size])
		median_file.write ( f"{size},{datapoints[size]:.7f}\n")
	print(f"{datapoints[4096]:.6f} {datapoints[16384]:.6f} {datapoints[65536]:.6f} {datapoints[131072]:.6f} {datapoints[1048576]:.6f} ")
	median_file.close ()

# system-wide network parameters:

median_file = open ( "median_from_all", 'w')
for size in union.keys():
	union[size] = median (union[size])
	median_file.write ( f"{size},{union[size]:.7f}\n")
print(f"Writing to median_from_all:\t{union[4096]:.6f} {union[16384]:.6f} {union[65536]:.6f} {union[131072]:.6f} {union[1048576]:.6f} ")
print ("Check for outliers!")
median_file.close ()
