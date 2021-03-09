#!/bin/bash

booma/booma-console/booma-console -o 0 -m AM -i NETWORK vlf2 1722 1723 -dr 1152000 -or 48000 -it Q -is RTLSDR -f 1440000 -g 1 $@
