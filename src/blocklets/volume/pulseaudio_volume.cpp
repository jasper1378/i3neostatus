#include "pulseaudio_volume.hpp"

#include "i3status_pulse.hpp"

#include <string>

#include <cstring>

PulseAudioVolume::PulseAudioVolume()
    : m_sink_idx{ DEFAULT_SINK_INDEX }, m_sink_name{ "" }
{
    Init();
}

PulseAudioVolume::PulseAudioVolume(const uint32_t sink_idx)
    : m_sink_idx{ sink_idx }, m_sink_name{ "" }
{
    Init();
}

PulseAudioVolume::PulseAudioVolume(const std::string& sink_name)
    : m_sink_idx{ DEFAULT_SINK_INDEX }, m_sink_name{ sink_name }
{
    Init();
}

PulseAudioVolume::PulseAudioVolume(const uint32_t sink_idx, const std::string& sink_name)
    : m_sink_idx{ sink_idx }, m_sink_name{ sink_name }
{
    Init();
}

PulseAudioVolume::PulseAudioVolume(const PulseAudioVolume& other)
    : m_sink_idx{ other.m_sink_idx },
      m_sink_name{ other.m_sink_name },
      m_volume{ other.m_volume },
      m_muted{ other.m_muted },
      m_description{ other.m_description }
{
}

void PulseAudioVolume::Init()
{
    const char* sink_name_as_c_style_str{ ((m_sink_name != "") ? (m_sink_name.c_str()) : (NULL)) };

    int cvolume{ PULSEAUDIO_VOLUME_FAIL };
    char description[MAX_SINK_DESCRIPTION_LEN]{ PULSEAUDIO_DESCRIPTION_FAIL };

    do
    {
        if (pulse_initialize())
        {
            cvolume = volume_pulseaudio(m_sink_idx, sink_name_as_c_style_str);

            if (!description_pulseaudio(m_sink_idx, sink_name_as_c_style_str, description))
            {
                std::strcpy(description, PULSEAUDIO_DESCRIPTION_FAIL);
            }
        }
        else
        {
            cvolume = PULSEAUDIO_VOLUME_FAIL;
        }
    }
    while ((cvolume == PULSEAUDIO_VOLUME_FAIL) || (std::strcmp(description, PULSEAUDIO_DESCRIPTION_FAIL) == 0));

    m_volume = DECOMPOSE_VOLUME(cvolume);
    m_muted = DECOMPOSE_MUTED(cvolume);
    m_description = description;
}

PulseAudioVolume::~PulseAudioVolume()
{
}

PulseAudioVolume& PulseAudioVolume::operator= (const PulseAudioVolume& other)
{
    if (this == &other)
    {
        return *this;
    }

    m_sink_idx = other.m_sink_idx;
    m_sink_name = other.m_sink_name;
    m_volume = other.m_volume;
    m_muted = other.m_muted;
    m_description = other.m_description;

    return *this;
}

int PulseAudioVolume::GetVolume() const
{
    return m_volume;
}

bool PulseAudioVolume::GetMuted() const
{
    return m_muted;
}

std::string PulseAudioVolume::GetDescription() const
{
    return m_description;
}

void PulseAudioVolume::UpdateVolume()
{
    Init();
}
