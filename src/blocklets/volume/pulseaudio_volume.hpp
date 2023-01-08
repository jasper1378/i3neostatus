#pragma once

#ifndef PULSEAUDIO_VOLUME_HPP
#define PULSEAUDIO_VOLUME_HPP

#include "volume.hpp"

#include <pulse/pulseaudio.h>

#include <condition_variable>
#include <cstdint>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>

class PulseaudioVolume : public Volume
{
    private:

        static const std::string m_k_app_name;
        static const std::string m_k_default_sink_name;

    private:

        DeviceId m_device_id;

        mutable std::mutex m_info_mx;
        uint32_t m_info_volume;
        bool m_info_muted;
        std::string m_info_description;

        pa_threaded_mainloop* m_pa_mainloop;
        bool m_mainloop_running;
        pa_mainloop_api* m_pa_api;
        pa_context* m_pa_context;
        bool m_context_connected;
        pa_proplist* m_pa_proplist;
        pa_operation* m_pa_subscribe_op;

        std::atomic<Status> m_status;
        std::string m_last_error;
        mutable std::mutex m_last_error_mx;
        std::mutex m_init_mx;
        std::mutex m_term_mx;
        std::atomic<pa_context_state_t> m_context_state;
        std::atomic<bool> m_context_ready;
        std::atomic<bool> m_subscribe_ready;
        std::atomic<bool> m_sink_info_ready;

    public:

        PulseaudioVolume();
        PulseaudioVolume(DeviceId device_id);

        PulseaudioVolume(const PulseaudioVolume& other) =delete;
        PulseaudioVolume(PulseaudioVolume&& other) =delete;

        virtual ~PulseaudioVolume() override;

    public:

        uint32_t GetVolume() const override;
        bool GetMuted() const override;
        std::string GetDescription() const override;

        Status GetStatus() const override;
        std::string GetLastError() const override;

    public:

        static DeviceId GetDefaultDeviceId();

    public:

        PulseaudioVolume& operator=(const PulseaudioVolume& other) =delete;
        PulseaudioVolume& operator=(PulseaudioVolume&& other) =delete;

    private:

        void Init() override;
        void Term() override;

        void SetLastError(const std::string& error);

        void GetSinkInfo();
        void StoreSinkInfo(const pa_sink_info* i);

    private:

        static void ContextStateCB(pa_context* c, void* userdata);
        static void SubscribeCB(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void* userdata);
        static void SubscribeSuccessCB(pa_context* c, int success, void* userdata);
        static void SinkInfoCB(pa_context* c, const pa_sink_info* i, int eol, void* userdata);
};

#endif
