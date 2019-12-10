#! /bin/bash

time (
    for i in {1..2000} ; do 
        mkdir /tmp/nnn$i
    done
)
