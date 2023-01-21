# jasper-i3blocks-blocklets

## Notice
In the coming weeks, I plan to merge the contents of this repository into a new project that I have started work on: a complete i3status alternative.

## About
Jasper's blocklets for [i3blocks](https://github.com/vivien/i3blocks).

## Blocklets
Blocklet | Description
---|---
backlight | Print backlight brightness
battery | Print battery stats
datetime | Print date and time
ethernet | Print ethernet information
volume | Print volume using PulseAudio
wireless | Print wireless information

## Installation
Install dependencies
- [PulseAudio](https://www.freedesktop.org/wiki/Software/PulseAudio/) (for volume blocklet)

Acquire the sources
```
$ git clone https://github.com/jasper1378/jasper-i3blocks-blocklets.git
$ cd jasper-i3blocks-blocklets
```
Build all the blocklets
```
$ ./build_all.sh
```
The resulting compiled blocklets can be found in the `bin/` directory

## Usage
The documentation for a particular blocklet (including an example i3blocks.conf) can be found in the `doc/` directory.

## License
See [LICENSE.md](LICENSE.md) for details.
