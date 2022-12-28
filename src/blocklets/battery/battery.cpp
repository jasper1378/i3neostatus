#include <iostream>

#include "battery.hpp"

#include "../../common/common.hpp"

#include <cmath>
#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

Battery::Battery(const std::string& name)
    : m_name{ name }, m_state{}, m_percent{}, m_time{}
{
    std::string path{ "/sys/class/power_supply/" + m_name + "/" };

    if (std::filesystem::is_directory(path) == false)
    {
        throw Battery::BatteryDoesNotExistError{ m_name };
    }

    m_state = common::ReadFirstLineOfFile(std::string{ path + "status" });

    m_percent = static_cast<int>(std::round(std::stod(common::ReadFirstLineOfFile(std::string{ path + "capacity" }))));

    AttrsForTimeCalc attrs_to_use{ FindAvailableAttrs(path) };

    if (attrs_to_use == AttrsForTimeCalc::charge_and_current)
    {
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

            std::string current_now_string;
            do
            {
                current_now_string = common::ReadFirstLineOfFile(std::string{ path + "current_now" });
            }
            while(current_now_string == "");
            double current_now{ std::stod(current_now_string) };

            m_time = (charge_now / current_now);
        }
        else
        {
            m_time = 0;
        }
    }
    else if (attrs_to_use == AttrsForTimeCalc::energy_and_power)
    {
        if (m_state == "Full")
        {
            m_time = 0;
        }
        else if (m_state == "Charging")
        {
            double energy_full{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "energy_full" })) };
            double energy_now{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "energy_now" })) };
            double power_now{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "power_now" })) };

            m_time = ((energy_full - energy_now) / power_now);
        }
        else if (m_state == "Discharging")
        {
            double energy_now{ std::stod(common::ReadFirstLineOfFile(std::string{ path + "energy_now" })) };

            std::string power_now_string;
            do
            {
                power_now_string = common::ReadFirstLineOfFile(std::string{ path + "power_now" });
            }
            while(power_now_string == "");
            double power_now{ std::stod(power_now_string) };

            m_time = (energy_now / power_now);
        }
        else
        {
            m_time = 0;
        }
    }
    else
    {
        throw std::runtime_error{ "unable to find all attributes required for time calculation" };
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

Battery::AttrsForTimeCalc Battery::FindAvailableAttrs(const std::string& bat_path)
{
    static const auto is_reg_file_or_symlink{ [](const std::string& path)
    {
         return ((std::filesystem::is_regular_file(path) == true) || (std::filesystem::is_symlink(path) == true));
    } };

    static const auto check_if_all_files_exist{ [](const std::vector<std::string>& paths)
    {
        for (size_t i{ 0 }; i < paths.size(); ++i)
        {
            if (is_reg_file_or_symlink(paths[i]) == false)
            {
                return false;
            }
        }

        return true;
    } };

    static const std::vector<std::string> charge_and_current_attrs{
        { std::string{ bat_path + "charge_full" } },
        { std::string{ bat_path + "charge_full_design" } },
        { std::string{ bat_path + "charge_now" } },
        { std::string{ bat_path + "current_now" } }
    };

    static const std::vector<std::string> energy_and_power_attrs{
        { std::string{ bat_path + "energy_full" } },
        { std::string{ bat_path + "energy_full_design" } },
        { std::string{ bat_path + "energy_now" } },
        { std::string{ bat_path + "power_now" } }
    };

    if (check_if_all_files_exist(charge_and_current_attrs) == true)
    {
        return AttrsForTimeCalc::charge_and_current;
    }
    else if (check_if_all_files_exist(energy_and_power_attrs) == true)
    {
        return AttrsForTimeCalc::energy_and_power;
    }
    else
    {
        return AttrsForTimeCalc::max;
    }
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
