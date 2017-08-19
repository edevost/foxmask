#!/bin/sh

result1=$(awk -F',' '{sum+=$2} END{print sum;}' ../FoxMaskResults/tables/example-1.csv)
# 29
result2=$(awk -F',' '{sum+=$2} END{print sum;}' ../FoxMaskResults/tables/example-2.csv)
#18
if [ "$result1" != "29" ]  || [ "$result2" != "18" ]; then
    return 1
else
    return 0
fi
