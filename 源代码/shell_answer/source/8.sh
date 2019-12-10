#! /bin/bash

free -m | sed -n '/Swap/p' | awk '{ print $2}'
