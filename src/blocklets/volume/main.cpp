#include "alsa_volume.hpp"
#include "../../common/common.hpp"
#include "pulseaudio_volume.hpp"
#include "volume.hpp"

#include <atomic>
#include <cctype>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void Print(
    const Volume* volume,
    const std::string& env_output_format,
    const std::string& env_output_format_muted,
    const std::string& env_color_normal,
    const std::string& env_color_muted
)
{
    std::string output_format{ env_output_format };
    std::string output_format_muted{ env_output_format_muted };

    const std::string& volume_volume{ std::to_string(volume->GetVolume()) + "%" };
    bool volume_muted{ volume->GetMuted() };
    const std::string& volume_devicename{ volume->GetDescription() };

    std::vector<common::Substitution> general_output_format_subs{
        { "%volume", volume_volume },
        { "%muted", ((volume_muted) ? ("True") : ("False")) },
        { "%devicename", volume_devicename }
    };
    for (size_t i{ 0 }; i < general_output_format_subs.size(); ++i)
    {
        if (volume_muted == true)
        {
            common::ReplaceAll(output_format_muted, general_output_format_subs[i].to_replace, general_output_format_subs[i].replace_with);
        }
        else
        {
            common::ReplaceAll(output_format, general_output_format_subs[i].to_replace, general_output_format_subs[i].replace_with);
        }
    }

    const std::string& full_text{ ((volume_muted) ? (output_format_muted) : (output_format)) };
    const std::string& color{ ((volume_muted) ? (env_color_muted) : (env_color_normal)) };

    common::PrintPangoMarkup(full_text, color);

    return;
}

int main()
{
    Volume* volume{ nullptr };

    try
    {
        static constexpr auto is_numeric{ [](const std::string& str)
        {
            for (std::string::size_type i{ 0 }; i < str.size(); ++i)
            {
                if (std::isdigit(str[i]) == false)
                {
                    return false;
                }
            }
            return true;
        } };

        const std::string env_volume_api{ common::GetEnvWrapper("volume_api", "pulseaudio") };

        const std::string env_device_id{ common::GetEnvWrapper("device_id", "_default_") };
        PulseaudioVolume::DeviceId device_id{};

        if (env_volume_api == "pulseaudio")
        {
            if (env_device_id == "_default_")
            {
                volume = new PulseaudioVolume{};
            }
            else if (is_numeric(env_device_id) == true)
            {
                volume = new PulseaudioVolume{ PulseaudioVolume::DeviceId{ PulseaudioVolume::IdType::num, std::stol(env_device_id) } };
            }
            else
            {
                volume = new PulseaudioVolume{ PulseaudioVolume::DeviceId{ PulseaudioVolume::IdType::string, env_device_id } };
            }
        }
        else if (env_volume_api == "alsa")
        {
            if (env_device_id == "_default_")
            {
                volume = new AlsaVolume{};
            }
            else
            {
                std::string::size_type comma_pos{ env_device_id.find(",") };

                if (comma_pos == std::string::npos)
                {
                    volume = new AlsaVolume{ env_device_id };
                }
                else
                {
                    std::string name{ env_device_id.substr(0, comma_pos) };
                    std::string idx{ env_device_id.substr(comma_pos + 1) };

                    if (is_numeric(idx) == false)
                    {
                        throw std::runtime_error{ "invalid `device_id` for `alsa`: \"" + env_device_id + "\"; try `mixer_name` or `mixer_name,mixer_index`" };
                    }
                    else
                    {
                        volume = new AlsaVolume{ name, std::stol(idx) };
                    }
                }
            }
        }
        else
        {
            throw std::runtime_error{ "invalid volume_api value: \"" + env_volume_api + "\"; options are \"pulseaudio\" or \"alsa\""};
        }

        const std::string env_output_format{ common::GetEnvWrapper("output_format", "(%devicename): %volume") };
        const std::string env_output_format_muted{ common::GetEnvWrapper("output_format_muted", "(%devicename): muted") };

        const std::string env_color_normal{ common::GetEnvWrapper("color_normal", "#FFFFFF") };
        const std::string env_color_muted{ common::GetEnvWrapper("color_muted", "#FFFFFF") };

        Print(volume, env_output_format, env_output_format_muted, env_color_normal, env_color_muted);

        while (true)
        {
            volume->m_updated.wait(false);
            volume->m_updated = false;
            switch (volume->GetStatus())
            {
                case Volume::Status::starting:
                    {
                        continue;
                    }
                    break;
                case Volume::Status::running:
                    {
                        Print(volume, env_output_format, env_output_format_muted, env_color_normal, env_color_muted);
                    }
                    break;
                case Volume::Status::stopped:
                    {
                        std::string error{ volume->GetLastError() };
                        if  (error != "")
                        {
                            throw std::runtime_error{ error };
                        }
                    }
                    break;
            }
        }

        return 0;
    }
    catch (const std::runtime_error& error)
    {
        if (volume != nullptr)
        {
            delete volume;
        }

        std::string error_string{ "Error: " + std::string{ error.what() } };
        std::cerr << error_string << '\n';
        common::PrintPangoMarkup(error_string, "#FF0000");
        return 1;
    }
    catch (const std::exception& error)
    {
        if (volume != nullptr)
        {
            delete volume;
        }

        std::string error_string{ "Error: " + std::string{ error.what() } };
        std::cerr << error_string << '\n';
        common::PrintPangoMarkup(error_string, "#FF0000");
        return 1;
    }
    catch (...)
    {
        if (volume != nullptr)
        {
            delete volume;
        }

        std::string error_string{ "Unknown Error" };
        std::cerr << error_string << '\n';
        common::PrintPangoMarkup(error_string, "#FF0000");
        return 1;
    }
}
