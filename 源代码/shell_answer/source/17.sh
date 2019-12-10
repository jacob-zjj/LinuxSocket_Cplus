#/bin/bash

echo -e "The program will Judge a file is or not a device file.\n\n"
read -p "Input a filename:" filename
if [ -b $filename -o -c $filename ]; then
    echo "$filename is a device file"
    exit 0
else
    echo "$filename is not a device file"
    exit 1
fi
