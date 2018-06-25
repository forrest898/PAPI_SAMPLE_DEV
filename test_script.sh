#!/bin/bash
counter=0
filename="$1"
while read -r line
do
    counter=$((counter + 1))
    name="$line"
    ./src/test_events $counter
done < "$filename"
