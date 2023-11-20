#!/usr/bin/python3
import os
import sys
import csv

# HOME="/home/pranjal/temp/"
# HOME = "/users/btech/prsingh/temp/"
HOME = sys.argv[1]

ls = os.listdir(HOME)
lsinfo = []
# print ("received directory")
# print(ls)

for t in ls:
	if t.startswith("info-"):
		lsinfo.append (t)
# print(lsinfo)
size = len(lsinfo)
def findrank (f):
	return int ( f.removeprefix('info-').removesuffix ('.txt'))
def findname (r):
	return "info-" + str(r) + ".txt"
# make many file objects and read in parallel
# for t in lsinfo:
# 	print ( findrank (t))
lastline = [''] * size
fobj = []
currstep = [0.0] * size # time in this step of a certain collective
steps = []
# print (lastline, currstep)

for t in range (size):
	# print ("opening\t", HOME + findname (t))
	fobj.append ( open (HOME + "/" +findname (t), "r") )
	fobj[-1].readline () # rank and processor name
	steps.append ( fobj[t].readlines())
# for j in steps:
	# print ("---------------------------------------------------------")
	# print (j)
	# print ("---------------------------------------------------------")

# start here

coll = 1
step = 0
time_so_far = 0.0;
lineindex = [0] * size # the index of the current element in the readlines() array for each rank
completed = [False] * size
rank_done_with_collective = [False] * size
position = -1 # -1 for realtime, -2 for historical data
count = 0
while (completed != [True] * size):
	# read through the next set of steps from each file
	count += 1
	if (count == 20): break
	flag = False
	
	for rank in range (size):
		index = lineindex[rank]
		if rank_done_with_collective[rank]:
			currstep [rank] = 0.0
			# print (f"{rank}, aaa")
		elif (steps[rank][index].startswith (f"{coll} {step}")):
			currstep[rank] = (steps[rank][index].split())[position]
			currstep[rank] = int (currstep[rank]) /10000000
			lineindex[rank] += 1
			# print (f"{rank}, bbb")
		elif steps[rank][index].startswith (f"{coll} "):
			currstep [rank] = 0.0
			# print (rank, "ccc")
		elif steps[rank][index].startswith ("*"):
			currstep [rank] = 0.0
			rank_done_with_collective[rank] = True
			completed[rank] = True
			# print (rank, "ddd")
		else :
			# this rank has reported all steps of this collective but not the full program
			rank_done_with_collective[rank] = True
			# print (rank, "eee")
			currstep [rank] = 0.0
	# currstep constructed
	# print ("Currstep is", currstep, "step", step, "coll", coll)
	# print (rank_done_with_collective)
	# print (completed)
	# print (lineindex)
	time_so_far += max (currstep)
	step = step + 1
	# print (f"%%%%%%%%%%%%%%%%%%%% time so far {time_so_far:.7f} %%%%%%%%%%%%%%%%%%%%")
	

	if (rank_done_with_collective == [True] * size):
		coll += 1
		step = 0
		rank_done_with_collective = [False] * size
		# print("\n\n")
	# main

print (f"{time_so_far:.7f}", end = "")
# end here
for j in fobj:
	j.close()
