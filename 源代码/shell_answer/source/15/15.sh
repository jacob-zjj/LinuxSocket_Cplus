#!/bin/bash

touch m1.txt m2.txt m3.txt m4.txt
I=1 

while [ $I -le 4 ]; do 
    mkdir m$I
    mv m$I.txt m$I
    I=$((I+1))
done
