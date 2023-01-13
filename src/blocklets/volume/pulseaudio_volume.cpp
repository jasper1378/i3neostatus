#include "pulseaudio_volume.hpp"

#include "volume.hpp"

#include <pulse/pulseaudio.h>

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>
#include <variant>

const std::string PulseaudioVolume::m_k_app_name{ "jasper-i3blocks-blocklets" };
const std::string PulseaudioVolume::m_k_default_sink_name{ "@DEFAULT_SINK@" };

PulseaudioVolume::PulseaudioVolume(const DeviceId& device_id/*= { IdType::string, m_k_default_sink_name }*/)
    : Volume{},
      m_device_id{ device_id },
      m_info_volume{},
      m_info_muted{},
      m_info_description{},
      m_pa_mainloop{ nullptr},
      m_mainloop_running{ false },
      m_pa_api{ nullptr },
      m_pa_context{ nullptr },
      m_context_connected{ false },
      m_pa_proplist{ nullptr },
      m_pa_subscribe_op{ nullptr },
      m_status{ Status::starting },
      m_last_error{ "ok" },
      m_context_state{ PA_CONTEXT_UNCONNECTED },
      m_context_ready{ false },
      m_subscribe_ready{ false },
      m_sink_info_ready{ false }
{
    Init();
}

PulseaudioVolume::~PulseaudioVolume()
{
    Term();
}

long PulseaudioVolume::GetVolume() const
{
    std::unique_lock<std::mutex> info_lck{ m_info_mx };
    return m_info_volume;
}

bool PulseaudioVolume::GetMuted() const
{
    std::unique_lock<std::mutex> info_lck{ m_info_mx };
    return m_info_muted;
}

std::string PulseaudioVolume::GetDescription() const
{
    std::unique_lock<std::mutex> info_lck{ m_info_mx };
    return m_info_description;
}

Volume::Status PulseaudioVolume::GetStatus() const
{
    return m_status;
}

std::string PulseaudioVolume::GetLastError() const
{
    std::unique_lock<std::mutex> last_error_lck{ m_last_error_mx };
    return m_last_error;
}

