if [ $# -ne 1 ]
then
	echo Usage $ ./run_text.sh file
	exit 1
fi

count=0
# for k in `sed '' temp`
for k in `cat $1`
do
	a=`echo $k | sed 's/,/\ /g'`
	a=($a)
	timeout 2 mpiexec -n 2 -host ${a[0]},${a[1]} ./a.out ${a[2]} 2> /dev/null
	if [ $? -ne 0 ]
	then
		echo failure -host ${a[0]},${a[1]}
		exit 12
	fi
	echo -n "-" > /dev/stderr
	count=$(($count+1))
	if [ $(($count%100)) -eq 0 ]
	then
		echo Done loop $count > /dev/stderr
	fi
done
exit
