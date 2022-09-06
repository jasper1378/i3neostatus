#include "ethernet.hpp"

#include <string>

Ethernet::Ethernet()
    : Network{}
{
    Init();
}

Ethernet::Ethernet(const std::string& interface)
    : Network{ interface }
{
    Init();
}

Ethernet::Ethernet(const Ethernet& other)
    : Network{ other }
{
}

Ethernet::~Ethernet()
{
}

Ethernet& Ethernet::operator= (const Ethernet& other)
{
    if (this == &other)
    {
        return *this;
    }

    Network::operator=(other);

    return *this;
}

void Ethernet::Update()
{
    Init();
}

void Ethernet::Init()
{
    bool success{ false };
    do
    {
        try
        {
            if (m_interface == "")
            {
                InitInterface(false);
                if (m_interface == "")
                {
                    m_is_up = false;
                    m_ip = "";

                    success = true;
                    break;
                }
            }

            InitIsUp();
            if (m_is_up == false)
            {
                m_ip = "";

                success = true;
                break;
            }

            InitIP();

            success = true;
            break;
        }
        catch (const Network::NetworkError& e)
        {
        }
    }
    while (!success);
}
