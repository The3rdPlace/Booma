#!/bin/bash

make -j 10 && booma/booma-console/booma-console -o 2 -m CW -i NETWORK vlf2 1722 1723 -dr 960000 -or 48000 -it IQ -is RTLSDR -f 144470200 -rtlo 5000 -rtlc 0 -dg 0 $@
