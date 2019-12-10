#! /bin/sh

cat /etc/passwd | awk -F: '{if ($7!="") print $7}' | sort | uniq -c

# cat /etc/passwd|awk -F: '{if ($7!="") print $7}'| sort | uniq -c | awk '{print $2,$1}'
