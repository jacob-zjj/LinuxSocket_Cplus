#!/bin/bash
# 需要root权限

echo "create /usrdata"

mkdir /usrdata

if [ $? -eq 0 ]; then
	i=1
	while [ $i -le 50 ]; do
		mkdir -p /usrdata/user$i
		chmod 754 /usrdata/user$i
		let i++
	done
else
	echo "bye"
	exit 1
fi

exit 0


