#!/bin/bash

make -j 10 && booma/booma-console/booma-console -i NETWORK vlf 1720 1721 -m CW -it REAL -is AUDIO -dr 48000 -or 48000 -f 23400 $@
