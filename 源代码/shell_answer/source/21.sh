#!/bin/bash
#编写shell程序，实现自动删除30个账号的功能。账号名为std01至stud30
#学要有root权限

for i in {9901..9930}; do
    xx=`echo $i | sed 's/99//g'`
        userdel -r std$xx
done

