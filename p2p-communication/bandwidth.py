import csv
import sys
import matplotlib.pyplot as plt
# refer to the comments in outliers2.py

print (len(sys.argv), sys.argv[0])
if (len(sys.argv) < 3):
    print("usage: $python3 bandwidth.py <#csvs> count <csv1> <csv2> ...")
    exit()
else:
    print ("valid call len(argv) is",len(sys.argv))
#plots all csvs on a single plot

colours = 'rbg'

row2 = []
col2 = []
for t in range (3, 3 + int (sys.argv[1])):
    print ("t is ",t)
    print ("opening csv ",sys.argv[t])
    f = open ( sys.argv[t], 'r')
    m = csv.reader (f)
    row = []
    column = []

    count = 0
    for line in m:
        count += 1
        if (count == 1000): print (line)
        row.append ( int(line[-2]))
        column.append (int (line[-2]) / float (line [-1]))
        if count>int(sys.argv[2]):
            break
    f.close()
    row2.append (list (row))
    col2.append (list (column))

print (len (row2), len (col2))

plt.figure (figsize= (15,10))
plt.plot (row2[0], col2[0],'r+')
if (len (row2) ==2):
    plt.plot (row2[1], col2[1],'b+')
plt.grid (axis = 'y')
plt.xscale("log")
plt.yscale("log")
plt.show()


plt.figure (figsize= (15,10))
plt.plot (row2[1], col2[1],'b+')
if (len (row2) ==2):
    plt.plot (row2[0], col2[0],'r+')
plt.grid (axis = 'y')
plt.xscale("log")
plt.yscale("log")
plt.show()
