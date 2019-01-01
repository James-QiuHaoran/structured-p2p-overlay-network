#!/bin/bash

if [ "$#" -ne 2 ]
then
    echo "Illegal number of parameters; Correct usage: ./time_spent num_broadcast_msgs num_nodes"
    exit 1
fi

content=""

for file in `ls *.csv`
do
	msg_timestamp=`cat $file | tail -n +2 | head -n 1 | cut -d',' -f1`
	content="$content $msg_timestamp"
	msg_timestamp=`cat $file | tail -n +2 | tail -n 1 | cut -d',' -f1`
	content="$content $msg_timestamp"
done

for i in $content
do
	echo $i >> "tmp"
done

content=""
for file in `ls *.csv`
do
	broadcast_ids=`cat $file | tail -n +2 | cut -d',' -f4 | sort | uniq`
	content="$content $broadcast_ids"
done

for i in $content
do
	echo $i >> "tmp-broadcast"
done

start_time=`cat tmp | sort | uniq | head -n 1`
end_time=`cat tmp | sort | uniq | tail -1`
duration=$(($end_time - $start_time))

echo "Broadcast start-time: $start_time"
echo "Broadcast end-time: $end_time"

echo "# of broadcasted messages: $1"
# echo "Total time used: $duration ms"

rm -rf tmp

total=0
count=0
for broadcast_id in `cat tmp-broadcast | sort | uniq`
do
	reachability=`grep ,$broadcast_id, ./*csv | cut -d',' -f6 | sort | uniq | wc -l`
	if [ $reachability == $2 ]
	then
		# echo "success"
		content=""
		for file in `ls *.csv`
		do
        		msg_timestamp=`grep ,$broadcast_id, $file | cut -d',' -f1` | head -n 1
        		content="$content $msg_timestamp"
        		msg_timestamp=`grep ,$broadcast_id, $file | tail -n 1 | cut -d',' -f1`
        		content="$content $msg_timestamp"
		done
		for i in $content
		do
        		echo $i >> "tmp"
		done
		start_time=`cat tmp | sort | uniq | head -n 1`
		end_time=`cat tmp | sort | uniq | tail -1`
		total=$(($total + $end_time - $start_time))
		count=$(($count + 1))
		rm -rf tmp
	fi
done 

echo "$count messages are successfully broadcasted to the whole network."

if [ $count -eq 0 ]
then
	avg="N/A"
	echo "Average time used to broadcast a message: $avg"
else
	avg=`echo $total/$count | bc -l`
	# echo "Total: $count * $avg = $total"
	printf "Average time used to broadcast a message: %.3f ms\n" "$avg"
fi

rm -rf tmp-broadcast
