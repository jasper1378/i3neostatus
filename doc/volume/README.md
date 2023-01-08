# volume
Print volume using PulseAudio

## Dependencies
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

The volume API to use. Available values are `pulseaudio` and `alsa`. The default value is `pulseaudio`. Please note that ALSA support has NOT BEEN IMPLEMENTED yet, this is simply a provision for when that does happen.
```
volume_api=pulseaudio
```
The audio device to monitor. If using `pulseaudio` this can be a sink name or sink id; once alsa support is added, this could be an equivalent identifier. The special value `_default_` refers to default pulseaudio or alsa device, depending on what `volume_api` is set to. Not setting this option is equivalent to `_default_`.
```
device_id=_default_
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

The name of the current device (i.e. "Speakers", "Headphones", etc.).
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
- If the default pulseaudio sink is unsatisfactory, you can use `pactl list sinks` to find the correct sink name/id.
