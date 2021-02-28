#!/bin/bash

booma/booma-console/booma-console -o 0 -m AM -i NETWORK vlf 1722 1723 -dr 960000 -or 48000 -it Q -is RTLSDR -f 1440000 -rtlc 20 -rtlo 5000 $@
