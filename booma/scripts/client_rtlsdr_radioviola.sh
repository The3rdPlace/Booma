#!/bin/bash

booma/booma-console/booma-console -o 0 -m AM -i NETWORK localhost 1720 1721 -dr 960000 -or 48000 -it Q -is RTLSDR -f 1440000 -rtlc 0 -rtlo 5000 $@
