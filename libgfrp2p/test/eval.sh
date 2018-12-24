#!/bin/bash

echo "===== Evaluation ====="

for i in `ls log`
do
	echo $i >> tmp
done

directory_name=`cat tmp | sort | tail -1`
echo "Time stamp: $directory_name"

cp num_msgs.sh log/$directory_name
cp time_spent.sh log/$directory_name

echo ""

cd log/$directory_name

size=`ls -l | wc -l`
size=$(($size-3))
echo "Network Size (# of nodes): $size"
echo ""
echo "Broadcast reachability: $size/$size"
echo ""

./num_msgs.sh

echo ""

./time_spent.sh

echo ""

echo "===== Evaluation Done ====="

cd ../..

rm -rf tmp