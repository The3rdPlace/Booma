#!/bin/bash

make -j 10 && booma/booma-console/booma-console -o 0 -m CW -i RTLSDR 0 -dr 1152000 -or 48000 -it IQ -f 144471000 -rtlo 5000 -rtlc 0 -dg 0 $@
