# script to use bash $RANDOM and check which nodes are live. Specify start and end without "csews"
# a.out is generated from one_msg.c
if [ $# -lt 2 ] ;
then
	echo "Usage ./test_nodes.sh <start> <end>" 
	exit
fi

list=""
from=$1
number=$(($2-$1))
number=$(($number+1))
echo "number $number"
i=$number
while [ $i -gt "0" ]
do
	# echo $from xx
	# echo $a
	a=$RANDOM
	to=$((a%$number))
	to=$((to+$1))
	# echo $a $from $to
	timeout 2 mpiexec -n 2 -host csews$from,csews$to ./a.out 100 > /dev/null 2> /dev/null
	if [ $? -eq 0 ]
	then
		# echo "				mpiexec failed hosts $from, $to"
	# else
		echo "success +++++ ${from:0}, ${to:0}"
		list="$list ${from:0}"
		from=$(($from+1))
	else
		echo "failure ----- ${from:0}, ${to:0}"
		if [ $(($RANDOM%3)) -eq 0 ]
		then
			from=$(($from+1))
			# echo from is $from
		else
			i=$(($i+1))
		fi
	fi
	i=$(($i-1))
done
echo $list
