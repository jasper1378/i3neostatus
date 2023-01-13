#pragma once

#ifndef VOLUME_HPP
#define VOLUME_HPP

#include <atomic>
#include <string>
#include <variant>

class Volume
{
    public:

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

        virtual long GetVolume() const =0;
        virtual bool GetMuted() const =0;
        virtual std::string GetDescription() const =0;

        virtual Status GetStatus() const =0;
        virtual std::string GetLastError() const =0;
};

#endif
