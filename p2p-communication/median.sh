# computes medians for each message size in each file in ./internode/ and ./intranode/
cd internode
for k in `ls`
do
	python3 ../median.py $k "$k"median
done
cd ../intranode
for k in `ls`
do
	python3 ../median.py $k "$k"median
done
cd ..
