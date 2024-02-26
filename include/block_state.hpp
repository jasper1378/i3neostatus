#ifndef I3NEOSTATUS_BLOCK_STATE_HPP
#define I3NEOSTATUS_BLOCK_STATE_HPP

namespace i3neostatus {
enum class block_state {
  idle = 0,
  info = 1,
  good = 2,
  warning = 3,
  critical = 4,
  error = 5,
};
}

#endif
