#ifndef I3NEOSTATUS_PROGRAM_CONSTANTS_HPP
#define I3NEOSTATUS_PROGRAM_CONSTANTS_HPP

#include <filesystem>
#include <string>

namespace i3neostatus {

namespace program_constants {
static constexpr std::string k_name{"i3neostatus"};
static constexpr std::string k_version{"0"};
static constexpr int k_year{2024};
static constexpr std::string k_authors{"Jasper Young"};
static const std::filesystem::path k_install_path{MAKE_INSTALL_PATH};
}; // namespace program_constants

} // namespace i3neostatus
#endif
