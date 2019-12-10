#! /bin/bash

netstat -apn | grep sshd | sed -n 's/.*:::\([0-9]*\)\ .* \ \([0-9]*\)\/sshd/\1 \2/p'
