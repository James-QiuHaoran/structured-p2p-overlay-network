#!/bin/bash

total=0

for file in `ls *.csv`
do
	count=`cat $file | tail -n +2 | cut -d',' -f4 | wc -l`
	total=$(($total+$count))
done

total=$(($total/2))
echo "# of messages sent in total: $total"
echo "# of messages received in total: $total"