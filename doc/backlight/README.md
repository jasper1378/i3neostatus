# backlight
Print backlight brightness

## Dependencies
[light](https://github.com/haikarainen/light)

## Configuration options

### Required
These options must be set to the specified values for the blocklet to work.

`$i3blocks-blocklets_dir` should be set the directory containing the compiled blocklets.
```
command=$i3blocks-blocklets_dir/backlight
```
```
interval=persist
```
```
markup=pango
```

### Optional
These options allow you to customize the blocklet.

The text to be printed when the blocklet is functioning.
```
output_format=B: %brightness
```
The text to be printed when the blocklet is not functioning.
```
output_format_down=No backlight
```
The color of the text when the blocklet is functioning.
```
color=#FFFFFF
```
The color of the text when the blocklet is not functioning.
```
color_down=#FF0000
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
- If `light` is not installed, the blocklet will switch into non-functioning mode. However if `light` is installed but no backlight controller is found `%brightness` will be set to `0%`.

## Bonus
The following can be inserted into your `i3` config to enable changing your backlight brightness via keybinds while automatically sending `SIGUSR1` to the blocklet to update it.
```
set $refresh_i3blocks_backlight killall -SIGUSR1 backlight
bindsym XF86MonBrightnessUp exec --no-startup-id light -A 5 && $refresh_i3blocks_backlight
bindsym XF86MonBrightnessDown exec --no-startup-id light -U 5 && $refresh_i3blocks_backlight
```
