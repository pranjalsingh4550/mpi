if [ $# -ne 2 ]
then
	echo "Usage $./bcash.sh file8 file16"
	exit 1
fi

if [ -f $1 ]
then
	echo $1 exists
	exit
fi
file1=$1
file2=$2
# iter=300
iter=20
for m in `seq 1 $iter`
do
	host=""
	size=$((4096<<$((RANDOM%13))))
	for m in `shuf hostlist`
	do
		host=$m:1,$host
	done
	timeout 3 mpiexec -n 8 -host $host ./coll $size >> $file1
done

for m in `seq 1 $iter`
do
	size=$((4096<<$((RANDOM%13))))
	host=""
	for m in `shuf hostlist_*`
	do
		host=$m:2,$host
	done
	timeout 3 mpiexec -n 16 -host $host ./coll $size >> $file2
done
# pkill -u prsingh
