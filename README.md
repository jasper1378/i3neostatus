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

Note that i3neostatus uses [libconfigfile](https://github.com/jasper1378/libconfigfile) to interface with its configuration file. If you wish for your module to be user-configurable, an understanding of this library is recommended.

Throughout the following `module_test` will serve as a stand-in for the name of your module.

All i3neostatus code relevant to module development is found within the `i3neostatus::module_dev` namespace. It might be a good idea to start your module with a using-namespace declaration to save yourself some typing. The following code examples assume that this line is present.

```cpp
using namespace i3neostatus::module_dev;
```

Start by including the `i3neostatus/module_dev.hpp` header file. If i3neostatus has been installed to your system, this header should be found in `/usr/local/include` or something similar. This header contains all the declarations needed to interface with i3neostatus, including access to `libconfigfile`.

```cpp
#include <i3neostatus/module_dev.hpp>
```

The basic structure of a module is a class that inherits from `base`.

```cpp
class module_test : public base {
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
class module_test : public base {
public:
  module_test();
};
```

Just like any other child class, your module class must have a virtual destructor. It's recommended that this destructor does little to nothing; proper termination of your module will be preformed earlier.

```cpp
class module_test : public base {
public:
  virtual ~module_test();
};
```

Your module class does not need to be copyable or movable, these operations will never be preformed.

The primary way your module will communicate with i3neostatus is through the `api` class. Your module will receive a pointer to an instance of this class during its initialization. Thus, we should familiarize ourselves with its interface before proceeding further.

There are four main data structures that will be passed between i3neostatus and your module.

`config_in` represents the user configuration of your module (see [Configuration](#configuration)). It is an alias for `libconfigfile::map_node`.

`config_out` represents the information about your module that will be passed back to i3neostatus. It is a struct containing the following members.

```cpp
struct config_out {
  std::string name // The name of your module (valid characters are [A-Za-z_-])
  bool click_events_enabled // Whether click events will be sent to your module
};
```

`block` represents a unit of information that will be displayed on the status line.

```cpp
struct block {
  struct theme theme; // See below
  std::string full_text; // The text displayed on the status line. The block will be hidden if empty.
  std::optional<std::string> short_text; // The text displayed on the status line when there is insufficient space.
  std::optional<std::variant<misc_types::pixel_count_t, std::string>> min_width; // Minimum width of the block. If the contents of the block take less space than this, the block will be padded. If the value is a string, the width is determined by the width of its text.
  std::optional<misc_types::text_align> align; // How to align text when the minimum width of the block is not reached.
  std::optional<bool> urgent; // Specifies whether the current value is urgent.
  std::optional<misc_types::markup> markup; // Indicates how the text of the block should be parsed.
};
```

The `theme` struct contained in the above represents information related to the visual styling of the block.

```cpp
struct theme {
  std::optional<misc_types::color> color; // The color of the text
  std::optional<misc_types::color> background; // The color of the background
  std::optional<misc_types::color> border; // The color of the border
  std::optional<misc_types::pixel_count_t> border_top; // The width of the top border
  std::optional<misc_types::pixel_count_t> border_right; // The width of the right border
  std::optional<misc_types::pixel_count_t> border_bottom; // The width of the bottom border
  std::optional<misc_types::pixel_count_t> border_left; // The width of the left border
};
```

`click_event` represents the information sent when a user clicks on a block.

```cpp
struct click_event {
  misc_types::pixel_count_t x; // X11 root window x-coordinate of click occurrence
  misc_types::pixel_count_t y; // X11 root window y-coordinate of click occurrence
  int button; // X11 button ID
  misc_types::pixel_count_t relative_x; // Click occurrence x-coordinate relative to the top left of block
  misc_types::pixel_count_t relative_y; // Click occurrence y-coordinate relative to the top left of block
  misc_types::pixel_count_t output_x; // Click occurrence x-coordinate relative to the current output
  misc_types::pixel_count_t output_y; // Click occurrence y-coordinate relative to the current output
  misc_types::pixel_count_t width; // Width of the block
  misc_types::pixel_count_t height; // Height of the block
  misc_types::click_modifiers; // Bitset of the modifiers active when the click occurred
};
```

The following data types defined in the `misc_types` namespace are used in the above structs.

```cpp
using pixel_count_t = /*signed integer type*/; // Represents a pixel count.

struct color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
}; // Represents an RGB color.

enum class text_align {
  none, // invalid value
  center,
  right,
  left,
  max, // invalid value
}; // Represents a text alignment.

enum class markup {
  none,
  pango,
  max, // invalid value
}; // Represents a text markup.

enum class click_modifiers {
  none,
  mod1,
  mod2,
  mod3,
  mod4,
  mod5,
  shift,
  control,
  lock,
}; Represents possible click modifiers. Flag operators are defined for this type.
```

Further information on most of the fields in the above structs (`block`, `theme`, `click_event`) can be found by looking for their namesakes in the [i3bar protocol](https://i3wm.org/docs/i3bar-protocol.html).

The `api` class is movable but not copyable. Your module should never have to create a new instance of `api`.

There are two primary API functions that will be called by your module.

The first is `api::put_block()`, which is used by your module to post new information to the status line.

```cpp
void api::put_block(const block& block);
void module_apu::put_block(block&& block);
```

The second is `api::put_error()`, which is used by your module to communicate an error to i3neostatus. Note that once `api::put_error()` has been called, no further calls to `api::put_block()` or `api::put_exception()` should be made.

```cpp
void api::put_error(const std::exception& error);
void api::put_error(std::exception&& error);
void api::put_error(const std::exception_ptr& error);
void api::put_error(std::exception_ptr&& error);
```

Returning to your module, there are several virtual functions in `module_base` that must be overriden by your class. Any exceptions thrown in these functions will be handled appropriately (as if by `api::put_error()`).

The first is `init()`, which should verify user configuration and initialize your module. This function will be executed before `run()`.

```cpp
#include <exception>
#include <stdexcept>
#include <utility>

class module_test : public module_base {
private:
  api* m_api;

public:
  virtual config_out init(api* api, config_in&& config) override {
    // store pointer to API instance
    m_api = api;

    // verify user configuration
    if (...) {
        // ...
    } else {
      throw std::runtime_error{"invalid configuration"};
    }

    // return module information
    return {"module_test", true};
  }
};
```

The next is `run()`, which is the main update loop of your module. This function will be executed concurrently in its own thread by i3neostatus.

```cpp
#include <utility>

class module_test : public module_base {
private:
  api* m_api;

public:
  virtual void run() override {
    while (true) {
      // wait for new info
      // get new info

      // post new info
      block block{/*new info*/};
      m_api->put_block(std::move(block));
    }
  }
};
```

Then we have `term()`, which should signal `run()` to exit and preform any needed cleanup. Due to the nature of i3neostatus (typically runs until your computer is shut off), it is not guaranteed that this function will be called.

```cpp
class module_test : public module_base {
public:
  virtual void term() override {
    // signal run() to exit
    // cleanup
  }
};
```

Finally, there is `on_click_event()`, which will be called when someone clicks on your module. This function only needs to be overriden if you want to receive click events.

```cpp
class module_test : public module_base {
  virtual void on_click_event(click_event &&click_event) override {
    // do something based on click event
  }
};
```

Because `run()` will be executed concurrently, some level of synchronization will likely be required in your module. Note that `api::put_block()`/`api::put_error()` are thread-safe. The synchronization mechanism should at least provide a means for `term()` to signal `run()` to exit. You might also wish for `on_click_event()` to be able to wake `run()` to preform an update immediately. Though synchronization can be implemented however you wish, the following example provides a starting point.

```cpp
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <utility>

class module_test : public module_base {
private:
  enum class state {
    cont,
    wait,
    stop,
  };

private:
  api* m_api;
  state m_state;
  std::mutex m_state_mtx;
  std::condition_variable m_state_cv;

public:
  module_test()
    : m_state{state::cont}, m_state_mtx{}, m_state_cv{}
  {}

public:
  virtual void run() override {
    while (true) {
      // get new info

      // post new info
      block block{/*new info*/};
      m_api->put_block(std::move(block));

      // sleep for 1 second or until woken up to continue or exit
      std::unique_lock<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::wait;
      m_state_cv.wait_for(
        lock_m_state_mtx, std::chrono::seconds{1},
        [this]() -> bool { return m_state != state::wait; });
     if (m_state == state::stop) {
       break;
     } else {
       m_state = state::cont;
     }
    }
  }

  virtual void term() override {
    // signal run() to exit
    {
      std::lock_guard<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::stop;
    }
    m_state_cv.notify_all();

    // cleanup
  }

  virtual void on_click_event(click_event&& click_event) override  {
    // do something based on click event

    // signal run() to continue
    {
      std::lock_guard<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::cont;
    }
    m_state_cv.notify_all();
  }
};
```

When fully completed, your module should look something like the following.

```cpp
// module_test.cpp

#include <i3neostatus/module_dev.hpp>

#include <chrono>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <utility>

class module_test : public module_base {
private:
  enum class state {
    cont,
    wait,
    stop,
  };

private:
  api* m_api;
  state m_state;
  std::mutex m_state_mtx;
  std::condition_variable m_state_cv;

public:
  module_test()
    : m_api{}, m_state{state::cont}, m_state_mtx{}, m_state_cv{}
  {}

  virtual ~module_test() {
  }

public:
  virtual config_out init(api&& api, config_in&& config) override {
    m_api = api;

    if (...) {
      // verify configuration
    } else {
      throw std::runtime_error{"invalid configuration"};
    }

    return {"module_test", true};
  }

  virtual void run() override {
    while (true) {
      // get new info

      block block{/*new info*/};
      m_api->put_block(std::move(block));

      std::unique_lock<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::wait;
      m_state_cv.wait_for(
        lock_m_state_mtx, std::chrono::seconds{1},
        [this]() -> bool { return m_state != state::wait; });
     if (m_state == state::stop) {
       break;
     } else {
       m_state = state::cont;
     }
    }
  }

  virtual void term() override {
    {
      std::lock_guard<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::stop;
    }
    m_state_cv.notify_all();

    // cleanup
  }

  virtual void on_click_event(click_event&& click_event) override {
    // do something based on click event

    {
      std::lock_guard<std::mutex> lock_m_state_mtx{m_state_mtx};
      m_state = state::cont;
    }
    m_state_cv.notify_all();
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

The final step is to compile your module to a shared object that can be loaded by i3neostatus. i3neostatus uses C++20, your module should as well. If using GCC, the following command should do the trick.
```
g++ -std=c++20 -Wall -Wextra -g -O2 -fPIC -shared module_test.cpp -o module_test
```
For more complex projects, I recommend using the Makefile found here: [generic-makefile/C++/library/Makefile](https://github.com/jasper1378/generic-makefile/blob/main/C%2B%2B/library/Makefile).
This binary can be placed anywhere, however, `/usr/local/lib/i3neostatus/modules/module_test` is recommended for consistency between third-party modules. The only naming convention that the binary file must follow is that it cannot be prefixed with an underscore, as i3neostatus reserves this as a shorthand to refer to built-in modules.

As a final piece of advice, be very wary of calling any non-thread-safe library functions in your module in order to avoid race conditions with other modules that may be loaded.

### Misc

Please note that the wording of some parts of this documentation has been lifted from i3status due to the similarity in functionality and my hatred for writing documentation :) .

## License

See [LICENSE](LICENSE) for details.
