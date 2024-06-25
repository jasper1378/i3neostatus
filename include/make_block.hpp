#ifndef I3NEOSTATUS_MAKE_BLOCK_HPP
#define I3NEOSTATUS_MAKE_BLOCK_HPP

#include "block_state.hpp"
#include "i3bar_data.hpp"
#include "theme.hpp"

namespace i3neostatus {
namespace make_block {
struct i3bar_data::block::data::program content(const theme::theme &theme,
                                                const block_state state,
                                                const bool apply_tint,
                                                bool custom_separators);

struct i3bar_data::block
separator(const theme::theme &theme,
          const struct i3bar_data::block::data::program::theme *left,
          const struct i3bar_data::block::data::program::theme *right);

namespace impl {
constexpr struct i3bar_data::block::data::program::global
global(bool custom_separators) {
  return ((custom_separators)
              ? ((struct i3bar_data::block::data::program::global){false, 0})
              : ((struct i3bar_data::block::data::program::global){true}));
}

struct i3bar_data::block::data::program::theme
content_theme(const theme::theme &theme, const block_state state,
              const bool apply_tint);

struct i3bar_data::block::data::program::theme
separator_theme(const theme::theme &theme, const theme::separator_type type,
                const struct i3bar_data::block::data::program::theme *left,
                const struct i3bar_data::block::data::program::theme *right);

inline theme::color maybe_apply_tint(const theme::color &original,
                                     const theme::color &tint,
                                     const bool apply_tint) {
  return ((apply_tint) ? (original + tint) : (original));
}
} // namespace impl
} // namespace make_block
} // namespace i3neostatus

#endif
