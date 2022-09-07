# battery
Print battery stats

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

The name of the battery to watch. Not setting this will cause the blocklet to use the first battery found on the system.
```
bat_name=BAT0
```
The text to be printed when the blocklet is functioning.
```
output_format=B: %state %percent %time
```
The text to be printed when no battery is found.
```
output_format_down=No battery
```
The color of the text when no battery is found
```
color_down=#FF0000
```
The string to be used for the `full` state.
```
state_full=FULL
```
The color of the text when the battery is full.
```
full_color=#00FF00
```
The string to be used for the `charging` state.
```
state_charging=CHR
```
The color of the text when the battery is charging.
```
charging_color=#FFFFFF
```
The string to be used for the `discharging` state.
```
state_discharging=BAT
```
The color of the text when the battery is discharging.
```
discharging_color=#FFFFFF
```
The string to be used for the `unknown` state.
```
state_unknown=UNK
```
The color of the text when the battery state is unknown
```
unknown_color=#FFFFFF
```
The threshold under which the battery is considered to be low (based on percent remaining).
```
low_threshold=30
```
The color of the text when the battery is below the low threshold mentioned above.
```
low_threshold_color=#FFFF00
```
The threshold under which the battery is considered to be critcally low (based on percent remaining).
```
critical_threshold=15
```
The color of the test when the battery is below the critical threshold mentioned above.
```
critical_threshold_color=#FF0000
```

### Variables that can be used in `output_format`

The battery's state(`full`, `charging`, `discharging`, or `unknown`).
```
%state
```
The battery's percent capacity remaining.
```
%percent
```
The battery's time remaining to empty or time remaing to full (depending on whether it's charging or discharging).
```
%time
```

## Example config
See [i3blocks.conf](i3blocks.conf)

## More information
- Although the blocklet is updated once a second, the `interval` is set to `persist` rather than `1` for performance reasons (no need to re-fetch environment variables each time).
- If no battery is found, the blocklet go to sleep. We assume that you're using the same config for a laptop and desktop and we don't want to waste CPU cycles continuously checking. You can always refresh `i3` to force the whole bar to update.
