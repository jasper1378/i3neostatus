#!/usr/bin/env bash

g++ main.cpp backlight.cpp ../../common/common.cpp ../../common/missing_dependency_error/missing_dependency_error.cpp -o backlight -O3
