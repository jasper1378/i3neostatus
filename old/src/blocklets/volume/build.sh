#!/usr/bin/env bash

# Dependencies:
# libpulse

g++ ../../common/common.cpp main.cpp volume.cpp alsa_volume.cpp pulseaudio_volume.cpp -lasound -lpulse -o volume -O3 -std=c++20
