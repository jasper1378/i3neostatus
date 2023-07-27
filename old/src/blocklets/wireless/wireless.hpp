#pragma once

#ifndef WIRELESS_HPP
#define WIRELESS_HPP

#include "../../common/network/network.hpp"

#include <string>

#include <linux/wireless.h>

class Wireless : public Network
{
    protected:

        std::string m_essid;
        double m_frequency;
        double m_quality;

    protected:

        int m_sockfd;

    public:

        Wireless();
        Wireless(const std::string& interface);
        Wireless(const Wireless& other);

        virtual ~Wireless() override;

        Wireless& operator= (const Wireless& other);

        std::string GetESSID() const;
        double GetFrequency() const;
        std::string GetFormattedFrequency() const;
        double GetQuality() const;
        std::string GetFormattedQuality() const;

        virtual void Update() override;

    protected:

        void OpenSocket();

        virtual void Init() override;

        void InitESSID();
        void InitFrequency();
        void InitQuality();
};

#endif
