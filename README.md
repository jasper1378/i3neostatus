# i3blocks-blocklets

## About
My personal blocklets for [i3blocks](https://github.com/vivien/i3blocks).

## Blocklets
```
backlight
battery
ethernet
timedate
volume
wireless
```

## Installation
i3blocks-blocklets can be installed from the source directly or as an AUR package.

### Dependencies
`backlight` depends on [light](https://github.com/haikarainen/light)
`volume` depends on [PulseAudio](https://www.freedesktop.org/wiki/Software/PulseAudio/)

### Source
Acquire the sources
```
$ git clone https://github.com/jasper1378/i3blocks-blocklets.git
$ cd i3blocks-blocklets
```
Build all the blocklets
```
$ ./build_all
```
The resulting compiled blocklets can be found in the `bin/` directory

### AUR
COMING SOON!

## Usage
The documentation for a particular blocklet (including and examples i3blocks.conf) can be found in the `doc/` directory.
