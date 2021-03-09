#!/bin/bash

booma/booma-console/booma-console -m CW -i NETWORK vlf2 1722 1723 -dr 1152000 -or 48000 -it Q -is RTLSDR -f 368000 $@
