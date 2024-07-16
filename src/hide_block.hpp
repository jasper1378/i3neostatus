#ifndef I3NEOSTATUS_HIDE_BLOCK_HPP
#define I3NEOSTATUS_HIDE_BLOCK_HPP

#include "i3bar_data.hpp"

#include <string>

namespace i3neostatus {
namespace hide_block {
template <typename t_set> t_set set();
template <> std::string set();
template <> struct i3bar_data::block::data::module set();

template <typename t_get> bool get(const t_get &val);
template <> bool get(const std::string &string);
template <> bool get(const i3bar_data::block &block);

namespace impl {
static constexpr std::string k_hidden_block_full_text{0x18};
}; // namespace impl
} // namespace hide_block
} // namespace i3neostatus

#endif
