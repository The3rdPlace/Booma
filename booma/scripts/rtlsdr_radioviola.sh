#!/bin/bash

make -j 10 && booma/booma-console/booma-console -o 0 -m AM -i RTLSDR 0 -dr 1152000 -or 48000 -it Q -f 1440000 -rtlo 10000 $@
