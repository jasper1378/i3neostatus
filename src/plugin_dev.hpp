#ifndef I3NEOSTATUS_PLUGIN_DEV_HPP
#define I3NEOSTATUS_PLUGIN_DEV_HPP

#include "block_state.hpp"
#include "i3bar_data.hpp"
#include "plugin_api.hpp"
#include "plugin_base.hpp"

#include "libconfigfile/libconfigfile.hpp"

#define I3NEOSTATUS_PLUGIN_DEV_DEFINE_ALLOC(plugin_)                           \
  extern "C" {                                                                 \
  i3neostatus::plugin_dev::base *allocator() { return new plugin_{}; }         \
  void deleter(i3neostatus::plugin_dev::base *m) { delete m; }                 \
  }

namespace i3neostatus {
namespace plugin_dev {
using base = i3neostatus::plugin_base;
using api = i3neostatus::plugin_api;

using state = i3neostatus::block_state;
using content = i3neostatus::plugin_api::content;
using block = i3neostatus::plugin_api::block;
using click_event = i3neostatus::plugin_api::click_event;
using config_in = i3neostatus::plugin_api::config_in;
using config_out = i3neostatus::plugin_api::config_out;

namespace types = i3neostatus::i3bar_data::types;
} // namespace plugin_dev
} // namespace i3neostatus

#endif
