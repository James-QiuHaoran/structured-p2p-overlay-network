#!/bin/bash

echo "===== Evaluation ====="

cp num_msgs_eth.sh log/
cp time_spent_eth.sh log/

echo ""

cd log

size=`ls -l eth* | wc -l`
echo "Network Size (# of nodes): $size"
echo ""

./num_msgs_eth.sh

echo ""

./time_spent_eth.sh 0 $size

echo ""

echo "===== Evaluation Done ====="
