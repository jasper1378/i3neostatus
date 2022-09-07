# volume
Print volume using PulseAudio

## Dependencies
[PulseAudio](https://www.freedesktop.org/wiki/Software/PulseAudio/)

## Configuration options

### Required
These options must be set to the specified values for the blocklets to work.

`$i3blocks-blocklets_dir` should be set the directory containing the compiled blocklets.
```
command=$i3blocks-blocklets_dir/battery
```
```
interval=persist
```
```
markup=pango
```

### Optional
These options allow you to customize the blocklet.

The id of the sink to use. The special value `_default_` will tell the blocklet to use the id of the default sink. Not setting this option is equivalent to `_default_`. The sink id should match with the sink name below.
```
sink_idx=_default_
```
The name of the sink to use. The special value `_default_` will tell the blocklet to use the name of the default sink. Not setting this options is equivalent to `_default_`. The sink name should match with the sink id above.
```
sink_name=_default_
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
- You usually want to set `sink_idx` and `sink_name` to `_default_` as there is no way to differentiate between an incorrect `sink_idx`/`sink_name` and PulseAudio taking a while to grab the requested information.
- If the default sink is unsatisfactory, you can use `pactl list` to find the correct `sink_idx` and `sink_name`.
