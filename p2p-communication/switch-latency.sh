# used to determine if interswitch messages take longer

cd $HOME/mpicode/run_random
# for j in csews21,csews22
# hosts="2 3 4 5 7 9 10 11 13 15 18 19 20"
hosts="2 5 18 20 26 31"
for j in $hosts
do
	for k in $hosts
	do
		echo $j $k > /dev/stderr
		echo -n csews$j,csews$k,
		for t in `seq 1 100`
		do
			mpiexec -n 2 -host csews$j,csews$k ./a.out 1024 2> /dev/null
		done | sort -gk 2 | tail -n 15 | head -n 1
	done
done > netwrokr-stats-4
pkill -u prsingh
