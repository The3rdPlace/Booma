#!/bin/bash

booma/booma-remote/booma-remote -s 1720 1721 -i RTLSDR 0 -dr 960000 -or 48000 -it Q -f 1440000 -rtlc 0 -rtlo 5000 -dg 100 $@
