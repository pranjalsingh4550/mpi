# generates and shuffles lists of source/destination/message size
# hosts should be listed with "csews" and groups of hosts should be inside quotes
if [ $# -ne 4 ]
then
	echo Usage $ ./text.sh "hosts_from" "hosts_to"  output_file \#messages
	exit
fi
rm -f temp2 temp &> /dev/null

host=$1
echo "host source $1, dest $2 $4 messages each. Continue? (y/n)"
read response
if [ $response != "y" ]
then
	exit 1
fi

for t in `cat sizestime`
do
	for j in $host
	do
		for k in $2
		do
			for c in `seq 1 $4`
			do
				echo $j,$k,$t >> temp2
			done
		done
	done
done
shuf temp2 > $3
rm temp2
