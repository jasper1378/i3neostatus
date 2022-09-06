#include "battery.hpp"
#include "../../common/common.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

void Print(
    const Battery& battery,
    const std::string& env_output_format,
    const std::string& env_state_full,
    const std::string& env_full_color,
    const std::string& env_state_charging,
    const std::string& env_charging_color,
    const std::string& env_state_discharging,
    const std::string& env_discharging_color,
    const std::string& env_state_unknown,
    const std::string& env_unknown_color,
    const int env_low_threshold,
    const std::string& env_low_threshold_color,
    const int env_critical_threshold,
    const std::string& env_critical_threshold_color
)
{
    std::string full_text{ env_output_format };

    const std::string& battery_name{ battery.GetName() };
    std::string battery_state{ battery.GetState() };
    const std::string& battery_percent{ battery.GetFormattedPercent() };
    const std::string& battery_time{ battery.GetFormattedTime() };

    if (battery_state == "Full")
    {
        battery_state = env_state_full;
    }
    else if (battery_state == "Charging")
    {
        battery_state = env_state_charging;
    }
    else if (battery_state == "Discharging")
    {
        battery_state = env_state_discharging;
    }
    else if (battery_state == "Unknown")
    {
        battery_state = env_state_unknown;
    }

    std::vector<common::Substitution> general_output_format_subs{
        { "%name", battery_name },
        { "%state", battery_state },
        { "%percent", battery_percent },
        { "%time", battery_time },
    };
    for (size_t i{ 0 }; i < general_output_format_subs.size(); ++i)
    {
        common::ReplaceAll(full_text, general_output_format_subs[i].to_replace, general_output_format_subs[i].replace_with);
    }

    std::string color{};

    if (battery_state == env_state_full)
    {
        color = env_full_color;
    }
    else if (battery_state == env_state_charging)
    {
        color = env_charging_color;
    }
    else if (battery_state == env_state_discharging)
    {
        color = env_discharging_color;
    }
    else if (battery_state == env_state_unknown)
    {
        color = env_unknown_color;
    }
    else
    {
        color = env_unknown_color;
    }

    if (battery_state == env_state_discharging)
    {
        double battery_numeric_percent{ battery.GetPercent() };
        if ((battery_numeric_percent <= env_low_threshold) && (battery_numeric_percent > env_critical_threshold))
        {
            color = env_low_threshold_color;
        }
        else if (battery_numeric_percent <= env_critical_threshold)
        {
            color = env_critical_threshold_color;
        }
    }

    common::PrintPangoMarkup(full_text, color);
}

int main()
{
    try
    {
        const std::string env_bat_name{ common::GetEnvWrapper("bat_name", "BAT0") };

        const std::string env_output_format{ common::GetEnvWrapper("output_format", "%state %percent %time") };

        const std::string env_state_full{ common::GetEnvWrapper("state_full", "Full") };
        const std::string env_full_color{ common::GetEnvWrapper("full_color", "#FFFFFF") };
        const std::string env_state_charging{ common::GetEnvWrapper("state_charging", "Charging") };
        const std::string env_charging_color{ common::GetEnvWrapper("charging_color", "#FFFFFF") };
        const std::string env_state_discharging{ common::GetEnvWrapper("state_discharging", "Discharging") };
        const std::string env_discharging_color{ common::GetEnvWrapper("discharging_color", "#FFFFFF") };
        const std::string env_state_unknown{ common::GetEnvWrapper("state_unknown", "Unknown") };
        const std::string env_unknown_color{ common::GetEnvWrapper("unknown_color", "#FFFFFF") };

        const int env_low_threshold{ std::stoi(common::GetEnvWrapper("low_threshold", "0")) };
        const std::string env_low_threshold_color{ common::GetEnvWrapper("low_threshold_color", "#FFFFFF") };
        const int env_critical_threshold{ std::stoi(common::GetEnvWrapper("critical_threshold", "0")) };
        const std::string env_critical_threshold_color{ common::GetEnvWrapper("critical_threshold_color", "#FFFFFF") };

        Battery battery{ env_bat_name };

        while (true)
        {
            battery.UpdateStatus();

            Print(
                battery,
                env_output_format,
                env_state_full,
                env_full_color,
                env_state_charging,
                env_charging_color,
                env_state_discharging,
                env_discharging_color,
                env_state_unknown,
                env_unknown_color,
                env_low_threshold,
                env_low_threshold_color,
                env_critical_threshold,
                env_critical_threshold_color
            );

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }

        return 0;
    }
    catch (const Battery::BatteryDoesNotExistError& error)
    {
        const std::string env_output_format_down{ common::GetEnvWrapper("output_format_down", "No battery") };
        const std::string env_color_down{ common::GetEnvWrapper("color_down", "#FFFFFF") };

        std::string full_text{ env_output_format_down };
        std::string color{ env_color_down };

        common::ReplacePangoSpecialChars(full_text);
        common::ReplacePangoSpecialChars(color);

        std::string pango_output{ "<span color='" + color + "'>" + full_text + "</span>" };

        std::cout << pango_output << std::endl;

        using namespace std::chrono_literals;
        while (true)
        {
            std::this_thread::sleep_for(1000h);
        }

        return 0;
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Error (std::runtime_error): " << error.what() << '\n';
        return 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error (std::exception): " << error.what() << '\n';
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown error" << '\n';
        return 1;
    }
}
