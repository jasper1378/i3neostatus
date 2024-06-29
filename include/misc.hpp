#ifndef I3NEOSTATUS_MISC_HPP
#define I3NEOSTATUS_MISC_HPP

#include <string>
#include <string_view>

namespace i3neostatus {

namespace misc {
namespace resolve_tilde {
bool would_resolve_tilde(const std::string_view str);
std::string resolve_tilde(const std::string &str);
} // namespace resolve_tilde

} // namespace misc

} // namespace i3neostatus
#endif
