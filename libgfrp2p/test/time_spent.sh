#!/bin/bash

if [ "$#" -ne 1 ]
then
    echo "Illegal number of parameters; Correct usage: ./time_spent num_broadcast_msgs"
    exit 1
fi

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
avg=`echo $duration/$1 | bc -l`

rm -rf tmp

echo "Broadcast start-time: $start_time"
echo "Broadcast end-time: $end_time"
echo "# of broadcasted messages: $1"
echo "Total time used to converge: $duration ms"
printf "Average time used to broadcast a message: %.3f ms\n" "$avg"