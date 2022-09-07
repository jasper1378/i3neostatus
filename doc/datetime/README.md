# datetime
Print date and time

## Dependencies
[`date` program from `coreutils`](https://www.gnu.org/software/coreutils/date)

## Configuration options

### Required
These options must be set to the specified values for the blocklet to work.

`$i3blocks-blocklets_dir` should be set the directory containing the compiled blocklets.
```
command=$i3blocks-blocklets_dir/datetime
```
```
interval=1
```
```
markup=pango
```

### Optional
These options allow you to customize the blocklet.

The text to be printed.
```
output_format=%a %d-%m-%Y %H:%M:%S
```

### Variables that can be used in `output_format`
Any format variables that are valid in `date` are also valid here. See `man 1 date` for a complete list.

## Example config
See [i3blocks.conf](i3blocks.conf)

## More information
- The blocklet is really just a simple wrapper for `date` that outputs in `pango` markup (to be consistent with the rest of the blocklets).
