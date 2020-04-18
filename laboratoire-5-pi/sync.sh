#!/bin/bash
set -e

# Sync executable
bn=$(basename $1)
rsync -az $1/build/SETR_TP5 pi@$2:/home/pi/projects/$bn/SETR_TP5