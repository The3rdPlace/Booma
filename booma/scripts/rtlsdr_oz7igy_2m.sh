#!/bin/bash

make -j 10 && booma/booma-console/booma-console -o 0 -m SSB -i RTLSDR 0 -dr 1152000 -or 48000 -it IQ -f 144470200 -ro Mode=USB -rtlo 5000 -rtlc 10 $@
