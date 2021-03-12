#!/bin/bash

booma/booma-console/booma-console -m CW -i RTLSDR 0 -dr 1152000 -or 48000 -it Q -f 10000000 $@
