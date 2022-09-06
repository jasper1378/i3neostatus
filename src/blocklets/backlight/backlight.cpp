#include "backlight.hpp"

#include "../../common/common.hpp"
#include "../../common/missing_dependency_error/missing_dependency_error.hpp"

#include <cmath>
#include <string>
#include <sstream>

Backlight::Backlight()
    : m_brightness{ -1 }
{
    if (common::CheckIfProgramExistsInPath("light") == false)
    {
        throw common::MissingDependencyError{ "light" };
    }

    m_brightness = std::stod(common::ExecCmdReadFirstLine("light -G"));
}

Backlight::Backlight(const Backlight& other)
    : m_brightness{ other.m_brightness }
{
    if (common::CheckIfProgramExistsInPath("light") == false)
    {
        throw common::MissingDependencyError{ "light" };
    }
}

Backlight& Backlight::operator= (const Backlight& other)
{
    if (this == &other)
    {
        return *this;
    }

    m_brightness = other.m_brightness;

    return *this;
}

double Backlight::GetBrightness() const
{
    return m_brightness;
}

std::string Backlight::GetFormattedBrightness() const
{
    if (m_brightness == static_cast<int>(m_brightness))
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
}

void Backlight::SetBrightness(double brightness)
{
    common::ExecCmdDontRead(std::string{ "light -S " + std::to_string(brightness) });
}

void Backlight::Update()
{
    m_brightness = std::stod(common::ExecCmdReadFirstLine("light -G"));
}
