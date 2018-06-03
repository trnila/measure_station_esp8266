#!/bin/sh
make -j4 -C examples/measure_station/ flash && minicom -D /dev/ttyUSB0
