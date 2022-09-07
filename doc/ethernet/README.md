# ethernet
Print ethernet information

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

The name of the ethernet interface to use. The special value `_default_` will tell the blocklet to use the first ethernet interface found on the system. Not setting this option is equivalent to `_default_`.
```
interface=_default_
```
The text to be printed when ethernet is connected.
```
output_format=E: %ip
```
The text to be printed when ethernet is not connected.
```
output_format_down=E: down
```
The color of the text when ethernet is connected.
```
color_regular=#FFFFFF
```
The color of the text when ethernet is not connected.
```
color_down=#FF0000
```

### Variables that can be used in `output_format`

The IPv4 address of the ethernet connection.
```
%ip
```

## Example config
See [i3blocks.conf](i3blocks.conf)

## More information
- Although the blocklet is updated once a second, the `interval` is set to `persist` rather than `1` for performance reasons (no need to re-fetch environment variables each time).
