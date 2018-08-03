#!/bin/bash
make clean && make 
filename="$1"
while read -r line
do
    name="$line"
    ./src/test_events $line
done < "$filename"
