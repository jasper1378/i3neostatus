# i3neostatus

## About

i3neostatus is a powerful and modular i3status replacement.

## Installation

### 1a. Acquire the sources (git repository).

```
$ git clone https://github.com/jasper1378/i3neostatus.git
$ cd i3neostatus
$ git submodule update --init --recursive
$ autoreconf -i
```

### 1b. Acquire the sources (distribution tarball).

```
$ tar -xvf i3neostatus.tar.gz
$ cd i3neostatus
```

### 2. Configure, build, and install.

```
$ mkdir build
$ cd build
$ ../configure # try `--help` for options
$ make
$ sudo make install
```

## Usage

i3neostatus is a replacement for i3status that provides a way to display a status line on bars that support the i3bar protocol. Unlike i3status, the design of i3neostatus emphasizes support for third-party plugins and asynchronous updates. I3neostatus aims to posses full feature parity with i3status (and then some) while maintaining a high degree of efficiency.

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

The basic idea of i3neostatus is that you can specify which "plugins" should be used. You can then configure each plugin with its own section. Note that i3neostatus uses the [libconfigfile](https://github.com/jasper1378/libconfigfile) syntax specification for its configuration file.

The configuration file has three main sections: `general` (`map`), which contains global options affecting the whole program; `theme` (`map`) which specifies the appearance of the status line; and `plugins` (`array`) which contains the configuration for each plugin.

The only option currently implemented in `general` is `custom_separators` (`integer`). Setting this to a non-zero value (default) will enable custom separators. Otherwise, the default i3bar separators are used.

The `theme` sections contains a variety of options that affect the styling of the status line. All options are optional (pun unintentional), those not set will possess default values.

A summary of these options is below.

| Name | Type | Description |
| --- | --- | --- |
| `<state>_color_foreground` | `string` (color) | Foreground color of plugins in `<state>` state.
| `<state>_color_background` | `string` (color) | Background color of plugins in `<state>` state.
| `<state>_color_border` | `string` (color or special) | Border color of plugins in `<state>` state.
| `alternating_tint_foreground` | `string` (color) | Added to the foreground color of every other block.
| `alternating_tint_background` | `string` (color) | Added to the background color of every other block.
| `alternating_tint_border` | `string` (color or special) | Added to the border color of every other block.
| `separator_<location>_sequence` | `string` (color) | String used as separator in `<location>` position.
| `separator_<location>_color_foreground` | `string` (color or special) | Foreground color of separator in `<location>` position.
| `separator_<location>_color_background` | `string` (color or special) | Background color of separator in `<location>` position.
| `border_width_<position>` | `integer` | Width of the `<position>` border around plugins. Units are pixels.

Strings specifying colors should be in hexadecimal RGBA format (i.e., they should match `^#?[0-9a-fA-F]{8}$`).

In the context of plugins, `<state>` can be one of: `idle`, `info`, `good`, `warning`, `critical`, `error`. In the context of separators, `<location>` can be one of: `begin`, `middle`, `end`. In the context of borders, `<position>` can be one of: `top`, `right`, `bottom`, `left`.

In addition to a color, `<state>_color_border` and `alternating_tint_boder` may also be specified as one of two 'special' strings: `"foreground"` (take color from foreground) or `"background"` (take color from background). In addition to a color, `separator_<location>_color_ground` and `separator_<location>_color_background` may also be specified as one of two 'special' strings: `"right"` (take color from right block) or `"left"` (take color from left block). Note that `begin` separators can't use `"left"` and `end` separators can't use `"right"`.

Each element (`map`) in the `plugins` section must contain a `path_or_name` option (`string`) which specifies the location of the plugin binary to load or the name of a built-in plugin suffixed with a underscore. Each element may also contain a `config` option (`map`) which will be forwarded to that plugin as its configuration.

Note that tildes in file paths handled by i3neostatus itself will be resolved.

#### Sample configuration

```
general = {
};

theme = {
    idle_color_foreground = "#FFFFFFFF";
    idle_color_background = "#000000FF";
    idle_color_border = "background";

    info_color_foreground = "#FFFFFFFF";
    info_color_background = "#000000FF";
    info_color_border = "background";

    good_color_foreground = "#FFFFFFFF";
    good_color_background = "#000000FF";
    good_color_border = "background";

    warning_color_foreground = "#FFFFFFFF";
    warning_color_background = "#000000FF";
    warning_color_border = "background";

    critical_color_foreground = "#FFFFFFFF";
    critical_color_background = "#000000FF";
    critical_color_border = "background";

    error_color_foreground = "#FFFFFFFF";
    error_color_background = "#000000FF";
    error_color_border = "background";

    alternating_tint_color_foreground = "#00000000";
    alternating_tint_color_background = "#00000000";
    alternating_tint_color_border = "#00000000";

    separator_middle_sequence = " | ";
    separator_middle_color_foreground = "#FFFFFFFF";
    separator_middle_color_background = "#000000FF";

    separator_begin_sequence = "";
    separator_begin_color_foreground = "#FFFFFFFF";
    separator_begin_color_background = "#000000FF";

    separator_end_sequence = "";
    separator_end_color_foreground = "#FFFFFFFF";
    separator_end_color_background = "#000000FF";

    border_width_top = 1;
    border_width_right = 1;
    border_width_bottom = 1;
    border_width_left = 1;
};

plugins = [
    {
        path = "_simple_date";
        config = {
            format = "%FT%TZ";
        };
    }
];
```

### Available plugins

Currently no plugins have been implemented :)

