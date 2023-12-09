#ifndef MODULE_ID
#define MODULE_ID

#include <cstddef>
#include <limits>

namespace module_id {
using type = std::size_t;
static constexpr type max = std::numeric_limits<type>::max() - 1;
static constexpr type null = std::numeric_limits<type>::max();
} // namespace module_id

#endif
