#!/bin/bash

a=`echo $0 | sed 's/..\(...\).*/\1/'`

for i in `w|awk -v b=$a 'NR>2{if($NF !~ b) print $2}'`
do 
    echo $i
	fuser -k /dev/$i 
done
