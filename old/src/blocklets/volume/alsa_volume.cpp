#include "alsa_volume.hpp"

#include "volume.hpp"

#include <alsa/asoundlib.h>

#include <atomic>
#include <cmath>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>

const std::string AlsaVolume::m_k_default_card{ "default" };

AlsaVolume::AlsaVolume(const std::string& mixer_name/*= "Master"*/, const long mixer_idx/*= 0*/)
    : m_mixer_name{ mixer_name },
      m_mixer_idx{ mixer_idx },
      m_info_volume{},
      m_info_muted{},
      m_info_description{},
      m_snd_mixer{ nullptr },
      m_snd_sid{ nullptr },
      m_snd_ctl{ nullptr },
      m_snd_async_handler{ nullptr },
      m_status{ Status::starting },
      m_last_error{ "ok" }
{
    Init();
}

AlsaVolume::~AlsaVolume()
{
    Term();
}

long AlsaVolume::GetVolume() const
{
    std::unique_lock<std::mutex> info_lck{ m_info_mx };
    return m_info_volume;
}

bool AlsaVolume::GetMuted() const
{
    std::unique_lock<std::mutex> info_lck{ m_info_mx };
    return m_info_muted;
}

std::string AlsaVolume::GetDescription() const
{
    std::unique_lock<std::mutex> info_lck{ m_info_mx };
    return m_info_description;
}

Volume::Status AlsaVolume::GetStatus() const
{
    return m_status;
}

std::string AlsaVolume::GetLastError() const
{
    std::unique_lock<std::mutex> last_error_lck{ m_last_error_mx };
    return m_last_error;
}

void AlsaVolume::Init()
{
    std::unique_lock<std::mutex> init_lck{ m_init_mx };

    try
    {
        if (int err{ snd_mixer_open(&m_snd_mixer, 0) }; err != 0)
        {
            throw std::runtime_error{ std::string{ "snd_mixer_open(): " } + snd_strerror(err) };
        }

        if (int err{ snd_mixer_attach(m_snd_mixer, m_k_default_card.c_str()) }; err != 0)
        {
            throw std::runtime_error{ std::string{ "snd_mixer_attach(): " } + snd_strerror(err) };
        }

        if (int err{ snd_mixer_selem_register(m_snd_mixer, NULL, NULL) }; err != 0)
        {
            throw std::runtime_error{ std::string{ "snd_mixer_selem_register(): " } + snd_strerror(err) };
        }

        if (int err{ snd_mixer_load(m_snd_mixer) }; err != 0 )
        {
            throw std::runtime_error{ std::string{ "snd_mixer_load(): " } + snd_strerror(err) };
        }

        if (int err{ snd_mixer_selem_id_malloc(&m_snd_sid) }; err != 0)
        {
            throw std::runtime_error{ std::string{ "snd_mixer_selem_id_malloc(): " } + snd_strerror(err) };
        }

        snd_mixer_selem_id_set_index(m_snd_sid, m_mixer_idx);
        snd_mixer_selem_id_set_name(m_snd_sid, m_mixer_name.c_str());

        if (int err{ snd_ctl_open(&m_snd_ctl, m_k_default_card.c_str(), 0) }; err != 0)
        {
            throw std::runtime_error{ std::string{ "snd_ctl_open(): " } + snd_strerror(err) };
        }

        if (int err{ snd_ctl_subscribe_events(m_snd_ctl, 1) }; err != 0)
        {
            throw std::runtime_error{ std::string{ "snd_ctl_subscribe_events(): " } + snd_strerror(err) };
        }

        if (int err{ snd_async_add_ctl_handler(&m_snd_async_handler, m_snd_ctl, CtlCB, this) }; err != 0)
        {
            throw std::runtime_error{ std::string{ "snd_async_add_ctl_handler(): " } + snd_strerror(err) };
        }

        StoreMixerInfo();

        m_status = Status::running;
    }

    catch (const std::exception& e)
    {
        SetLastError(e.what());
        Term();
        throw;
    }
    catch (...)
    {
        SetLastError("unknown error");
        Term();
        throw;
    }
}

