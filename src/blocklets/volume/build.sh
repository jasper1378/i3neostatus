#!/usr/bin/env bash

# Dependencies:
# libpulse

g++ ../../common/common.cpp main.cpp volume.cpp pulseaudio_volume.cpp -lpulse -o volume -O3 -std=c++20
