#!/bin/bash

# NDB Roskilde (RK) at 368.000
booma/booma-console/booma-console -m CW -i NETWORK vlf2 1722 1723 -dr 1152000 -or 48000 -it Q -is RTLSDR -f 368400 -g 30 $@
