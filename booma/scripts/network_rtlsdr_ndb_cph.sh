#!/bin/bash

booma/booma-console/booma-console -o 0 -m CW -i NETWORK vlf2 1722 1723 -dr 1152000 -or 48000 -it Q -is RTLSDR -f 368000 -rtlc 20 -rtlo 5000 -o 1 $@
