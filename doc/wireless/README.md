# wireless
Print wireless information

## Dependencies
None

## Configuration options

### Required
These options must be set to the specified values for the blocklet to work.

`$i3blocks_blocklets_dir` should be set the directory containing the compiled blocklets.
```
command=$i3blocks_blocklets_dir/battery
```
```
interval=persist
```
```
markup=pango
```

### Optional
These options allow you to customize the blocklet.

The name of the wireless interface to use. The specifal value `_default_` will tell the blocklets to use the first wireless interface found on the system. Not setting this option is equivalent to `_default_`.
```
interface=_default_
```
The text to be printed when wireless is connected.
```
output_format=W: (%quality at %essid %frequency) %ip
```
The text to be printed when wireless is not connected.
```
output_format_down=W: down
```
The color of the text when wireless is connected.
```
color_regular=#FFFFFF
```
The threshold under which the wireless signal is to be considered poor quality (based on percent signal quality).
```
threshold_low_quality=50
```
The color of the text when the wireless signal quality is below the threshold specified above.
```
color_low_quality=#FFFF00
```
The color of the text when wireless is not connected.
```
color_down=#FF0000
```

### Variables that can be used in `output_format`

The quality of the wireless signal.
```
%quality
```
The ESSID of the connected network.
```
%essid
```
The frequency of the connected network.
```
%frequency
```
The IPv4 address of the wireless connection.
```
%ip
```

## Example config
See [i3blocks.conf](i3blocks.conf)

## More information
- Although the blocklet is updated once a second, the `interval` is set to `persist` rather than `1` for performance reasons (no need to re-fetch environment variables each time).
