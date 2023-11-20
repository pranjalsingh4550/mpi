cd internode
for k in `ls data*`
do
	echo sorting $k
	sort -t , -k 1 -g $k > temp
	mv temp $k
done
cd ../intranode
for k in `ls data*`
do
	sort -t , -k 1 -g $k > temp
	mv temp $k
done
cd ..
