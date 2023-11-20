import csv
import sys
import matplotlib.pyplot as plt
# refer to the comments in outliers.py

print (len(sys.argv), sys.argv[0])
if (len(sys.argv) < 3):
    print("usage: $python3 csvplot.py <dummy arg> count <csv1> <csv2> ...")
    exit()
else:
    print ("valid call len(argv) is",len(sys.argv))
# modification of outliers.py - used to plot the observed LATENCY = time - size*100MBps

colours = 'rbg'

row2 = []
col2 = []
for t in range (3, 5):
    print ("t is ",t)
    if (len (sys.argv) < 5):
        continue
    print ("opening csv ",sys.argv[t])
    try:
        f = open ( sys.argv[t], 'r')
    except:
        print ("not opening this file")
        break
    m = csv.reader (f)
    row = []
    column = []

    p = 0
    q = 1

    count = 0
    for line in m:
        count += 1
        if (count == 1000): print (line)
        row.append ( int(line[p]))
        column.append (float (line [q]) - int(line[p]) * 10**(-8))
        if count>int(sys.argv[2]):
            break
    f.close()
    row2.append (list (row))
    col2.append (list (column))

print (len (row2), len (col2))

plt.figure (figsize= (15,10))
plt.plot (row2[0], col2[0], 'r+')
if len(row2) == 2:
    plt.plot (row2[1], col2[1],'b+')
plt.show ()

plt.figure (figsize= (15,10))
plt.plot (row2[0], col2[0],'r+')
if len(row2) == 2:
    plt.plot (row2[1], col2[1],'b+')
plt.grid (axis = 'y')
plt.xscale("log")
plt.yscale("log")
plt.show()


plt.figure (figsize= (15,10))
plt.plot (row2[1], col2[1],'b+')
if len(row2) == 2:
    plt.plot (row2[0], col2[0],'r+')
plt.grid (axis = 'y')
plt.xscale("log")
plt.yscale("log")
plt.show()
