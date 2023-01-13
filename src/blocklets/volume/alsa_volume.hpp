#pragma once

#ifndef ALSA_VOLUME_HPP
#define ALSA_VOLUME_CPP

#include "volume.hpp"

#include <alsa/asoundlib.h>

#include <atomic>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>

class AlsaVolume : public Volume
{
    private:

        static const std::string m_k_default_card;

    private:

        std::string m_mixer_name;
        long m_mixer_idx;

        mutable std::mutex m_info_mx;
        long m_info_volume;
        bool m_info_muted;
        std::string m_info_description;

        snd_mixer_t* m_snd_mixer;
        snd_mixer_selem_id_t* m_snd_sid;
        snd_ctl_t* m_snd_ctl;
        snd_async_handler_t* m_snd_async_handler;

        std::atomic<Status> m_status;
        std::string m_last_error;
        mutable std::mutex m_last_error_mx;
        std::mutex m_init_mx;
        std::mutex m_term_mx;

    public:

        AlsaVolume(const std::string& mixer_name = "Master", const long mixer_idx = 0);

        AlsaVolume(const AlsaVolume& other) =delete;
        AlsaVolume(AlsaVolume&& other) =delete;

        virtual ~AlsaVolume() override;

    public:

        long GetVolume() const override;
        bool GetMuted() const override;
        std::string GetDescription() const override;

        Status GetStatus() const override;
        std::string GetLastError() const override;

    public:

        AlsaVolume& operator=(const AlsaVolume& other) =delete;
        AlsaVolume& operator=(AlsaVolume&& other) =delete;

    private:

        void Init();
        void Term();

        void SetLastError(const std::string& error);

        void StoreMixerInfo();

    private:

        static void CtlCB(snd_async_handler_t* handler);
};

#endif
