# prints predictions from historical data, realtime data and actual execution time for Bcast
# ipmpi.cpp was changed after this file was created. Uncommenting the relevant parts AND PRINTING THE FINAL NUMBERS TO /dev/stderr will make it work
if [ $# -ne 4 ]
then
	echo Usage $./compare.sh hosts nprocs message_size count
	exit
fi

dir=/users/btech/prsingh/mpich-install/bin/
exe=/users/btech/prsingh/mpicode/ipmpi_input/a.out
source /users/btech/prsingh/.ipmpi
$dir/mpiexec -n $2 -host $1 $exe $3 > /dev/null
export LD_PRELOAD=""

for e in `seq 1 $4`
do
	$dir/mpiexec -n $2 -host $1 $exe $3
done
