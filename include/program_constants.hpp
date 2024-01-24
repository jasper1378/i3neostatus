#ifndef I3NEOSTATUS_PROGRAM_CONSTANTS_HPP
#define I3NEOSTATUS_PROGRAM_CONSTANTS_HPP

#include <filesystem>
#include <string>

namespace i3neostatus {

namespace program_constants {
static constexpr std::string g_k_name{"i3neostatus"};
static constexpr std::string g_k_version{"0"};
static constexpr int g_k_year{2024};
static constexpr std::string g_k_authors{"Jasper Young"};
static const std::filesystem::path g_k_install_path{MAKE_INSTALL_PATH};
}; // namespace program_constants

} // namespace i3neostatus
#endif
