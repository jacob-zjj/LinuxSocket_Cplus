#! /bin/bash
# $1 为要测试的日志文件

awk '{print $1}' $1 | sort | uniq -c | sort -k1nr | head -n3
