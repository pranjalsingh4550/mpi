#!/home/pranjal/anaconda3/bin/python3
from matplotlib import pyplot as plt
import csv
import sys
if (len(sys.argv) == 1):
	print ("Wrong usage")
	exit(10)

fobj = open (sys.argv[1], 'r')
rd = csv.reader (fobj)

flag = False # plot historical data if true
if (len (sys.argv) == 3):
	print (sys.argv)
	flag = True
x = []
x_act = []
y_hist = []
y_dyn = []
y_act = []
for line in rd:
	size = int(line[0])
	hist = float(line[1])
	dynamic = float(line[2])
	actual = line[3:]
	x.append(size)
	y_hist.append (hist)
	y_dyn.append (dynamic)
	x_act = x_act + [size] * len (actual)
	# y_act = y_act + actual
	for t in actual:
		y_act.append (float(t))
# print (x)
# print(y_hist)
# print(y_dyn)
# print(x_act)
# print (y_act)

# plt.plot (x, y_hist, 'r+') # nodes for which I have historical data are down at the moment
# plt.plot (x, y_dyn, 'b+')
# plt.plot (x_act, y_act, 'g+')
# plt.show ()

# plt.plot (x, y_hist, 'r+')
plt.plot (x, y_dyn, 'rx')
plt.plot (x, y_hist, 'kx')
plt.plot (x_act, y_act, 'b+')
plt.xscale ('log')
plt.yscale ('log')
# plt.legend(['Historical data', 'realtime data', 'observed values'])
title=''
if ('alltoall' in sys.argv[1]): title = "MPI Alltoall"
if ('allgather' in sys.argv[1]): title = "MPI Allgather"
if ('bcast' in sys.argv[1]): title = "MPI Broadcast"
title += ': Predictions'
plt.title (title)
plt.legend(['Predictions using Network Health', 'Predictions using Historical Data', 'Observed Values'])
plt.xlabel ("Message Size (B)")
plt.ylabel ("Time Taken in Collective (s)")
plt.show()
print (flag)
