host=
for i in `shuf $1`
do
	host=$i:1,$host
done
echo hosting noise at $host
mpiexec -n 8 -host $host ./a.out 3000 250000
exit
