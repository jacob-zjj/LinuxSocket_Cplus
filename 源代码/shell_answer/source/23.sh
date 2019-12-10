#!/bin/bash
#需要有root权限

filename=`date +%y%m%d`_etc.tar.gz
#cd /etc/
tar -zcvf $filename *
mv $filename /root/bak/


# vim /etc/crontab 加入
# * * 1 * * root ./23.sh &
# 加入定时管理模块
