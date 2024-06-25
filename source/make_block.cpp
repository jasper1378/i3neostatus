#include "make_block.hpp"

#include "block_state.hpp"
#include "i3bar_data.hpp"
#include "module_id.hpp"
#include "theme.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <variant>

struct i3neostatus::i3bar_data::block::data::program
i3neostatus::make_block::content(const theme::theme &theme,
                                 const block_state state, const bool tint,
                                 bool custom_separators) {
  return (struct i3bar_data::block::data::program){
      .global{impl::global(custom_separators)},
      .theme{impl::content_theme(theme, state, tint)}};
}

struct i3neostatus::i3bar_data::block i3neostatus::make_block::separator(
    const theme::theme &theme,
    const struct i3bar_data::block::data::program::theme *left,
    const struct i3bar_data::block::data::program::theme *right) {
  assert(left || right);
  const theme::separator_type type{[left, right]() -> theme::separator_type {
    if (left && right) {
      return theme::separator_type::middle;
    } else if (left) {
      return theme::separator_type::end;
    } else if (right) {
      return theme::separator_type::begin;
    } else {
      throw std::runtime_error{"impossible!"};
    }
  }()};

  return i3bar_data::block{
      .id{.name{}, .instance{module_id::null}},
      .data{
          .program{
              .global{impl::global(true)},
              .theme{impl::separator_theme(theme, type, left, right)},
          },
          .module{
              .full_text{
                  [&theme, type]()
                      -> decltype(i3bar_data::block::data::module::full_text) {
                    switch (type) {
                    case theme::separator_type::begin: {
                      return theme.separator_begin_sequence;
                    } break;
                    case theme::separator_type::middle: {
                      return theme.separator_middle_sequence;
                    } break;
                    case theme::separator_type::end: {
                      return theme.separator_end_sequence;
                    } break;
                    }
                  }()},
              .short_text{std::nullopt},
              .min_width{std::nullopt},
              .align{std::nullopt},
              .urgent{false},
              .markup{i3bar_data::types::markup::none}}}

  };
}

struct i3neostatus::i3bar_data::block::data::program::theme
i3neostatus::make_block::impl::content_theme(const theme::theme &theme,
                                             const block_state state,
                                             const bool apply_tint) {
  const std::size_t state_as_index{static_cast<std::size_t>(state)};

  const theme::color foreground{
      maybe_apply_tint(theme.state_dependent_color_foreground[state_as_index],
                       theme.alternating_tint_color_foreground, apply_tint)};
  const theme::color background{
      maybe_apply_tint(theme.state_dependent_color_background[state_as_index],
                       theme.alternating_tint_color_background, apply_tint)};

  return (struct i3bar_data::block::data::program::theme){
      .color{foreground},
      .background{background},
      .border{impl::maybe_apply_tint(
          [&theme, state_as_index, &foreground, &background]() -> theme::color {
            switch (
                theme.state_dependent_color_border[state_as_index].index()) {
            case 0: {
              return std::get<0>(
                  theme.state_dependent_color_border[state_as_index]);
            } break;
            case 1: {
              switch (std::get<1>(
                  theme.state_dependent_color_border[state_as_index])) {
              case theme::special_border_color::foreground: {
                return foreground;
              } break;
              case theme::special_border_color::background: {
                return background;
              } break;
              default: {
                throw std::runtime_error{"impossible!"};
              } break;
              }
            } break;
            default: {
              throw std::runtime_error{"impossible!"};
            } break;
            }
          }(),
          [&theme]() -> theme::color {
            switch (theme.alternating_tint_color_border.index()) {
            case 0: {
              return std::get<0>(theme.alternating_tint_color_border);
            } break;
            case 1: {
              switch (std::get<1>(theme.alternating_tint_color_border)) {
              case theme::special_border_color::foreground: {
                return theme.alternating_tint_color_foreground;
              } break;
              case theme::special_border_color::background: {
                return theme.alternating_tint_color_background;
              } break;
              default: {
                throw std::runtime_error{"impossible!"};
              } break;
              }
            } break;
            default: {
              throw std::runtime_error{"impossible!"};
            } break;
            }
          }(),
          apply_tint)},
      .border_top{theme.border_width_top},
      .border_right{theme.border_width_right},
      .border_bottom{theme.border_width_bottom},
      .border_left{theme.border_width_left}};
}

