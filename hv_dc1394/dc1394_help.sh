#!/bin/sh

# if it doesn't work, try with "coriander"
# also, try "modprobe ieee1394 raw1394"
# modprobe ohci1394 attempt_root=1
# modprobe video1394
#
# if it worked but does not after some camera replugging, call
# modprobe -r raw1394 video1394
# modprobe raw1394 video1394

/sbin/modprobe -r raw1394 video1394
/sbin/modprobe raw1394 video1394
/sbin/modprobe ohci1394 attempt_root=1

mknod -m 600 /dev/raw1394 c 171 0
chown root.root /dev/raw1394
chmod a+rwx /dev/raw1394

mkdir /dev/video1394
chown root.root /dev/video1394
chmod a+rwx /dev/video1394

mknod -m 666 /dev/video1394/0 c 171 16
