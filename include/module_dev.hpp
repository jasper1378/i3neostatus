#ifndef I3NEOSTATUS_MODULE_DEV_HPP
#define I3NEOSTATUS_MODULE_DEV_HPP

#include "block_state.hpp"
#include "i3bar_data.hpp"
#include "module_api.hpp"
#include "module_base.hpp"

#include "libconfigfile/libconfigfile.hpp"

namespace i3neostatus {
namespace module_dev {
using base = i3neostatus::module_base;
using api = i3neostatus::module_api;

using state = i3neostatus::block_state;
using content = i3neostatus::module_api::content;
using block = i3neostatus::module_api::block;
using click_event = i3neostatus::module_api::click_event;
using config_in = i3neostatus::module_api::config_in;
using config_out = i3neostatus::module_api::config_out;

namespace types = i3neostatus::i3bar_data::types;
} // namespace module_dev
} // namespace i3neostatus

#endif
