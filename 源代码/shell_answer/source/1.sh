#/bin/bash

first=0
second=0

read -p "Input the first number:" first
read -p "Input the second number:" second
result=$[$first+$second]

echo "result is : $result"

exit 0
