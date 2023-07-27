#include "../../common/network/network.hpp"
#include "wireless.hpp"
#include "../../common/common.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

void Print(
    const Wireless& wireless,
    const std::string& env_output_format,
    const std::string& env_output_format_down,
    const std::string& env_color_regular,
    const double env_threshold_low_quality,
    const std::string& env_color_low_quality,
    const std::string& env_color_down
)
{
    const bool wireless_is_up{ wireless.GetIsUp() };
    const std::string& wireless_ip{ wireless.GetIP() };
    const std::string& wireless_essid{ wireless.GetESSID() };
    const std::string& wireless_frequency{ wireless.GetFormattedFrequency() };
    const std::string& wireless_quality{ wireless.GetFormattedQuality() };

    std::string full_text;
    std::string color;

    if (wireless_is_up)
    {
        full_text = env_output_format;

        if (wireless.GetQuality() <= env_threshold_low_quality)
        {
            color = env_color_low_quality;
        }
        else
        {
            color = env_color_regular;
        }

        std::vector<common::Substitution> subs{
            { "%ip", wireless_ip },
            { "%essid", wireless_essid },
            { "%frequency", wireless_frequency },
            { "%quality", wireless_quality }
        };
        for (size_t i{ 0 }; i < subs.size(); ++i)
        {
            common::ReplaceAll(full_text, subs[i].to_replace, subs[i].replace_with);
        }
    }
    else
    {
        full_text = env_output_format_down;
        color = env_color_down;
    }

    common::PrintPangoMarkup(full_text, color);
}

int main()
{
    try
    {
        const std::string env_interface{ common::GetEnvWrapperExtraEmpties("interface", "", { "_default_" }) };
        const std::string env_output_format{ common::GetEnvWrapper("output_format", "W: (%quality at %essid %frequency) %ip") };
        const std::string env_output_format_down{ common::GetEnvWrapperExtraEmpties("output_format_down", "W: down") };
        const std::string env_color_regular{ common::GetEnvWrapper("color_regular", "#FFFFFF") };
        const double env_threshold_low_quality{ std::stod(common::GetEnvWrapper("threshold_low_quality", std::to_string(50))) };
        const std::string env_color_low_quality{ common::GetEnvWrapper("color_low_quality", "#FFFFFF") };
        const std::string env_color_down{ common::GetEnvWrapper("color_down", "#FFFFFF") };

        Wireless wireless{ env_interface };

        while (true)
        {
            wireless.Update();

            Print(
                wireless,
                env_output_format,
                env_output_format_down,
                env_color_regular,
                env_threshold_low_quality,
                env_color_low_quality,
                env_color_down
            );

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }
    }
    catch (const std::runtime_error& error)
    {
        std::string error_string{ "Error: " + std::string{ error.what() } };
        std::cerr << error_string << '\n';
        common::PrintPangoMarkup(error_string, "#FF0000");
        return 1;
    }
    catch (const std::exception& error)
    {
        std::string error_string{ "Error: " + std::string{ error.what() } };
        std::cerr << error_string << '\n';
        common::PrintPangoMarkup(error_string, "#FF0000");
        return 1;
    }
    catch(...)
    {
        std::string error_string{ "Unknown Error" };
        std::cerr << error_string << '\n';
        common::PrintPangoMarkup(error_string, "#FF0000");
        return 1;
    }
}
