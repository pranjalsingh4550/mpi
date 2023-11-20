#script to use bash $RANDOM and send random sized messages
if [ $# -lt 4 ] ;
then
	echo "Usage ./random.sh <number of messages> <sizes file> log_[nprocs] host1 host2 .. > OUTPUT_FILE"
	exit
fi

mexec () {
	echo "mrexec called"
	echo "arg1 is $1"
	mpiexec -n 2 -host $1 ./a.out $2 2> /dev/null
}
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
	if (( $message < ${s[0]} ))
	then
		# echo "source ${!from} dest ${!to} message number $message message size $size"
		# timeout 2 mexec "csews${!from},csews${!to}" $size 2> /dev/null
		timeout 2 ./mpiexec.sh csews${!from},csews${!to} $size
		# echo "return $?"
		if [ $? -ne 0 ]
		then
			echo "mpiexec failed hosts ${!from}, ${!to}" > /dev/stderr
			# i=$(($i+1))
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
		# echo done
	fi
done
