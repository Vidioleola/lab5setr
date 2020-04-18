#!/bin/bash

#must be use when installing ned libs on Rpi

cd ~/arm-cross-comp-env/arm-raspbian-linux-gnueabihf/arm-raspbian-linux-gnueabihf
rsync -av --numeric-ids --exclude "*.ko" --exclude "*.fw" --exclude "/opt/vc/src" --delete pi@$1:{/lib,/opt} sysroot
rsync -av --numeric-ids --exclude "/usr/lib/.debug" --delete pi@$1:{/usr/lib,/usr/include} sysroot/usr

cd ~/arm-cross-comp-env/arm-raspbian-linux-gnueabihf/arm-raspbian-linux-gnueabihf/sysroot
find . -lname '/*' | while read l ; do   echo ln -sf $(echo $(echo $l | sed 's|/[^/]*|/..|g')$(readlink $l) | sed 's/.....//') $l; done | sh
