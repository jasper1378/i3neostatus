# i3neostatus

## About

i3neostatus is a powerful and modular i3status replacement.

## Installation

Acquire the sources
```
$ git clone https://github.com/jasper1378/i3neostatus.git
$ cd i3neostatus
```
Build
```
$ make
```
Install
```
$ make install
```

## Usage

i3neostatus is a replacement for i3status that provides a way to display a status line on bars that support the i3bar protocol. Unlike i3status, the design of i3neostatus emphasizes support for third-party modules and asynchronous updates. i3neostatus aims to posses full feature parity with i3status (and then some) while maintaining a high degree of efficiency.

### Command-line options

- `-h`/`--help`
    - Print a help message
- `-v`/`--version`
    - Print version information
- `-c`/`--config`
    - Specify an alternate configuration file path. By default, i3neostatus looks for a configuration files in the following order:
        1. `~/.config/i3neostatus/config` (or `$XDG_CONFIG_HOME/i3neostatus/config` if set)
        2. `/etc/xdg/i3status/config` (or `$XDG_CONFIG_DIRS/i3neostatus/config` if set)
        3. `~/.i3neostatus.conf`
        4. `/etc/i3neostatus.conf`

### Configuration

The basic idea of i3neostatus is that you can specify which "modules" should be used. You can then configure each module with its own section. Note that i3neostatus uses the [libconfigfile](https://github.com/jasper1378/libconfigfile) syntax specification for its configuration file.

The configuration file has two main sections: `general` (map), which contains global options affecting the whole program; and `modules` (array) which contains the configuration for each module.

Currently, there are no options implemented in `general`.

Each element (map) in the modules section must contain a `path` option (string) which specifies the location of the module binary to load. This location may be substituted for the name of a built-in module prefixed with a underscore. Each element may also contain a `config` option (map) which will be forwarded to that module.

#### Sample configuration

```
general = {
};

modules = [
    {
        path = "_wireless";
        config = {
            interface = "wlan0";
            format_up = "W: (%quality at %essid, %bitrate) %ip";
            format_down = "W: down";
        };
    },
    {
        path = "_ethernet";
        config = {
            interface = "eth0";
            format_up = "E: %ip (%speed)";
            format_down = "E: down";
        };
    },
    {
        path = "_battery";
        config = {
            battery = "0";
            format = "%status %percentage %remaining %emptytime";
            format_down = "No battery";
            status_chr = "⚡ CHR";
            status_bat = "🔋 BAT";
            status_unk = "? UNK";
            status_full = "☻ FULL";
            path = "/sys/class/power_supply/BAT%d/uevent";
            low_threshold = 10;
        };
    },
    {
        path = "_run_watch";
        config = {
            pidfile = "/var/run/dhclient*.pid";
        };
    },
    {
        path = "_run_watch";
        config = {
            # file containing the PID of a vpnc process
            pidfile = "/var/run/vpnc/pid";
        };
    },
    {
        path = "_path_exists";
        config = {
            # path exists when a VPN tunnel launched by nmcli/nm-applet is active
            path = "/proc/sys/net/ipv4/conf/tun0";
        },
    },
    {
        path = "_tztime";
        config = {
            format = "%Y-%m-%d %H:%M:%S";
            hide_if_equals_localtime = 1;
        };
    },
    {
        path = "_tztime";
        config = {
            format = "%Y-%m-%d %H:%M:%S %Z";
            timezone = "Europe/Berlin";
        };
    },
    {
        path = "_load";
        config = {
            format = "%5min";
        };
    },
    {
        path = "_cpu_temperature";
        config = {
            format = "T: %degrees °C";
            path = "/sys/devices/platform/coretemp.0/temp1_input";
        };
    },
    {
        path = "_memory";
        config = {
            format = "%used";
            threshold_degraded = "10%";
            format_degraded = = "MEMORY: %free";
        };
    },
    {
        path = "_disk";
        config = {
            filesystem = "/";
            format = "%free";
        };
    },
    {
        path = "_read_file";
        config = {
            path = "/proc/uptime";
        };
    },
];
```

### Available modules

Currently no modules have been implemented :)

### Bar support

Currently, i3neostatus only supports bars using the i3bar protocol. Support for dzen2, xmobar, and lemonbar may be implemented in the future.

### Module development

### Misc

Please note that the wording of some parts of this documentation has been lifted from i3status due to the similarity in functionality and my hatred for writing documentation :) .

## License

See [LICENSE.md](LICENSE.md) for details.
