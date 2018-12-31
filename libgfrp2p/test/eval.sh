#!/bin/bash

if [ "$#" -ne 1 ]
then
    echo "Illegal number of parameters; Correct usage: ./eval num_broadcast_msgs"
    exit 1
fi

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

total_num_received_msgs=0
for file in `ls *.csv`
do
	num_received_msgs=`cat $file | tail -n +2 | cut -d',' -f4 | sort | uniq | wc -l`
	total_num_received_msgs=$(($total_num_received_msgs+$num_received_msgs))
done

size=`ls -l | wc -l`
size=$(($size-3))
echo "Network Size (# of nodes): $size"
echo ""
total_num_broadcast_msgs=$(($size*$1))
avg_reachability=`echo 100*$total_num_received_msgs/$total_num_broadcast_msgs | bc -l`
printf "Average broadcast reachability: %.2f %% \n" "$avg_reachability"
echo ""

./num_msgs.sh

echo ""

./time_spent.sh $1 $size

echo ""

echo "===== Evaluation Done ====="

cd ../..

rm -rf tmp
