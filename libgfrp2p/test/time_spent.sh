#!/bin/bash

content=""

for file in `ls *.csv`
do
	msg_timestamps=`cat $file | tail -n +2 | cut -d',' -f1`
	for i in $msg_timestamps
	do
		content="$content $i"
	done
done

for i in $content
do
	echo $i >> "tmp"
done

start_time=`cat tmp | sort | uniq | head -n 1`
end_time=`cat tmp | sort | uniq | tail -1`
duration=$(($end_time-$start_time))

echo "Broadcast start-time: $start_time"
echo "Broadcast end-time: $end_time"
echo "Total time used to converge: $duration ms"

rm -rf tmp
