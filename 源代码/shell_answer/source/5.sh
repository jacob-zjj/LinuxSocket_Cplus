#! /bin/bash

echo "root's bins: $(find ./ -type f | xargs ls -l | sed '/-..x/p' | wc -l)"
