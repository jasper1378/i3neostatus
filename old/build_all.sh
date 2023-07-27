#!/usr/bin/env bash

function build() {
    cd src/blocklets/$1
    ./build.sh
    mv $1 ../../../bin
    cd ../../../
}

function copy_script() {
    cd src/blocklets/$1
    cp $1 ../../../bin
    cd ../../../
}

mkdir -p bin

build backlight
build battery
build ethernet
build volume
build wireless

copy_script datetime
