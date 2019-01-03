#!/bin/bash

if [ "$#" -ne 2 ] 
then
    echo "Illegal number of parameters"
    echo "Usage: ./local_eval_client.sh num_clients bootstrap_port"
    exit 1
fi

mkdir -p log

echo $'== HGFRR Evaluation ==\n'

bootstrap_port_base="20200"
broadcast_port_base="30300"

for (( i=0; i<$1; i++ ))
do
    bootstrap_port=$(($bootstrap_port_base+$i))
    broadcast_port=$(($broadcast_port_base+$i))
    ../bin/eval_client $bootstrap_port $broadcast_port 127.0.0.1 $2 &
    sleep 0.05
done