# backlight
Print backlight brightness

## Dependencies
None

## Configuration options

### Required
These options must be set to the specified values for the blocklet to work.

`$i3blocks_blocklets_dir` should be set the directory containing the compiled blocklets.
```
command=$i3blocks_blocklets_dir/backlight
```
```
interval=persist
```
```
markup=pango
```

### Optional
These options allow you to customize the blocklet.

The backlight device to monitor. It should refer to a directory under `/sys/class/backlight/`. The special value `_default_` will tell the blocklet to use the first backlight device found. Not setting this option is equivalent to `_default_`.
```
backlight_device=_default_
```
The text to be printed when the blocklet is functioning.
```
output_format=B: %brightness
```
The text to be printed when no backlight device is found.
```
output_format_down=No backlight
```
The color of the text when the blocklet is functioning.
```
color=#FFFFFF
```
The color of the text when no backlight device is found.
```
color_down=#FF0000
```
Whether the brightness value should be printed with decimals or not. Valid options are `never`, `always`, and `auto`. The `auto` option will cause the brightness to be printed with decimals only if it's not an integer. Note that the number of decimals places will always be either 0 or 2.
```
decimals=never
```

### Variables that can be used in `output_format`

The brightness of the backlight.
```
%brightness
```

## Example config
See [i3blocks.conf](i3blocks.conf)

## More information
- Although the blocklet is update once a second, the `interval` is set to `persist` rather than `1` for performance reasons (no need to re-fetch environment variables each time).
- The blocklet will also be updated if it receives a `SIGUSR1` signal. Note that signal should be sent to the blocklet itself, not `i3blocks`.

## Bonus
The following can be inserted into your `i3` config to enable changing your backlight brightness via keybinds while automatically sending `SIGUSR1` to the blocklet to update it.
```
set $refresh_i3blocks_backlight killall -SIGUSR1 backlight
bindsym XF86MonBrightnessUp exec --no-startup-id light -A 5 && $refresh_i3blocks_backlight
bindsym XF86MonBrightnessDown exec --no-startup-id light -U 5 && $refresh_i3blocks_backlight
```
