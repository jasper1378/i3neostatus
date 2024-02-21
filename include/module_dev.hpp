#ifndef I3NEOSTATUS_MODULE_DEV_HPP
#define I3NEOSTATUS_MODULE_DEV_HPP

#include "module_api.hpp"
#include "module_base.hpp"

namespace i3neostatus {
namespace module_dev {
using base = i3neostatus::module_base;
using api = i3neostatus::module_api;

using config_in = i3neostatus::module_api::config_in;
using config_out = i3neostatus::module_api::config_out;

using block = i3neostatus::module_api::block;
using click_event = i3neostatus::module_api::click_event;

namespace misc_types = i3neostatus::i3bar_data::types;
} // namespace module_dev
} // namespace i3neostatus

#endif
