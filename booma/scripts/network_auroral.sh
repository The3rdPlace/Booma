#!/bin/bash

make -j 10 && booma/booma-console/booma-console -i NETWORK vlf 1720 -m AURORAL -it REAL -dr 48000 -or 48000 $@
