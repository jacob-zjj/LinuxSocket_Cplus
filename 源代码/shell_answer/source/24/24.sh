#! /bin/bash

# show top 10

file=$1
awk '{print $1}' testdata.txt | sort | uniq -c | sort -k1nr | head -n10
