#pragma once

#ifndef ETHERNET_HPP
#define ETHERNET_HPP

#include "../../common/network/network.hpp"

#include <string>

class Ethernet : public Network
{
    protected:

    public:

        Ethernet();
        Ethernet(const std::string& interface);
        Ethernet(const Ethernet& other);

        virtual ~Ethernet() override;

        Ethernet& operator= (const Ethernet& other);

        virtual void Update() override;

    protected:

        virtual void Init() override;
};

#endif
