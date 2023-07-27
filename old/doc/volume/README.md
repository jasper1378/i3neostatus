# volume
Print volume using PulseAudio

## Dependencies
[ALSA](https://www.alsa-project.org/)
[PulseAudio](https://www.freedesktop.org/wiki/Software/PulseAudio/)

## Configuration options

### Required
These options must be set to the specified values for the blocklets to work.

`$i3blocks_blocklets_dir` should be set the directory containing the compiled blocklets.
```
command=$i3blocks_blocklets_dir/volume
```
```
interval=persist
```
```
markup=pango
```

### Optional
These options allow you to customize the blocklet.

The volume API to use. Available values are `pulseaudio` and `alsa`. The default value is `pulseaudio`.
```
volume_api=pulseaudio
```
The audio device to monitor. If using `pulseaudio` this can be a sink name or sink index; if using `alsa` this is a mixer name with an optional comma-separated mixer index. The special value `_default_` refers to default PulseAudio or ALSA device, depending on what `volume_api` is set to. Not setting this option is equivalent to `_default_`.
```
device_id=_default_ # for either
device_id=alsa_output.pci-0000_04_00.6.HiFi__hw_Generic_1__sink # PulseAudio sink name
device_id=0 # PulseAudio sink index
device_id=Master # ALSA mixer name
device_id=Master,0 # Alsa mixer name with mixer index
```
The text to be printed when the volume is unmuted.
```
output_format=V: (%devicename): %volume
```
The text to be printed when the volume is muted.
```
output_format_muted=V: (%devicename): muted
```
The color of the text when the volume is unmuted.
```
color_regular=#FFFFFF
```
The color of the text when the volume is muted.
```
color_muted=#FFFF00
```

### Variables that can be uesd in `output_format` and `output_format_muted`

If using `pulseaudio`, the description of the active sink port (ex. "Speaker", "Headphones"); if using `alsa`, the name of the current mixer (ex. "Master", "Speaker").
```
%devicename
```
The volume of the current device.
```
%volume
```
The mute state of the current device ("True" or "False"). It may be preferred to ignore this option and instead used `output_format` and `output_format_muted` to display the mute state.
```
%muted
```

## Example config
See [i3blocks.conf](i3blocks.conf)

## More information
- If using `alsa`, only mixers associated with the default sound card will be monitored. For information on how to set your default sound card for ALSA, see this [ArchWiki article](https://wiki.archlinux.org/title/Advanced_Linux_Sound_Architecture#Set_the_default_sound_card).
- If the default PulseAudio sink is unsatisfactory, you can use `pactl list sinks` to find the correct sink name/index.
- If the default ALSA mixer is unsatisfactory, you can use `amixer` to find the correct mixer name and index.
