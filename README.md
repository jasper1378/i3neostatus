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

Each element (map) in the modules section must contain a `path` option (string) which specifies the location of the module binary to load. This location may be substituted for the name of a built-in module prefixed with a underscore. Each element may also contain a `config` option (map) which will be forwarded to that module as its configuration.

Note that tildes in file paths handled by the i3neostatus itself will be resolved.

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

Currently, i3neostatus only supports bars using the i3bar protocol. Support for dzen2, xmobar, and lemonbar, etc. may be implemented in the future.

### Module development

i3neostatus aims to deliver first-class support for third-party modules by simplifying development while still allowing a great degree of freedom. Modules are C++ shared objects that are then loaded by i3neostatus at runtime. The "built-in" modules that i3neostatus ships with are implemented identically to any third-party module, meaning that they can be used a reference during the development of your own module.

The following will walk you through the development process step-by-step.

Note that i3neostatus uses (libconfigfile)[https://github.com/jasper1378/libconfigfile] to interface with its configuration file. If you wish for your module to be user-configurable, an understanding of this library is recommended.

Start by including the `i3neostatus/module_dev.hpp` header file. If i3neostatus has been installed to your system, this header should be found in `/usr/local/include` or something similar. This header contains all the declarations needed to interface with i3neostatus, including access to `libconfigfile`.

```cpp
#include <i3neostatus/module_dev.hpp>
```

The basic structure of a module is a class that inherits from `module_base`.

```cpp
class module_test : public module_base {
};
```

Before we start implementing the `module_test` class, i3neostatus needs a way to create and destroy instances of your module. This is accomplished by defining a pair of allocator and deleter functions in the global namespace of your module. Note that these functions are wrapped in an `extern "C"` block to prevent name mangling issues when i3neostatus loads you module.

```cpp
extern "C" {
module_base* allocator() {
  return new module_test{};
}

void deleter(module_base *m) {
  delete m;
}
}
```

Your module class must be default constructible. It's recommended that this constructor does little to nothing; proper initialization of your module will be preformed later.

```cpp
class module_test : public module_base {
  public:
    module_test();
};
```

Just like any other child class, your module class must have a virtual destructor. It's recommended that this destructor does little to nothing; proper termination of your module will be preformed earlier.

```cpp
class module_test : public module_base {
  public:
    virtual ~module_test();
};
```

Your module class does not need to be copyable or movable, these operations will never be preformed.

The primary way your module will communicate with i3neostatus is through the `module_api` class. Your module will receive an instance of this class during its initialization. Thus, we should familiarize ourselves with its interface before proceeding further.

There are four main data structures that will be passed between i3neostatus and your module.

`module_api::config_in` represents the user configuration of your module (see [Configuration](#configuration)). It is an alias for `libconfigfile::map_node`.

`module_api::config_out` represents the information about your module that will be passed back to i3neostatus. It is a `struct` containing the following members.
- `std::string name`
    - The name of your module (valid characters are [A-Za-z_-])
- `bool click_events_enabled`
    - Whether click events will be sent to your module

`module_api::block` represents a unit information that will be displayed on the status line. The fields of this `struct` correspond to the fields of the same name in the [i3bar protocol](https://i3wm.org/docs/i3bar-protocol.html).
- std::string full_text;
- std::optional<std::string> short_text;
- std::optional<std::string> color;
- std::optional<std::string> background;
- std::optional<std::string> border;
- std::optional<pixel_count_t> border_top;
- std::optional<pixel_count_t> border_right;
- std::optional<pixel_count_t> border_bottom;
- std::optional<pixel_count_t> border_left;
- std::optional<std::variant<pixel_count_t, std::string>> min_width;
- std::optional<std::string> align;
- std::optional<bool> urgent;
- std::optional<bool> separator;
- std::optional<pixel_count_t> separator_block_width;
- std::optional<std::string> markup;

`module_api::click_event` represents the information sent when a user clicks on a block. The fields of this `struct` correspond to the fields of the same name in the [i3bar protocol](https://i3wm.org/docs/i3bar-protocol.html).
- pixel_count_t x;
- pixel_count_t y;
- int button;
- pixel_count_t relative_x;
- pixel_count_t relative_y;
- pixel_count_t output_x;
- pixel_count_t output_y;
- pixel_count_t width;
- pixel_count_t height;
- std::vector<std::string> modifiers;

When fully completed, your module should look something like the following.

```cpp
// module_test.cpp

#include <i3neostatus/module_dev.hpp>

class module_test : public module_base {
  public:
    module_test() {
    }

    virtual ~module_test() {
    }
};

extern "C" {
module_base* allocator() {
  return new module_test{};
}

void deleter(module_base *m) {
  delete m;
}
}
```

### Misc

Please note that the wording of some parts of this documentation has been lifted from i3status due to the similarity in functionality and my hatred for writing documentation :) .

## License

See [LICENSE.md](LICENSE.md) for details.
