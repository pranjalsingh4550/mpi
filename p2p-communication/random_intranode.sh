#script to use bash $RANDOM and send random sized INTRANODE messages
if [ $# -lt 4 ] ;
then
	echo "Usage ./random.sh <number of messages> <sizes file> log_[nprocs] host1 host2 .. > OUTPUT_FILE"
	exit
fi

i=0
s=`wc -l $2`
s=($s)
sizes=`cat $2`
sizes=($sizes)
sumfrom=0
sumto=0
while [ $i -lt $1 ]
do
	a=$RANDOM
	number=$((2**$3))
	from=$((a%$number))
	from=$((from+4))
	a=$((a>>$3))
	message=$((a&255))
	size=${sizes[$message]}
	if (( $message < ${s[0]} ))
	then
		# echo "source ${!from} message number $message message size $size"
		# timeout 2 mpiexec -n 2 -host csews${!from} ./a.out $size 2> /dev/null
		timeout 2 ./mpiexec.sh csews${!from} $size
		if [ $? -ne 0 ]
		then
			echo "mpiexec failed hosts ${!from}" > /dev/stderr
		fi
		i=$(($i+1))
		echo -n "-" > /dev/stderr
		if (( i % 40 == 0 ))
		then
			echo "" > /dev/stderr
			echo "Loop $i done" > /dev/stderr
		fi
	fi

done
