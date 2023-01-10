#include "backlight.hpp"

#include "../../common/common.hpp"

#include <cmath>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sstream>

const std::string Backlight::m_s_c_backlight_sys_path{ "/sys/class/backlight/" };

Backlight::Backlight()
    : m_brightness{ -1 }
{
    namespace sfs = std::filesystem;

    if (sfs::is_empty(m_s_c_backlight_sys_path) == true)
    {
        throw NoBacklightDeviceError{};
    }

    sfs::directory_iterator di{ m_s_c_backlight_sys_path };
    m_device_name = (di->path().filename().string());
    m_device_path = std::string{ (di->path().string()) + "/" };

    ReadAndCalcBrightness();
}

Backlight::Backlight(const std::string& device_name)
    : m_brightness{ -1 }
{
    m_device_name = device_name;
    m_device_path = std::string{ m_s_c_backlight_sys_path + device_name + "/"};

    ReadAndCalcBrightness();
}

Backlight::Backlight(const Backlight& other)
    : m_device_name{ other.m_device_name },
      m_device_path{ other.m_device_path },
      m_brightness{ other.m_brightness }
{
    CheckThatDeviceExists();
}

Backlight& Backlight::operator= (const Backlight& other)
{
    if (this == &other)
    {
        return *this;
    }

    m_device_name = other.m_device_name;
    m_device_path = other.m_device_path;
    m_brightness = other.m_brightness;

    CheckThatDeviceExists();

    return *this;
}

double Backlight::GetBrightness() const
{
    return m_brightness;
}

std::string Backlight::GetFormattedBrightness(DecimalFormat decimal_format) const
{
    /*
    if (common::ApproximatelyEqualAbsRel(m_brightness, static_cast<int>(m_brightness)))
    {
        return std::string{ std::to_string(static_cast<int>(std::round(m_brightness))) + "%" };
    }
    else
    {
        std::stringstream sstream;
        sstream.setf(std::ios::fixed);
        sstream.precision(2);
        sstream << m_brightness;

        return std::string{ sstream.str() + "%" };
    }
    */

    while (true)
    {
        switch (decimal_format)
        {
            case DecimalFormat::never:
                {
                    return std::string{ std::to_string(static_cast<int>(std::round(m_brightness))) + "%" };
                }
                break;

            case DecimalFormat::always:
                {
                    std::ostringstream out;
                    out.precision(2);
                    out << std::fixed << m_brightness;
                    return std::string{ out.str() + "%"};
                }
                break;

            case DecimalFormat::automatic:
                {
                    if (m_brightness == std::floor(m_brightness))
                    {
                        decimal_format = DecimalFormat::never;
                    }
                    else
                    {
                        decimal_format = DecimalFormat::always;
                    }
                }
                break;
        }
    }
}

void Backlight::SetBrightness(double brightness)
{
    double max_brightness{ std::stod(common::ReadFirstLineOfFile(std::string{ m_device_path + "/max_brightness" })) };
    double actual_brightness{ (brightness / 100) * max_brightness };

    common::WriteOneLineToFile(std::string{ m_device_path + "/brightness" }, std::to_string(static_cast<int>(std::round(actual_brightness))));
}

void Backlight::Update()
{
    ReadAndCalcBrightness();
}

void Backlight::CheckThatDeviceExists()
{
   namespace sfs = std::filesystem;

    if ((sfs::is_directory(m_device_path) == false) && (sfs::is_symlink(m_device_path) == false))
    {
        throw NoBacklightDeviceError{};
    }
}

void Backlight::ReadAndCalcBrightness()
{
    CheckThatDeviceExists();

    double actual_brightness{ std::stod(common::ReadFirstLineOfFile(std::string{ m_device_path + "/actual_brightness"})) };
    double max_brightness{ std::stod(common::ReadFirstLineOfFile(std::string{ m_device_path + "/max_brightness" })) };

    m_brightness = ((actual_brightness / max_brightness) * 100);
}

const std::string Backlight::NoBacklightDeviceError::m_s_c_what_arg{ "No backlight device found" };

Backlight::NoBacklightDeviceError::NoBacklightDeviceError()
    : std::runtime_error{ m_s_c_what_arg }
{
}

Backlight::NoBacklightDeviceError::NoBacklightDeviceError(const NoBacklightDeviceError& other) noexcept
:   std::runtime_error{ other }
{
}

Backlight::NoBacklightDeviceError::~NoBacklightDeviceError()
{
}

Backlight::NoBacklightDeviceError& Backlight::NoBacklightDeviceError::operator=(const NoBacklightDeviceError& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    std::runtime_error::operator=(other);
    return *this;
}

const char* Backlight::NoBacklightDeviceError::what() const noexcept
{
    return std::runtime_error::what();
}
