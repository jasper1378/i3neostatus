#include <iostream>

#include "battery.hpp"

#include "../../common/common.hpp"

#include <cmath>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>

Battery::Battery(const std::string& name)
    : m_name{ name }, m_state{}, m_percent{}, m_time{}
{
    std::string path{ "/sys/class/power_supply/" + m_name + "/" };

    if (std::filesystem::is_directory(path) == false)
    {
        throw Battery::BatteryDoesNotExistError{ m_name };
    }

    m_state = common::ReadFirstLineOfFile(std::string{ path + "status" });

    m_percent = std::stoi(common::ReadFirstLineOfFile(std::string{ path + "capacity" }));

    if (m_state == "Full")
    {
        m_time = 0;
    }
    else if (m_state == "Charging")
    {
        double charge_full{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "charge_full" })) };
        double charge_now{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "charge_now" })) };
        double current_now{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "current_now" })) };

        m_time = ((charge_full - charge_now) / current_now);
    }
    else if (m_state == "Discharging")
    {
        double charge_now{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "charge_now" })) };
        double current_now{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "current_now" })) };

        m_time = (charge_now / current_now);
    }
    else
    {
        m_time = 0;
    }
}

Battery::Battery(const Battery& b)
    : m_name{ b.m_name }, m_state{ b.m_state }, m_percent{ b.m_percent }, m_time{ b.m_time }
{
}

Battery::~Battery()
{
}

Battery& Battery::operator= (const Battery& b)
{
    if (this == &b)
    {
        return *this;
    }

    m_name = b.m_name;
    m_state = b.m_state;
    m_percent = b.m_percent;
    m_time = b.m_time;

    return *this;
}

std::string Battery::GetName() const
{
    return m_name;
}

std::string Battery::GetState() const
{
    return m_state;
}

double Battery::GetPercent() const
{
    return m_percent;
}

std::string Battery::GetFormattedPercent() const
{
    return std::string{ std::to_string(static_cast<int>(std::round(m_percent))) + "%" };
}

double Battery::GetTime() const
{
    return m_time;
}

std::string Battery::GetFormattedTime() const
{
    if (m_time <= (double{ 1 } / double{ 60 }))
    {
        return "";
    }

    int hour{ static_cast<int>(m_time) };

    int min{ static_cast<int>(std::round((m_time - hour) * 60)) };

    std::string formatted_time{ std::to_string(hour) + ":" + ((min < 10) ? ("0") : ("")) + std::to_string(min) };

    return formatted_time;
}

void Battery::UpdateStatus()
{
    *this = Battery{ m_name };
}

Battery::BatteryDoesNotExistError::BatteryDoesNotExistError(const std::string& bat_name)
    : m_bat_name{ bat_name }, std::runtime_error{ "battery does not exist" }
{
}

Battery::BatteryDoesNotExistError::BatteryDoesNotExistError(const char* bat_name)
    : m_bat_name{ bat_name }, std::runtime_error{ "battery does not exist" }
{
}

Battery::BatteryDoesNotExistError::BatteryDoesNotExistError(const BatteryDoesNotExistError& other) noexcept
    : m_bat_name{ other.m_bat_name }, std::runtime_error{ other }
{
}

Battery::BatteryDoesNotExistError::~BatteryDoesNotExistError()
{
}

Battery::BatteryDoesNotExistError& Battery::BatteryDoesNotExistError::operator= (const BatteryDoesNotExistError& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    m_bat_name = other.m_bat_name;
    std::runtime_error::operator=(other);

    return *this;
}

const char* Battery::BatteryDoesNotExistError::what() const noexcept
{
    return GetBatName();
}

const char* Battery::BatteryDoesNotExistError::GetBatName() const noexcept
{
    return m_bat_name.c_str();
}
