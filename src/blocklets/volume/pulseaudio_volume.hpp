#pragma once

#ifndef PULSEAUDIO_VOLUME_HPP
#define PULSEAUDIO_VOLUME_HPP

#include <string>

class PulseAudioVolume
{
    private:

        uint32_t m_sink_idx;
        std::string m_sink_name;
        int m_volume;
        bool m_muted;
        std::string m_description;

    public:

        PulseAudioVolume();
        PulseAudioVolume(const uint32_t sink_idx);
        PulseAudioVolume(const std::string& sink_name);
        PulseAudioVolume(const PulseAudioVolume& other);
        PulseAudioVolume(const uint32_t sink_idx, const std::string& sink_name);

    private:

        void Init();

    public:

        ~PulseAudioVolume();

        PulseAudioVolume& operator= (const PulseAudioVolume& other);


        int GetVolume() const;
        bool GetMuted() const;
        std::string GetDescription() const;

        void UpdateVolume();
};

#endif
