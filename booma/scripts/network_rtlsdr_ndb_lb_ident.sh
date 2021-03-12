#!/bin/bash

# NDB Angelholm (LB) at 370.500
booma/booma-console/booma-console -m CW -i NETWORK vlf2 1722 1723 -dr 1152000 -or 48000 -it Q -is RTLSDR -f 370900 -g 0 -shift 10000000 $@
