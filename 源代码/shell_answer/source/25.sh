#! /bin/sh

sudo apt-get update
sudo apt-get intall aptitude -y
sudo apt-get intall tree
sudo apt-get intall vim -y 
sudo echo "sudo cp vimrc /etc/vim/vimrc"
sudo cp vimrc /etc/vim/vimrc

sudo apt-get intall stardict -y
sudo echo "sudo cp dic /usr/share/stardict -r"
sudo cp dic /usr/share/stardict -r

sudo apt-get intall g++ -y
sudo apt-get intall vaftpd -y
sudo apt-get intall lftp -y
sudo apt-get intall nfs-kernel-server -y









