#!/bin/bash 
#insmod /usr/src/linux-2.6.18/rkmod/rk.ko
#lsmod | grep rk
/sbin/insmod q8Driv.ko
/sbin/lsmod | grep q8Driv

TMP=$(cat /proc/devices | grep Q8driver | cut -d ' ' -f 1)
mknod /dev/q8 c $TMP 0

