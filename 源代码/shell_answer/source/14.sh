#!/bin/bash

# itcst by xingwenpeng

user=`whoami`
case $user in
    root)
        echo "hello root";;
    xingwenpeng)
        echo "hello xingwenpeng";;
    itcast)
        echo "hello itcast";;
    *)
        echo "hello $user,welcome"
esac

echo "日期和时间: `date`"
echo "本月的日历: `cal`"
echo "本机的机器名:`uname -n`"
echo "当前这个操作系统的名称和版本:`uname -s;uname -r`"
echo "父目录中的所有文件的列表:`ls ../`"
echo "root正在运行的所有进程:` ps -u root`"
echo "变数TERM的值:$TERM"
echo "变数PATH的值:$PATH"
echo "变数HOME的值:$HOME"
echo "磁盘的使用情况:`df`"
echo "用id命令打印出你的组ID:`id -g`"
echo "Good bye!"
exit 0