### Bar support

Currently, i3neostatus only supports bars using the i3bar protocol. Support for dzen2, xmobar, and lemonbar, etc. may be implemented in the future.

### plugin development

i3neostatus aims to deliver first-class support for third-party plugins by simplifying development while still allowing a great degree of freedom. plugins are C++ shared objects that are then loaded by i3neostatus at runtime. The "built-in" plugins that i3neostatus ships with are implemented identically to any third-party plugin, meaning that they can be used a reference during the development of your own plugin.

The following will walk you through the development process step-by-step.

plugins are not responsible for setting their own theme, instead they pass a "state" value to i3neostatus (see below), which then uses this value, as well as a user specified global theme, to determine the visual styling of the plugin. If your plugin wants to override this theme, you can make use of the Pango markup option (see below). This is discouraged as it interferes with a cohesive status line appearance and may break certain user settings affecting separator color.

Note that i3neostatus uses [libconfigfile](https://github.com/jasper1378/libconfigfile) to interface with its configuration file. If you wish for your plugin to be user-configurable, familiarity with this library is recommended.

Throughout the following `test_plugin` will serve as a stand-in for the name of your plugin.

All i3neostatus code relevant to plugin development is found within the `i3neostatus::plugin_dev` namespace. It might be a good idea to start your plugin with a namespace alias to save yourself some typing. The following code examples assume that this line is present.

```cpp
namespace i3ns = i3neostatus::plugin_dev;
```

Start by including the `i3neostatus/plugin_dev.hpp` header file. If i3neostatus has been installed to your system, this header should be found in `/usr/local/include` or something similar. This header contains all the declarations needed to interface with i3neostatus, including access to `libconfigfile`.

```cpp
#include <i3neostatus/plugin_dev.hpp>
```

The basic structure of a plugin is a class that inherits from `i3ns::base`.

```cpp
class test_plugin : public i3ns::base {
};
```

Before we start implementing the `test_plugin` class, i3neostatus needs a way to create and destroy instances of your plugin. This is handled for you by invoking the macro `I3NEOSTATUS_PLUGIN_FACTORY_DEFINE()` with an argument corresponding to the name of your plugin class.

```cpp
I3NEOSTATUS_PLUGIN_FACTORY_DEFINE(test_plugin);
```

Your plugin class must be default constructible. It's recommended that this constructor does little to nothing; proper initialization of your plugin will be preformed later.

```cpp
class test_plugin : i3ns::public base {
public:
  test_plugin();
};
```

Just like any other child class, your plugin class must have a virtual destructor. It's recommended that this destructor does little to nothing; proper termination of your plugin will be preformed earlier.

```cpp
class test_plugin : i3ns::public base {
public:
  virtual ~test_plugin();
};
```

Your plugin class does not need to be copyable or movable, these operations will never be preformed.

The primary way your plugin will communicate with i3neostatus is through the `i3ns::api` class. Your plugin will receive a pointer to an instance of this class during its initialization. Thus, we should familiarize ourselves with its interface before proceeding further.

There are several main data structures that will be passed between i3neostatus and your plugin.

`i3ns::config_in` represents the user configuration of your plugin (see [Configuration](#configuration)). It is an alias for `libconfigfile::map_node`.

`i3ns::config_out` represents the information about your plugin that will be passed back to i3neostatus. It is a struct containing the following members.

```cpp
struct i3ns::config_out {
  bool click_events_enabled // Whether click events will be sent to your plugin
};
```

`i3ns::state` represents the current state of your plugin.

```cpp
enum class i3ns::state {
  idle = 0,
  info = 1,
  good = 2,
  warning = 3,
  critical = 4,
  error = 5,
  max = 6, // invalid value
};
```

`i3ns::content` represents a unit of information that will be displayed on the status line.

```cpp
struct i3ns::content {
  std::string full_text; // The text displayed on the status line.
  std::optional<std::string> short_text; // The text displayed on the status line when there is insufficient space.
  std::optional<std::variant<i3ns::types::pixel_count_t, std::string>> min_width; // Minimum width of the block. If the contents of the block take less space than this, the block will be padded. If the value is a string, the width is determined by the width of its text.
  std::optional<i3ns::types::text_align> align; // How to align text when the minimum width of the block is not reached.
  std::optional<bool> urgent; // Specifies whether the current value is urgent.
  std::optional<i3ns::types::markup> markup; // Indicates how the text of the block should be parsed.
};
```

`i3ns::block` contains the primary information passed between your plugin and i3neostatus.

```cpp
using i3ns::block = std::pair<i3ns::content, i3ns::state>;
```

`i3ns::click_event` represents the information sent when a user clicks on a block.

```cpp
struct i3ns::click_event {
  i3ns::types::pixel_count_t x; // X11 root window x-coordinate of click occurrence
  i3ns::types::pixel_count_t y; // X11 root window y-coordinate of click occurrence
  int button; // X11 button ID
  i3ns::types::pixel_count_t relative_x; // Click occurrence x-coordinate relative to the top left of block
  i3ns::types::pixel_count_t relative_y; // Click occurrence y-coordinate relative to the top left of block
  i3ns::types::pixel_count_t output_x; // Click occurrence x-coordinate relative to the current output
  i3ns::types::pixel_count_t output_y; // Click occurrence y-coordinate relative to the current output
  i3ns::types::pixel_count_t width; // Width of the block
  i3ns::types::pixel_count_t height; // Height of the block
  i3ns::types::click_modifiers; // Bitset of the modifiers active when the click occurred
};
```

The following data types defined in the `i3ns::types` namespace are used in the above structs.

```cpp
using i3ns::pixel_count_t = /*signed integer type*/; // Represents a pixel count.

enum class i3ns::text_align {
  none, // invalid value
  center,
  right,
  left,
  max, // invalid value
}; // Represents a text alignment.

enum class i3ns::markup {
  none,
  pango,
  max, // invalid value
}; // Represents a text markup.

enum class i3ns::click_modifiers {
  none,
  mod1,
  mod2,
  mod3,
  mod4,
  mod5,
  shift,
  control,
  lock,
}; // Represents possible click modifiers. Flag operators are defined for this type.
```

Further information on most of the fields in the above structs (`i3ns::content`, `i3ns::click_event`, etc.) can be found by looking for their namesakes in the [i3bar protocol](https://i3wm.org/docs/i3bar-protocol.html).

The `i3ns::api` class is movable but not copyable. Your plugin should never have to create a new instance of `i3ns::api`.

There are three primary API functions that will be called by your plugin.

The first is `i3ns::api::put_block()`, which is used by your plugin to post new information to the status line.

```cpp
void i3ns::api::put_block(const i3ns::block& block);
void i3ns::api::put_block(i3ns::block&& block);
```

The second is `i3ns::api::put_error()`, which is used by your plugin to communicate an error to i3neostatus. Note that once `i3ns::api::put_error()` has been called, no further calls to `i3ns::api::put_block()` or `api::put_error()` should be made.

```cpp
void i3ns::api::put_error(const std::exception& error);
void i3ns::api::put_error(std::exception&& error);
void i3ns::api::put_error(const std::exception_ptr& error);
void i3ns::api::put_error(std::exception_ptr&& error);
```

The last is `i3ns::api::hide()`, which is used to temporarily hide your plugin on the status line. Call `i3ns::api::put_block()` or `i3ns::api::put_error()` to make it visible again.

```cpp
void i3ns::api::hide();
```

Returning to your plugin, there are several virtual functions in `i3ns::base` that must be overriden by your class. Any exceptions thrown in these functions will be handled appropriately (as if by `i3ns::api::put_error()`).

The first is `init()`, which should verify user configuration and initialize your plugin. This function will be executed before `run()`.

```cpp
#include <stdexcept>

class test_plugin : public i3ns::base {
private:
  i3ns::api* m_api;

public:
  virtual i3ns::config_out init(i3ns::api* api, i3ns::config_in&& config) override {
    // store pointer to API instance
    m_api = api;

    // verify user configuration
    if (...) {
        // ...
    } else {
      throw std::runtime_error{"invalid configuration"};
    }

    // return plugin information
    return {.click_events_enabled{true}};
  }
};
```

The next is `run()`, which is the main update loop of your plugin. This function will be executed concurrently in its own thread by i3neostatus.

```cpp
#include <utility>

class test_plugin : public i3ns::base {
private:
  i3ns::api* m_api;

public:
  virtual void run() override {
    while (true) {
      // wait for new info
      // get new info

      // post new info
      i3ns::block block{/*new info*/};
      m_api->put_block(std::move(block));
    }
  }
};
```

Then we have `term()`, which should signal `run()` to exit and preform any needed cleanup. Due to the nature of i3neostatus (typically runs until your computer is shut off), it is not guaranteed that this function will be called.

```cpp
class test_plugin : public i3ns::base {
public:
  virtual void term() override {
    // signal run() to exit
    // cleanup
  }
};
```

Finally, there is `on_click_event()`, which will be called when a user clicks on your plugin. This function only needs to be overriden if you want to receive click events.

```cpp
class test_plugin : public i3ns::base {
  virtual void on_click_event(i3ns::click_event &&click_event) override {
    // do something based on click event
  }
};
```

Because `run()` will be executed concurrently, some level of synchronization will likely be required in your plugin. Note that `i3ns::api::put_block()`/`i3ns::api::put_error()` are thread-safe. The synchronization mechanism should at least provide a means for `term()` to signal `run()` to exit. You might also wish for `on_click_event()` to be able to wake `run()` to preform an update immediately. Though synchronization can be implemented however you wish, the following example provides a starting point.

```cpp
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <utility>

class test_plugin : public i3ns::base {
private:
  enum class action {
    cont,
    wait,
    stop,
  };

private:
  i3ns::api* m_api;
  action m_action;
  std::mutex m_action_mtx;
  std::condition_variable m_action_cv;

public:
  test_plugin()
    : m_action{action::cont}, m_action_mtx{}, m_action_cv{}
  {}

public:
  virtual void run() override {
    while (true) {
      // get new info

      // post new info
      i3ns::block block{/*new info*/};
      m_api->put_block(std::move(block));

      // sleep for 1 second or until woken up to continue or exit
      std::unique_lock<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::wait;
      m_action_cv.wait_for(
        lock_m_action_mtx, std::chrono::seconds{1},
        [this]() -> bool { return m_action != action::wait; });
     if (m_action == action::stop) {
       break;
     } else {
       m_action = action::cont;
     }
    }
  }

  virtual void term() override {
    // signal run() to exit
    {
      std::lock_guard<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::stop;
    }
    m_action_cv.notify_all();

    // cleanup
  }

  virtual void on_click_event(i3ns::click_event&& click_event) override  {
    // do something based on click event

    // signal run() to continue
    {
      std::lock_guard<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::cont;
    }
    m_action_cv.notify_all();
  }
};
```

When fully completed, your plugin should look something like the following.

```cpp
// test_plugin.cpp

#include <i3neostatus/plugin_dev.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <utility>

class test_plugin : public i3ns::base {
private:
  enum class action {
    cont,
    wait,
    stop,
  };

private:
  i3ns::api* m_api;
  action m_action;
  std::mutex m_action_mtx;
  std::condition_variable m_action_cv;

public:
  test_plugin()
    : m_api{}, m_action{action::cont}, m_action_mtx{}, m_action_cv{}
  {}

  virtual ~test_plugin() {
  }

public:
  virtual i3ns::config_out init(i3ns::api&& api, i3ns::config_in&& config) override {
    m_api = api;

    if (...) {
      // verify configuration
    } else {
      throw std::runtime_error{"invalid configuration"};
    }

    return {.click_events_enabled{true}};
  }

  virtual void run() override {
    while (true) {
      // get new info

      i3ns::block block{/*new info*/};
      m_api->put_block(std::move(block));

      std::unique_lock<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::wait;
      m_action_cv.wait_for(
        lock_m_action_mtx, std::chrono::seconds{1},
        [this]() -> bool { return m_action != action::wait; });
     if (m_action == action::stop) {
       break;
     } else {
       m_action = action::cont;
     }
    }
  }

  virtual void term() override {
    {
      std::lock_guard<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::stop;
    }
    m_action_cv.notify_all();

    // cleanup
  }

  virtual void on_click_event(i3ns::click_event&& click_event) override {
    // do something based on click event

    {
      std::lock_guard<std::mutex> lock_m_action_mtx{m_action_mtx};
      m_action = action::cont;
    }
    m_action_cv.notify_all();
  }
};

I3NEOSTATUS_PLUGIN_FACTORY_DEFINE(test_plugin);
```

The final step is to compile your plugin as a shared library that can be dynamically loaded by i3neostatus. I3neostatus uses C++20, your plugin should as well.

The plugin binary file path cannot contain a trailing underscore, as i3neostatus reserves this as a means to refer to built-in plugins.

As a final piece of advice, be very wary of calling any non-thread-safe library functions in your plugin in order to avoid race conditions with other plugins that may be loaded.

### Misc

Please note that the wording of some parts of this documentation has been lifted from i3status due to the similarity in functionality and my hatred for writing documentation :) .

## License

See [LICENSE](LICENSE) for details.
