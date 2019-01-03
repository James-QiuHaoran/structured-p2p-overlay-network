#!/bin/bash

total=0

for file in `ls eth*.txt`
do
	count=`cat $file | wc -l`
	total=$(($total+$count))
done

total=$(($total/2))
echo "# of messages sent in total: $total"
echo "# of messages received in total: $total"
