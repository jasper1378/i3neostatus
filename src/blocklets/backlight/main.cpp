#include "backlight.hpp"
#include "../../common/common.hpp"
#include "../../common/missing_dependency_error/missing_dependency_error.hpp"

#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

void sigusr1(int sig)
{
}

void Print(
    const Backlight& backlight,
    const std::string& env_output_format,
    const std::string& env_color
)
{
    std::string output_format{ env_output_format };

    const std::string& backlight_brightness{ backlight.GetFormattedBrightness() };

    common::ReplaceAll(output_format, "%brightness", backlight_brightness);

    const std::string& full_text{ output_format };
    const std::string& color{ env_color };

    common::PrintPangoMarkup(full_text, color);
}

int main()
{
    try
    {
        std::signal(SIGUSR1, sigusr1);

        std::string env_output_format{ common::GetEnvWrapper("output_format", "%brightness") };

        const std::string env_color{ common::GetEnvWrapper("color", "#FFFFFF") };

        Backlight backlight{};

        while (true)
        {
            backlight.Update();
            Print(backlight, env_output_format, env_color);

            sleep(1);
        }

        return 0;
    }
    catch (const common::MissingDependencyError& error)
    {
        const std::string env_output_format_down{ common::GetEnvWrapper("output_format_down", "No backlight") };
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
    catch (...)
    {
        std::string error_string{ "Unknown Error" };
        std::cerr << error_string << '\n';
        common::PrintPangoMarkup(error_string, "#FF0000");
        return 1;
    }
}
