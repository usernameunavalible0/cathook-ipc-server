#!/bin/bash

numcpu=$(grep -c ^processor /proc/cpuinfo)

make clean
make -j$numcpu
sudo make install