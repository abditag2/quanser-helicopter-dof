#!/bin/bash 
#rmmod /usr/src/linux-2.6.18/rkmod/rk.ko
#lsmod | grep rk
/sbin/rmmod q8Driv.ko
/sbin/lsmod | grep q8Driv
rm /dev/q8
