#script to use bash $RANDOM and send random sized messages
# list hosts without "csews"
# sends internode messages ONLY, usually sends the exact number of messages speficied despite errors
# message size is randomly chosen. use historical-data/* if you want N messages of each size
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
res=0
while [ $i -lt $1 ]
do
	a=$RANDOM
	number=$((2**$3))
	from=$((a%$number))
	from=$((from+4))
	a=$((a>>$3))
	to=$((a%$number))
	to=$((to+4))
	a=$((a>>$3))
	message=$((a&255))
	size=${sizes[$message]}
	if (( $message < ${s[0]} && $from != $to ))
	then
		# echo "source ${!from} dest ${!to} message number $message message size $size"
		timeout 2 ./mpiexec.sh csews${!from},csews${!to} $size
		if [ $? -ne 0 ]
		then
			echo "mpiexec failed hosts ${!from}, ${!to}" > /dev/stderr
		else
			echo -n "-" > /dev/stderr
			i=$(($i+1))
			sumfrom=$((sumfrom+from-4))
			if (( i % 40 == 0))
			then
				echo "" > /dev/stderr
				echo Done loop $i > /dev/stderr
			fi
		fi
	fi
done
