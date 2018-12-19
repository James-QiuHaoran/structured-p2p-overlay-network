#!/bin/bash

if [ "$#" -ne 2 ] 
then
    echo "Illegal number of parameters; Correct usage: ./reset_evaluation.sh num_nodes start_port"
    exit 1
fi

for (( i=0; i<$1; i++ ))
do
    eval "port=\$(( $i + $2 ))"
    pid="`lsof -i tcp:$port | tail -1 | awk -F ' ' '{print $2}'`"
    if [ ! -z $pid ]
    then
        echo "Closing port $port, which is listened by $pid"
        kill -9 $pid
    fi
done    
