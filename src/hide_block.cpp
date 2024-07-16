#include "hide_block.hpp"

#include "i3bar_data.hpp"

#include <string>

template <> std::string i3neostatus::hide_block::set() {
  return impl::k_hidden_block_full_text;
}

template <>
struct i3neostatus::i3bar_data::block::data::module
i3neostatus::hide_block::set() {
  return (struct i3neostatus::i3bar_data::block::data::module){
      .full_text{set<std::string>()}};
}

template <> bool i3neostatus::hide_block::get(const std::string &string) {
  static_assert(impl::k_hidden_block_full_text.size() == 1);
  return ((string.size() == impl::k_hidden_block_full_text.size()) &&
          (string[0] == impl::k_hidden_block_full_text[0]));
}

template <> bool i3neostatus::hide_block::get(const i3bar_data::block &block) {
  return get(block.data.module.full_text);
}
