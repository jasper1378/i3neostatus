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
    const PulseaudioVolume& pavolume,
    const std::string& env_output_format,
    const std::string& env_output_format_muted,
    const std::string& env_color_normal,
    const std::string& env_color_muted
)
{
    std::string output_format{ env_output_format };
    std::string output_format_muted{ env_output_format_muted };

    const std::string& pavolume_volume{ std::to_string(pavolume.GetVolume()) + "%" };
    bool pavolume_muted{ pavolume.GetMuted() };
    const std::string& pavolume_devicename{ pavolume.GetDescription() };

    std::vector<common::Substitution> general_output_format_subs{
        { "%volume", pavolume_volume },
        { "%muted", ((pavolume_muted) ? ("True") : ("False")) },
        { "%devicename", pavolume_devicename }
    };
    for (size_t i{ 0 }; i < general_output_format_subs.size(); ++i)
    {
        if (pavolume_muted == true)
        {
            common::ReplaceAll(output_format_muted, general_output_format_subs[i].to_replace, general_output_format_subs[i].replace_with);
        }
        else
        {
            common::ReplaceAll(output_format, general_output_format_subs[i].to_replace, general_output_format_subs[i].replace_with);
        }
    }

    const std::string& full_text{ ((pavolume_muted) ? (output_format_muted) : (output_format)) };
    const std::string& color{ ((pavolume_muted) ? (env_color_muted) : (env_color_normal)) };

    common::PrintPangoMarkup(full_text, color);

    return;
}

int main()
{
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
        Volume::DeviceId device_id{};

        if (env_volume_api == "pulseaudio")
        {
            if (env_device_id == "_default_")
            {
                device_id = PulseaudioVolume::GetDefaultDeviceId();
            }
            else if (is_numeric(env_device_id) == true)
            {
                device_id = { Volume::IdType::num, static_cast<uint32_t>(std::stoul(env_device_id)) };
            }
            else
            {
                device_id = {Volume::IdType::string, env_device_id};
            }
        }
        else if (env_volume_api == "alsa")
        {
            throw std::runtime_error{ "alsa support has not yet been added" };
        }
        else
        {
            throw std::runtime_error{ "invalid volume_api value: \"" + env_volume_api + "\"; options are \"pulseaudio\" or \"alsa\""};
        }

        // const uint32_t& env_sink_idx{ static_cast<uint32_t>(std::stoul(common::GetEnvWrapperExtraEmpties("sink_idx", std::to_string(DEFAULT_SINK_INDEX), { "_default_" }))) };
        // const std::string env_sink_name{ common::GetEnvWrapperExtraEmpties("sink_name", "", { "_default_" }) };

        const std::string env_output_format{ common::GetEnvWrapper("output_format", "(%devicename): %volume") };
        const std::string env_output_format_muted{ common::GetEnvWrapper("output_format_muted", "(%devicename): muted") };

        const std::string env_color_normal{ common::GetEnvWrapper("color_normal", "#FFFFFF") };
        const std::string env_color_muted{ common::GetEnvWrapper("color_muted", "#FFFFFF") };

        PulseaudioVolume pavolume{ device_id };
        Print(pavolume, env_output_format, env_output_format_muted, env_color_normal, env_color_muted);

        while (true)
        {
            pavolume.m_updated.wait(false);
            pavolume.m_updated = false;
            switch (pavolume.GetStatus())
            {
                case Volume::Status::starting:
                    {
                        continue;
                    }
                    break;
                case Volume::Status::running:
                    {
                        Print(pavolume, env_output_format, env_output_format_muted, env_color_normal, env_color_muted);
                    }
                    break;
                case Volume::Status::stopped:
                    {
                        std::string error{ pavolume.GetLastError() };
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
