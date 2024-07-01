# The serialmenu command

## Pre-requesites (short version)

You need:

+ The expecet package installed
+ The gcc package installed
+ Root priviledge on the system

## Pre-requesites (long version)

The expect package must be installed on the system and the expect command should be in one of the directories listed in the
PATH ebvironment variable.

If running:

```
which expect
```

produces output such as:

```
/usr/bin/expect
```

or:

```
/usr/local/bin/expect
```

or similar then the expect package is probably installed.

The gcc package must be installed on the system. If running:

```
gcc --version
```

generates output similar to:

```
gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0
Copyright (C) 2019 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

then gcc is installed.

You must have root priviledge. If you can run the sudo command and, if necessary, supply a password that the
sudo command accepts then you will have suitable root priviledge. on the system.

## Compiling and installing

As a normal user run:

```
./Install.sh
```

You might be prompted by the sudo command for a password to get to root priviledge. Enter the password when prompted.

Typical output:

```
Compiling serialterm.c
Copying files into place
[sudo] password for andyc:
Creating /var/logl/serial
Finished
```

## Configuring

All the configuration for the serialmenu command is set up in the file called:

```
/usr/local/etc/serialmenu.conf
```

Edit this file with any text editor - you will need root priviledge to end the file.

The default file content will look like:

```
#
# /usr/local/etc/serialmenu.conf
#

1:/dev/ttyUSB0:115200:Test of serial port:minicom -D %d -b %b

# end of file
```

Lines beginning with a '#' character are treated as comments and ignored and blank lines are also ignored.

Lines such as:

```
1:/dev/ttyUSB0:115200:Test of serial port:minicom -D %d -b %b
```

consist of five fields separate by ':' characters. The fields are, in order:

```
Menu option number
Device name of the serial port
Baud rate/speed (common values are 9600 and 115200)
Description for the menu option
Command to invoke to make a connection
```

In the above example the command to invoke is:

```
minicom -D %d -b %b
```

The command line arguments are not the actual values used. Whenever a %d is encountered it is changed to the
device name of the serial port and whenever a %b is encountered it is changed to the baud rate/speed to
use when connecting.

So the above example would actually run the following command line:

```
minicom -D /dev/ttyUSB0 -b 115200
```

Edit the /usr/local/etc/serialmenu.conf file as required. For example if there is a second port you might add a line:

```
2:/dev/ttyUSB1:9600:Cisco switch:minicom -D %d -b %b
```

## Running serialmenu

Run the command with no arguments:

```
serialterm
```

A menu will be displayed - for example:

```
Serial Menu
-----------

   1) Test of serial port   (USB0 - 115200)
   0) Exit
```

To exit type "0" and press return. To connect to the first (and only) serial port enter "1" and press return.

## Can the serialmenu command be used as a login shell?

Yes it can. Make sure that:

```
/usr/local/bin/serialmenu
```

is added to any configuration file necessary to make it an aloowable login shell.

Create a user with /usr/local/bin/serialmenu as the login shell. Also put the user into the group called:

```
dialout
```

---------------
End of Document
