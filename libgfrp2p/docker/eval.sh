#!/bin/bash

echo "===== Evaluation ====="

cp num_msgs.sh log/
cp time_spent.sh log/

echo ""

cd log

size=`ls -l hgfr* | wc -l`
echo "Network Size (# of nodes): $size"
echo ""

./num_msgs.sh

echo ""

./time_spent.sh 0 $size

echo ""

echo "===== Evaluation Done ====="
