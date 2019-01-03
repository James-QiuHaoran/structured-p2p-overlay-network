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
echo ""
echo "# of broadcasted messages: $1"

rm -rf tmp

total=0
count=0
total_half=0
count_half=0
total_two_third=0
count_two_third=0
total_five_sixth=0
count_five_sixth=0
thre_half=$(($2 /2))
thre_two_third=$(($2 * 2 / 3))
thre_five_sixth=$(($2 * 5 / 6))

for broadcast_id in `cat tmp-broadcast | sort | uniq`
do
	reachability=`grep ,$broadcast_id, ./*csv | cut -d',' -f6 | sort | uniq | wc -l`
	if [ $reachability == $2 ]
	then
		content=""
		for file in `ls *.csv`
		do
        		msg_timestamp=`grep ,$broadcast_id, $file | cut -d',' -f1 | head -n 1` # the first time that a node receive the message
        		content="$content $msg_timestamp"
		done
		for i in $content
		do
        		echo $i >> "tmp"
		done
		start_time=`cat tmp | sort | uniq | head -n 1`
		end_time=`cat tmp | sort | uniq | tail -1`
		end_time_half=`cat tmp | sort | uniq | head -n $thre_half | tail -1`
		end_time_two_third=`cat tmp | sort | uniq | head -n $thre_two_third | tail -1`
		end_time_five_sixth=`cat tmp | sort | uniq | head -n $thre_five_sixth | tail -1`

		total=$(($total + $end_time - $start_time))
		count=$(($count + 1))
		total_half=$(($total_half + $end_time_half - $start_time))
		count_half=$(($count_half + 1))
		total_two_third=$(($total_two_third + $end_time_two_third - $start_time))
                count_two_third=$(($count_two_third + 1))
		total_five_sixth=$(($total_five_sixth + $end_time_five_sixth - $start_time))
                count_five_sixth=$(($count_five_sixth + 1))
		rm -rf tmp
	elif [ $reachability -gt $thre_five_sixth ]
	then
		content=""
		for file in `ls *.csv`
		do
			msg_timestamp=`grep ,$broadcast_id, $file | cut -d',' -f1 | head -n 1`
                        content="$content $msg_timestamp"
		done
		for i in $content
                do
                        echo $i >> "tmp"
                done
                start_time=`cat tmp | sort | uniq | head -n 1`
                end_time_half=`cat tmp | sort | uniq | head -n $thre_half | tail -1`
                end_time_two_third=`cat tmp | sort | uniq | head -n $thre_two_third | tail -1`
                end_time_five_sixth=`cat tmp | sort | uniq | head -n $thre_five_sixth | tail -1`
                total_half=$(($total_half + $end_time_half - $start_time))
                count_half=$(($count_half + 1))
                total_two_third=$(($total_two_third + $end_time_two_third - $start_time))
                count_two_third=$(($count_two_third + 1))
                total_five_sixth=$(($total_five_sixth + $end_time_five_sixth - $start_time))
                count_five_sixth=$(($count_five_sixth + 1))
		rm -rf tmp
	elif [ $reachability -gt $thre_two_third ]
        then
                content=""
                for file in `ls *.csv`
                do
                        msg_timestamp=`grep ,$broadcast_id, $file | cut -d',' -f1 | head -n 1`
                        content="$content $msg_timestamp"
                done
                for i in $content
                do
                        echo $i >> "tmp"
                done
                start_time=`cat tmp | sort | uniq | head -n 1`
                end_time_half=`cat tmp | sort | uniq | head -n $thre_half | tail -1`
                end_time_two_third=`cat tmp | sort | uniq | head -n $thre_two_third | tail -1`
                total_half=$(($total_half + $end_time_half - $start_time))
                count_half=$(($count_half + 1))
                total_two_third=$(($total_two_third + $end_time_two_third - $start_time))
                count_two_third=$(($count_two_third + 1))
                rm -rf tmp
	elif [ $reachability -gt $thre_two_third ]
        then
                content=""
                for file in `ls *.csv`
                do
                        msg_timestamp=`grep ,$broadcast_id, $file | cut -d',' -f1 | head -n 1`
                        content="$content $msg_timestamp"
                done
                for i in $content
                do
                        echo $i >> "tmp"
                done
                start_time=`cat tmp | sort | uniq | head -n 1`
                end_time_half=`cat tmp | sort | uniq | head -n $thre_half | tail -1`
                total_half=$(($total_half + $end_time_half - $start_time))
                count_half=$(($count_half + 1))
                rm -rf tmp
	fi
done 

echo "$count messages are successfully broadcasted to 100% of the whole network."
echo "$count_five_sixth messages are successfully broadcasted to 83.3% of the whole network."
echo "$count_two_third messages are successfully broadcasted to 66.7% of the whole network."
echo "$count_half messages are successfully broadcasted to 33.3% of the whole network."
echo ""

if [ $count -eq 0 ]
then
	avg="N/A"
	echo "Average time used to broadcast a message to the whole network: $avg"
else
	avg=`echo $total/$count | bc -l`
	printf "Average time used to broadcast a message to the whole network: %.3f ms\n" "$avg"
fi

if [ $count_five_sixth -eq 0 ]
then
        avg="N/A"
        echo "Average time used to broadcast a message to 5/6 of the network: $avg"
else
        avg=`echo $total_five_sixth/$count_five_sixth | bc -l`
        printf "Average time used to broadcast a message to 5/6 of the network: %.3f ms\n" "$avg"
fi

if [ $count_two_third -eq 0 ]
then
        avg="N/A"
        echo "Average time used to broadcast a message to 2/3 of the network: $avg"
else
        avg=`echo $total_two_third/$count_two_third | bc -l`
        printf "Average time used to broadcast a message to 2/3 of the network: %.3f ms\n" "$avg"
fi

if [ $count_half -eq 0 ]
then
        avg="N/A"
        echo "Average time used to broadcast a message to 1/2 of the network: $avg"
else
        avg=`echo $total_half/$count_half | bc -l`
        printf "Average time used to broadcast a message to 1/2 of the network: %.3f ms\n" "$avg"
fi

rm -rf tmp-broadcast
