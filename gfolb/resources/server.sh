#!/bin/sh
export GFOLB_PATH=`echo $(dirname $(readlink -f $0))`
echo $GFOLB_PATH
export LD_LIBRARY_PATH=$GFOLB_PATH/lib:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH
export PATH=$GFOLB_PATH/lib:$PATH
echo $PATH
#/usr/sbin/setenforce 0
./gfolb > gfolb.log 2>&1