void AlsaVolume::Term()
{
    std::unique_lock<std::mutex> term_lck{ m_term_mx };

    m_status = Status::stopped;

    if (m_snd_sid != nullptr)
    {
        snd_mixer_selem_id_free(m_snd_sid);
        m_snd_sid = nullptr;
    }

    if (m_snd_mixer != nullptr)
    {
        snd_mixer_close(m_snd_mixer);
        m_snd_mixer = nullptr;
    }

    if (m_snd_ctl != nullptr)
    {
        snd_ctl_close(m_snd_ctl);
        m_snd_ctl = nullptr;
    }
}

void AlsaVolume::SetLastError(const std::string& error)
{
    std::unique_lock<std::mutex> last_eror_lck{ m_last_error_mx };
    m_last_error = error;
    m_updated = true;
    m_updated.notify_all();
}

void AlsaVolume::StoreMixerInfo()
{
    if (m_status == Status::stopped)
    {
        return;
    }

    snd_mixer_elem_t* elem{ nullptr };
    elem = snd_mixer_find_selem(m_snd_mixer, m_snd_sid);
    if (elem == nullptr)
    {
        SetLastError( "snd_mixer_find_selem()" );
        Term();
        return;
    }

    if (int err{ snd_mixer_handle_events(m_snd_mixer) }; err < 0)
    {
        SetLastError(std::string{ std::string{ "snd_mixer_handle_events(): " } + snd_strerror(err) });
        Term();
        return;
    }

    int err{ 0 };
    long min{};
    long max{};
    long val{};
    bool force_linear{ false };

    err = ((snd_mixer_selem_get_playback_dB_range(elem, &min, &max)) || (snd_mixer_selem_get_playback_dB(elem, SND_MIXER_SCHN_FRONT_LEFT, &val)));
    if ((err != 0) || (min >= max))
    {
        err = ((snd_mixer_selem_get_playback_volume_range(elem, &min, &max)) || (snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &val)));
        force_linear = true;
    }
    if (err != 0)
    {
        SetLastError("unable to get playback volume");
        Term();
        return;
    }

    static constexpr long max_linear_db_scale{ 24 };

    int avg{};

    if ((force_linear == true) || ((max - min) <= (max_linear_db_scale * 100)))
    {
        float avgf{ (static_cast<float>(val - min) / (max - min)) * 100 };
        avg = static_cast<int>(avgf);
        avg = (((avgf - avg) < 0.5) ? (avg) : (avg + 1));
    }
    else
    {
        double normalized{ exp10((val - max) / 6000.0) };
        if (min != SND_CTL_TLV_DB_GAIN_MUTE)
        {
            double min_norm{ exp10((min - max) / 6000.0) };
            normalized = ((normalized - min_norm) / (1 - min_norm));
        }
        avg = std::lround(normalized * 100);
    }

    int pbval{ 1 };
    bool muted{ false };

    if (snd_mixer_selem_has_playback_switch(elem))
    {
        if (int err{ snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &pbval) }; err != 0)
        {
            SetLastError(std::string{ std::string{ "snd_mixer_selem_get_playback_switch()" } + snd_strerror(err) });
            Term();
            return;
        }

        if (!pbval)
        {
            muted = true;
        }
    }

    const char* mixer_name{ nullptr };
    mixer_name = snd_mixer_selem_get_name(elem);
    if (mixer_name == nullptr)
    {
        SetLastError("snd_mixer_selem_get_name()");
        Term();
        return;
    }

    {
        std::unique_lock<std::mutex> info_lck{ m_info_mx };
        m_info_volume = avg;
        m_info_muted = muted;
        m_info_description = mixer_name;
    }

    m_updated = true;
    m_updated.notify_all();
}

void AlsaVolume::CtlCB(snd_async_handler_t* handler)
{
    AlsaVolume* this_ptr{ static_cast<AlsaVolume*>(snd_async_handler_get_callback_private(handler)) };
    if (this_ptr->m_status == Status::stopped)
    {
        return;
    }

    this_ptr->StoreMixerInfo();
}
