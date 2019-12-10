#/bin/bash

# Using for move currently directory lt 10k file to /tmp
 
for FileName in `ls -l | awk '$5>10240 {print $9}'`; do
    mv $FileName /tmp
done
 
ls -al /tmp
 
echo "Done! "
