# echo "host is $1 size is $2"
# used in random.sh and some other scripts
# need not be run directly
output=`mpiexec -n 2 -host $1 ./a.out $2`
if [ $? -eq 0 ]
then
	echo $output 
	exit 0
else
	exit 2
fi