struct i3neostatus::i3bar_data::block::data::program::theme
i3neostatus::make_block::impl::separator_theme(
    const theme::theme &theme, const theme::separator_type type,
    const struct i3bar_data::block::data::program::theme *left,
    const struct i3bar_data::block::data::program::theme *right) {
  switch (type) {
  case theme::separator_type::begin: {
    return (struct i3bar_data::block::data::program::theme){
        .color{[&theme, &right]() -> i3bar_data::types::color {
          switch (theme.separator_begin_color_foreground.index()) {
          case 0: {
            return std::get<0>(theme.separator_begin_color_foreground);
          } break;
          case 1: {
            switch (std::get<1>(theme.separator_begin_color_foreground)) {
            case theme::special_separator_color_begin::right: {
              return right->color;
            } break;
            }
          } break;
          default: {
            throw std::runtime_error{"impossible!"};
          } break;
          }
        }()},
        .background{[&theme, &right]() -> i3bar_data::types::color {
          switch (theme.separator_begin_color_background.index()) {
          case 0: {
            return std::get<0>(theme.separator_begin_color_background);
          } break;
          case 1: {
            switch (std::get<1>(theme.separator_begin_color_background)) {
            case theme::special_separator_color_begin::right: {
              return right->background;
            } break;
            }
          } break;
          default: {
            throw std::runtime_error{"impossible!"};
          } break;
          }
        }()},
        .border{},
        .border_top{0},
        .border_right{0},
        .border_bottom{0},
        .border_left{0}};
  } break;
  case theme::separator_type::middle: {
    return (struct i3bar_data::block::data::program::theme){
        .color{[&theme, &left, &right]() -> i3bar_data::types::color {
          switch (theme.separator_middle_color_foreground.index()) {
          case 0: {
            return std::get<0>(theme.separator_middle_color_foreground);
          } break;
          case 1: {
            switch (std::get<1>(theme.separator_middle_color_foreground)) {
            case theme::special_separator_color::right: {
              return right->color;
            } break;
            case theme::special_separator_color::left: {
              return left->color;
            } break;
            }
          } break;
          default: {
            throw std::runtime_error{"impossible!"};
          } break;
          }
        }()},
        .background{[&theme, &left, &right]() -> i3bar_data::types::color {
          switch (theme.separator_middle_color_background.index()) {
          case 0: {
            return std::get<0>(theme.separator_middle_color_background);
          } break;
          case 1: {
            switch (std::get<1>(theme.separator_middle_color_background)) {
            case theme::special_separator_color::right: {
              return right->background;
            } break;
            case theme::special_separator_color::left: {
              return left->background;
            } break;
            }
          } break;
          default: {
            throw std::runtime_error{"impossible!"};
          } break;
          }
        }()},
        .border{},
        .border_top{0},
        .border_right{0},
        .border_bottom{0},
        .border_left{0}};
  } break;
  case theme::separator_type::end: {
    return (struct i3bar_data::block::data::program::theme){
        .color{[&theme, &left]() -> i3bar_data::types::color {
          switch (theme.separator_end_color_foreground.index()) {
          case 0: {
            return std::get<0>(theme.separator_end_color_foreground);
          } break;
          case 1: {
            switch (std::get<1>(theme.separator_end_color_foreground)) {
            case theme::special_separator_color_end::left: {
              return left->color;
            } break;
            }
          } break;
          default: {
            throw std::runtime_error{"impossible!"};
          } break;
          }
        }()},
        .background{[&theme, &left]() -> i3bar_data::types::color {
          switch (theme.separator_end_color_background.index()) {
          case 0: {
            return std::get<0>(theme.separator_end_color_background);
          } break;
          case 1: {
            switch (std::get<1>(theme.separator_end_color_background)) {
            case theme::special_separator_color_end::left: {
              return left->background;
            } break;
            }
          } break;
          default: {
            throw std::runtime_error{"impossible!"};
          } break;
          }
        }()},
        .border{},
        .border_top{0},
        .border_right{0},
        .border_bottom{0},
        .border_left{0}};
  } break;
  }
}
