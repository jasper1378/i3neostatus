#!/usr/bin/env bash

# Dependencies:
# libpulse

mkdir temp_0102325082022
g++ -fdiagnostics-color=always -D_FILE_OFFSET_BITS=64 -Wall -Winvalid-pch -O0 -g -Wunused-value -D_GNU_SOURCE -D_REENTRANT -pthread -c -o temp_0102325082022/i3status_pulse.o i3status_pulse.cpp
g++ -O0 -c -o temp_0102325082022/globals.o globals.cpp
g++ -O0 -c -o temp_0102325082022/main.o main.cpp
g++ -O0 -c -o temp_0102325082022/pulseaudio_volume.o pulseaudio_volume.cpp
g++ -O0 -c -o temp_0102325082022/common.o ../../common/common.cpp
g++  -lpulse -lm -o volume temp_0102325082022/*.o
rm -r temp_0102325082022
