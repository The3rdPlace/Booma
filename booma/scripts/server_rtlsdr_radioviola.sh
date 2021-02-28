#!/bin/bash

booma/booma-remote/booma-remote -s 1722 1723 -i RTLSDR 0 -dr 960000 -or 48000 -it Q -f 1440000 -rtlc 20 -rtlo 5000 -g 100 $@
