#!/bin/bash
#
# @(!--#) @(#) Install.sh, sversion 0.1.0, fversion 002, 01-july-2024
#
# Install the serial menu code and config files
#

set -u

PATH=/usr/bin:/bin:/usr/local/bin
export PATH

progname=`basename $0`

expect=`which expect`

if [ $? -ne 0 ]
then
  echo "$progname: unable to find the expect command - maybe install with \"sudo apt install expect\"?" 1>&2
  exit 2
fi

echo "Compiling serialterm.c"

rm -f serialmenu
gcc -D"EXPECT_EXECUTABLE=\"$expect\"" -o serialmenu serialmenu.c

if [ $? -ne 0 ]
then
  echo "$progname: the compile of the serialmenu command appears to have failed" 1>&2
  exit 2
fi

if [ ! -x serialmenu ]
then
  echo "$progname: the gcc command did not produce the serialmenu executable" 1>&2
  exit 2
fi

echo "Copying files into place"

sudo cp serialmenu     /usr/local/bin/serialmenu
sudo chown root:root   /usr/local/bin/serialmenu
sudo chmod u=rwx,go=rx /usr/local/bin/serialmenu

sudo cp serialconnect.exp   /usr/local/bin/serialconnect.exp
sudo chown root:root        /usr/local/bin/serialconnect.exp
sudo chmod u=rwx,g=rwx,o=rx /usr/local/bin/serialconnect.exp

sudo cp serialmenu.conf  /usr/local/etc/serialmenu.conf
sudo chown root:root     /usr/local/etc/serialmenu.conf
sudo chmod u=rw,g=rw,o=r /usr/local/etc/serialmenu.conf

echo "Creating /var/logl/serial"

if [ ! -d /var/local/serial ]
then
  sudo mkdir /var/local/serial
fi
sudo chmod u=rwx,g=rwx,o=rx /var/local/serial
sudo chown root:dialout     /var/local/serial

echo "Finished"

exit 0
