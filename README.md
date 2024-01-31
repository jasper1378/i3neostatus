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

Throughout the following `module_test` will serve as a stand-in for the name of your module.

All i3neostatus code is found within the `i3neostatus` namespace. It might be a good idea to start your module with using-namespace declaration to save yourself some typing. The following code examples assume that this line is present.

```cpp
using namespace i3neostatus;
```

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

The primary way your module will communicate with i3neostatus is through the `module_api` class. Your module will receive a pointer to an instance of this class during its initialization. Thus, we should familiarize ourselves with its interface before proceeding further.

There are four main data structures that will be passed between i3neostatus and your module.

`module_api::config_in` represents the user configuration of your module (see [Configuration](#configuration)). It is an alias for `libconfigfile::map_node`.

`module_api::config_out` represents the information about your module that will be passed back to i3neostatus. It is a struct containing the following members.
```cpp
struct config_out {
  std::string name // The name of your module (valid characters are [A-Za-z_-])
  bool click_events_enabled // Whether click events will be sent to your module
};
```

`module_api::block` represents a unit of information that will be displayed on the status line. The fields of this struct correspond to the fields of the same name in the [i3bar protocol](https://i3wm.org/docs/i3bar-protocol.html).
```cpp
struct block {
  std::string full_text;
  std::optional<std::string> short_text;
  std::optional<std::string> color;
  std::optional<std::string> background;
  std::optional<std::string> border;
  std::optional<pixel_count_t> border_top;
  std::optional<pixel_count_t> border_right;
  std::optional<pixel_count_t> border_bottom;
  std::optional<pixel_count_t> border_left;
  std::optional<std::variant<pixel_count_t, std::string>> min_width;
  std::optional<std::string> align;
  std::optional<bool> urgent;
  std::optional<bool> separator;
  std::optional<pixel_count_t> separator_block_width;
  std::optional<std::string> markup;
};
```

`module_api::click_event` represents the information sent when a user clicks on a block. The fields of this struct correspond to the fields of the same name in the [i3bar protocol](https://i3wm.org/docs/i3bar-protocol.html).
```cpp
struct click_event {
  pixel_count_t x;
  pixel_count_t y;
  int button;
  pixel_count_t relative_x;
  pixel_count_t relative_y;
  pixel_count_t output_x;
  pixel_count_t output_y;
  pixel_count_t width;
  pixel_count_t height;
  std::vector<std::string> modifiers;
};
```

Note that the `pixel_count_t` data type in the above structs is an alias for a signed integer type (currently `long`).

The `module_api` class is movable but not copyable. Your module should never have to create a new instance of `module_api`.

There are two primary API functions that will be called by your module.

The first is `module_api::put_block()`, which is used by your module to post new information to the status line.
```cpp
void module_api::put_block(const block& block);
void module_apu::put_block(block&& block);
```

The second is `module_api::put_error()`, which is used by your module to communicate an error to i3neostatus. Note that once `module_api::put_error()` has been called, no further calls to `module_api::put_block()` or `module_api::put_exception()` should be made.
```cpp
void module_api::put_error(const std::exception& error);
void module_api::put_error(std::exception&& error);
void module_api::put_error(const std::exception_ptr& error);
void module_api::put_error(std::exception_ptr&& error);
```

Returning to your module, there are several virtual functions in `module_base` that must be overriden by your class. Any exceptions thrown in these functions will be handled appropriately (as if by `module_api::put_error()`).

The first is `init()`, which should verify user configuration and initialize your module. This function will be executed before `run()`.
```cpp
#include <exception>
#include <stdexcept>
#include <utility>

class module_test : public module_base {
private:
  module_api* m_api;

public:
  virtual module_api::config_out init(module_api* api, module_api::config_in&& config) override {
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
  module_api* m_api;

public:
  virtual void run() override {
    while (true) {
      // wait for new info
      // get new info

      // post new info
      module_api::block block{/*new info*/};
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
  virtual void on_click_event(module_api::click_event &&click_event) override {
    // do something based on click event
  }
};
```

Because `run()` will be executed concurrently, some level of synchronization will likely be required in your module. Note that `module_api::put_block()`/`module_api::put_error()` are thread-safe. The synchronization mechanism should at least provide a means for `term()` to signal `run()` to exit. You might also wish for `on_click_event()` to be able to wake `run()` to preform an update immediately. Though synchronization can be implemented however you wish, the following example provides a starting point.
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
  module_api* m_api;
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
      module_api::block block{/*new info*/};
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

  virtual void on_click_event(module_api::click_event&& click_event) override  {
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
  module_api* m_api;
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
  virtual module_api::config_out init(module_api&& api, module_api::config_in&& config) override {
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

      module_api::block block{/*new info*/};
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

  virtual void on_click_event(module_api::click_event&& click_event) override {
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
This binary can be placed anywhere, however, `/usr/local/lib/i3neostatus_modules/module_test` is recommended for consistency between third-party modules. The only naming convention that the binary file must follow is that it cannot be prefixed with an underscore, as i3neostatus reserves this as a shorthand to refer to built-in modules.

As a final piece of advice, be very wary of calling any non-thread-safe library functions in your module in order to avoid race conditions with other modules that may be loaded.

### Misc

Please note that the wording of some parts of this documentation has been lifted from i3status due to the similarity in functionality and my hatred for writing documentation :) .

## License

See [LICENSE](LICENSE) for details.
