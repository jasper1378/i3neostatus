#include "../../common/network/network.hpp"
#include "ethernet.hpp"
#include "../../common/common.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

void Print(
    const Ethernet& ethernet,
    const std::string& env_output_format,
    const std::string& env_output_format_down,
    const std::string& env_color_regular,
    const std::string& env_color_down
)
{
    const bool ethernet_is_up{ ethernet.GetIsUp() };
    const std::string& ethernet_ip{ ethernet.GetIP() };

    std::string full_text;
    std::string color;

    if (ethernet_is_up)
    {
        full_text = env_output_format;
        color = env_color_regular;

        std::vector<common::Substitution> subs{
            { "%ip", ethernet_ip }
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
        const std::string env_output_format{ common::GetEnvWrapper("output_format", "E: %ip") };
        const std::string env_output_format_down{ common::GetEnvWrapper("output_format_down", "E: down") };
        const std::string env_color_regular{ common::GetEnvWrapper("color_regular", "#FFFFFF") };
        const std::string env_color_down{ common::GetEnvWrapper("color_down", "#FFFFFF") };

        Ethernet ethernet{ env_interface };

        while (true)
        {
            ethernet.Update();

            Print(
                ethernet,
                env_output_format,
                env_output_format_down,
                env_color_regular,
                env_color_down
            );

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }
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
    catch(...)
    {
        std::cerr << "Unknown error" << '\n';
        return 1;
    }
}
