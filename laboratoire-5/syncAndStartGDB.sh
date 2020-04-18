#!/bin/bash
set -e

# Sync executable
bn=$(basename $1)
rsync -az $1/build/SETR_TP5 pi@$2:/home/pi/projects/$bn/SETR_TP5

# Execute GDB
ssh pi@$2 "nohup gdbserver :4567 /home/pi/projects/$bn/SETR_TP5 > /dev/null 2> /dev/null < /dev/null &"
sleep 1 