void PulseaudioVolume::Init()
{
    std::unique_lock<std::mutex> init_lck{ m_init_mx };

    try
    {
        if (m_pa_mainloop == nullptr)
        {
            m_pa_mainloop = pa_threaded_mainloop_new();
            if (m_pa_mainloop == nullptr)
            {
                throw std::runtime_error{ "pa_threaded_mainloop_new()" };
            };
        }

        if (m_pa_api == nullptr)
        {
            m_pa_api = pa_threaded_mainloop_get_api(m_pa_mainloop);
            if (m_pa_api == nullptr)
            {
                throw std::runtime_error{ "pa_threaded_mainloop_get_api()" };
            }
        }

        if (m_pa_proplist == nullptr)
        {
            m_pa_proplist = pa_proplist_new();
            if (m_pa_proplist == nullptr)
            {
                throw std::runtime_error{ "pa_proplist_new()" };
            }

            int pa_proplist_sets_result{ pa_proplist_sets(m_pa_proplist, PA_PROP_APPLICATION_NAME, m_k_app_name.c_str()) };
            if (pa_proplist_sets_result < 0)
            {
                throw std::runtime_error{ std::string{ "pa_proplist_sets(): " } + pa_strerror(pa_proplist_sets_result) };
            }

        }

        if (m_pa_context == nullptr)
        {
            m_pa_context = pa_context_new_with_proplist(m_pa_api, m_k_app_name.c_str(), m_pa_proplist);
            if (m_pa_context == nullptr)
            {
                throw std::runtime_error{ "pa_context_new_with_proplist()" };
            }

            pa_context_set_state_callback(m_pa_context, ContextStateCB, this);
        }

        if (m_context_connected == false)
        {
            static constexpr pa_context_flags_t context_flags{ static_cast<pa_context_flags_t>(PA_CONTEXT_NOFAIL | PA_CONTEXT_NOAUTOSPAWN) };
            if (pa_context_connect(m_pa_context, NULL, context_flags, NULL) < 0)
            {
                throw std::runtime_error{ std::string{ "pa_context_connect(): " } + pa_strerror(pa_context_errno(m_pa_context)) };
            }
            m_context_connected = true;
        }

        if (m_mainloop_running == false)
        {
            if (pa_threaded_mainloop_start(m_pa_mainloop) < 0)
            {
                throw std::runtime_error{ std::string{ "pa_threaded_mainloop_start(): " } + pa_strerror(pa_context_errno(m_pa_context))};
            }
            m_mainloop_running = true;
        }

        m_context_ready.wait(false);

        if (m_pa_subscribe_op == nullptr)
        {
            pa_context_set_subscribe_callback(m_pa_context, SubscribeCB, this);

            static constexpr pa_subscription_mask_t sub_mask{ static_cast<pa_subscription_mask_t>(PA_SUBSCRIPTION_EVENT_SINK | PA_SUBSCRIPTION_EVENT_SERVER) };
            m_pa_subscribe_op = pa_context_subscribe(m_pa_context, sub_mask, SubscribeSuccessCB, this);

            while (pa_operation_get_state(m_pa_subscribe_op) != PA_OPERATION_DONE)
            {
                m_subscribe_ready.wait(false);
            }
        }

        m_status = Status::running;

        GetSinkInfo();
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

void PulseaudioVolume::Term()
{
    std::unique_lock<std::mutex> term_lck{ m_term_mx };

    m_status = Status::stopped;

    m_updated = true;
    m_updated.notify_all();

    if (m_context_connected == true)
    {
        pa_context_disconnect(m_pa_context);
        m_context_connected = false;
    }

    if (m_pa_context != nullptr)
    {
        pa_context_unref(m_pa_context);
        m_pa_context = nullptr;
    }

    if (m_mainloop_running == true)
    {
        pa_threaded_mainloop_stop(m_pa_mainloop);
        m_mainloop_running = false;
    }

    if (m_pa_mainloop != nullptr)
    {
        pa_threaded_mainloop_free(m_pa_mainloop);
        m_pa_mainloop = nullptr;
    }

    if (m_pa_proplist != nullptr)
    {
        pa_proplist_free(m_pa_proplist);
        m_pa_proplist = nullptr;
    }

    if (m_pa_subscribe_op != nullptr)
    {
        pa_operation_unref(m_pa_subscribe_op);
        m_pa_subscribe_op = nullptr;
    }
}

void PulseaudioVolume::SetLastError(const std::string& error)
{
    std::unique_lock<std::mutex> last_eror_lck{ m_last_error_mx };
    m_last_error = error;
    m_updated = true;
    m_updated.notify_all();
}

void PulseaudioVolume::GetSinkInfo()
{
    if (m_status == Status::stopped)
    {
        return;
    }

    pa_operation* get_sink_info_oper{ nullptr };

    switch (static_cast<int>(m_device_id.type))
    {
        case static_cast<int>(IdType::string):
            {
                get_sink_info_oper = pa_context_get_sink_info_by_name(
                    m_pa_context,
                    std::get<std::string>(m_device_id.value).c_str(),
                    SinkInfoCB,
                    this);
            }
            break;

        case static_cast<int>(IdType::num):
            {
                get_sink_info_oper = pa_context_get_sink_info_by_index(
                    m_pa_context,
                    static_cast<uint32_t>(std::get<long>(m_device_id.value)),
                    SinkInfoCB,
                    this);
            }
            break;
    }

    if (get_sink_info_oper != nullptr)
    {
        pa_operation_unref(get_sink_info_oper);
    }
}

void PulseaudioVolume::StoreSinkInfo(const pa_sink_info* i)
{
    if (m_status == Status::stopped)
    {
        return;
    }

    static constexpr auto convert_volume_to_percent{ [](const pa_cvolume& raw_volume) -> long
    {
        return (std::round(static_cast<double>(pa_cvolume_avg(&raw_volume) * 100) / PA_VOLUME_NORM));
    } };

    std::unique_lock<std::mutex> info_lck{ m_info_mx };

    m_info_muted = i->mute;
    m_info_description = i->active_port->description;
    m_info_volume = convert_volume_to_percent(i->volume);

    m_updated = true;
    m_updated.notify_all();
}

void PulseaudioVolume::ContextStateCB(pa_context* c, void* userdata)
{
    PulseaudioVolume* this_ptr{ static_cast<PulseaudioVolume*>(userdata) };

    if (this_ptr->m_status == Status::stopped)
    {
        return;
    }

    this_ptr->m_context_state = pa_context_get_state(c);

    switch (this_ptr->m_context_state)
    {
        case PA_CONTEXT_TERMINATED:
            {
                this_ptr->m_context_ready = false;
                this_ptr->SetLastError("PA_CONTEXT_TERMINATED");
                this_ptr->Term();
            }
            break;

        case PA_CONTEXT_FAILED:
            {
                this_ptr->m_context_ready = false;
                this_ptr->SetLastError("PA_CONTEXT_FAILED");
                this_ptr->Term();
            }
            break;

        case PA_CONTEXT_READY:
            {
                this_ptr->m_context_ready = true;
                this_ptr->m_context_ready.notify_all();
            }
            break;

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            {
                this_ptr->m_context_ready = false;
            }
            break;
    }
}

void PulseaudioVolume::SubscribeCB(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void* userdata)
{
    PulseaudioVolume* this_ptr{ static_cast<PulseaudioVolume*>(userdata) };

    if (this_ptr->m_status == Status::stopped)
    {
        return;
    }

    if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK)  == PA_SUBSCRIPTION_EVENT_CHANGE)
    {
        pa_subscription_event_type_t facility{ static_cast<pa_subscription_event_type_t>(t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) };

        switch (facility)
        {
            case PA_SUBSCRIPTION_EVENT_SERVER:
            case PA_SUBSCRIPTION_EVENT_SINK:
                {
                    this_ptr->GetSinkInfo();
                }
                break;
            default:
                break;
        }
    }
}

void PulseaudioVolume::SubscribeSuccessCB(pa_context* c, int success, void* userdata)
{
    PulseaudioVolume* this_ptr{ static_cast<PulseaudioVolume*>(userdata) };

    if (this_ptr->m_status == Status::stopped)
    {
        return;
    }

    this_ptr->m_subscribe_ready = true;
    this_ptr->m_subscribe_ready.notify_all();
}

void PulseaudioVolume::SinkInfoCB(pa_context* c, const pa_sink_info* i, int eol, void* userdata)
{
    PulseaudioVolume* this_ptr{ static_cast<PulseaudioVolume*>(userdata) };

    if (this_ptr->m_status == Status::stopped)
    {
        return;
    }

    if (eol < 0)
    {
        this_ptr->SetLastError(std::string{ "SinkInfoCB(): " } + pa_strerror(pa_context_errno(c)));
        this_ptr->Term();
        return;
    }
    else if (eol > 0)
    {
    }
    else
    {
        this_ptr->StoreSinkInfo(i);
    }
}
