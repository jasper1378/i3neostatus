#include "pulseaudio_volume.hpp"
#include "../../common/common.hpp"
#include "i3status_pulse.hpp"

#include "globals.hpp"

#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <pthread.h>
#include <stdexcept>
#include <string>
#include <vector>

void sigusr1(int sig)
{
}

void Print(
    const PulseAudioVolume& pavolume,
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
        main_thread = pthread_self();

        std::signal(SIGUSR1, sigusr1);

        sigset_t sig_mask;
        sigfillset(&sig_mask);
        sigdelset(&sig_mask, SIGUSR1);
        sigdelset(&sig_mask, SIGINT);
        sigdelset(&sig_mask, SIGQUIT);
        sigdelset(&sig_mask, SIGTSTP);

        const uint32_t& env_sink_idx{ static_cast<uint32_t>(std::stoul(common::GetEnvWrapperExtraEmpties("sink_idx", std::to_string(DEFAULT_SINK_INDEX), { "_default_" }))) };

        const std::string env_sink_name{ common::GetEnvWrapperExtraEmpties("sink_name", "", { "_default_" }) };

        const std::string env_output_format{ common::GetEnvWrapper("output_format", "(%devicename): %volume") };
        const std::string env_output_format_muted{ common::GetEnvWrapper("output_format_muted", "(%devicename): muted") };

        const std::string env_color_normal{ common::GetEnvWrapper("color_normal", "#FFFFFF") };
        const std::string env_color_muted{ common::GetEnvWrapper("color_muted", "#FFFFFF") };

        std::cout << "<span>Waiting for PulseAudio</span>" << std::endl;

        PulseAudioVolume pavolume{ env_sink_idx, env_sink_name };
        Print(pavolume, env_output_format, env_output_format_muted, env_color_normal, env_color_muted);

        while (true)
        {
            sigsuspend(&sig_mask);
            pavolume.UpdateVolume();
            Print(pavolume, env_output_format, env_output_format_muted, env_color_normal, env_color_muted);
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
