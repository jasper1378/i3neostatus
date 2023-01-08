#pragma once

#ifndef VOLUME_HPP
#define VOLUME_HPP

#include <atomic>
#include <cstdint>
#include <string>
#include <variant>

class Volume
{
    public:

        enum class IdType
        {
            string,
            num,
        };

        struct DeviceId
        {
            IdType type;
            std::variant<std::string, uint32_t> value;
        };

        enum class Status
        {
            starting,
            running,
            stopped,
        };

    public:

        std::atomic<bool> m_updated;

    protected:

        Volume();

    public:

        virtual ~Volume();

    public:

        virtual uint32_t GetVolume() const =0;
        virtual bool GetMuted() const =0;
        virtual std::string GetDescription() const =0;

        virtual Status GetStatus() const =0;
        virtual std::string GetLastError() const =0;

    private:

        virtual void Init() =0;
        virtual void Term() =0;
};

#endif
