#include "wireless.hpp"

#include <exception>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cmath>
#include <cstdlib>
#include <cstring>

#include <linux/wireless.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

Wireless::Wireless()
    : Network{}, m_essid{ "" }, m_frequency{ 0 }, m_quality{ 0 }, m_sockfd{ -1 }
{
    OpenSocket();
    Init();
}

Wireless::Wireless(const std::string& interface)
    : Network{}, m_essid{ "" }, m_frequency{ 0 }, m_quality{ 0 }, m_sockfd{ -1 }
{
    OpenSocket();
    Init();
}

Wireless::Wireless(const Wireless& other)
    : Network{ other }, m_essid{ other.m_essid }, m_frequency{ other.m_frequency }, m_quality{ other.m_quality }
{
    OpenSocket();
}

Wireless::~Wireless()
{
    close(m_sockfd);
}

Wireless& Wireless::operator= (const Wireless& other)
{
    if (this == &other)
    {
        return *this;
    }

    Network::operator=(other);
    m_essid = other.m_quality;
    m_frequency = other.m_frequency;
    m_quality = other.m_quality;

    return *this;
}

std::string Wireless::GetESSID() const
{
    return m_essid;
}

double Wireless::GetFrequency() const
{
    return m_frequency;
}

std::string Wireless::GetFormattedFrequency() const
{
    std::ostringstream oss{};
    oss << std::fixed << std::setprecision(1) << (m_frequency / 1000000000) << " GHz";
    return oss.str();
}

double Wireless::GetQuality() const
{
    return m_quality;
}

std::string Wireless::GetFormattedQuality() const
{
    return std::string{ std::to_string(static_cast<int>(std::round(m_quality))) + "%" };
}

void Wireless::Update()
{
    Init();
}

void Wireless::OpenSocket()
{
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sockfd == -1)
    {
        throw Network::NetworkError{ "cannot open socket" };
    }
}

void Wireless::Init()
{
    bool success{ false };
    do
    {
        try
        {
            if (m_interface == "")
            {
                InitInterface(true);
                if (m_interface == "")
                {
                    m_is_up = false;
                    m_ip = "";
                    m_essid = "";
                    m_frequency = 0;
                    m_quality = 0;

                    success = true;
                    break;
                }
            }

            InitIsUp();
            if (m_is_up == false)
            {
                m_ip = "";
                m_essid = "";
                m_frequency = 0;
                m_quality = 0;

                break;
                return;
            }

            InitIP();

            InitESSID();

            InitFrequency();

            InitQuality();

            success = true;
            break;
        }
        catch (const Network::NetworkError& e)
        {
        }
    }
    while (!success);
}

void Wireless::InitESSID()
{
    iwreq wreq;
    std::memset(&wreq, 0, sizeof(iwreq));
    std::strcpy(wreq.ifr_name, m_interface.c_str());

    char* essid{ static_cast<char*>(std::malloc(IW_ESSID_MAX_SIZE + 1)) };
    wreq.u.essid.pointer = essid;
    wreq.u.essid.length = (IW_ESSID_MAX_SIZE + 1);

    if (ioctl(m_sockfd, SIOCGIWESSID, &wreq) == -1)
    {
        throw Network::NetworkError{ "cannot acquire ESSID information" };
    }

    m_essid = essid;

    std::free(essid);
}

void Wireless::InitFrequency()
{
    iwreq wreq;
    std::memset(&wreq, 0, sizeof(iwreq));
    std::strcpy(wreq.ifr_name, m_interface.c_str());

    if (ioctl(m_sockfd, SIOCGIWFREQ, &wreq) == -1)
    {
        throw Network::NetworkError{ "cannot acquire frequency information" };
    }

    if (wreq.u.freq.e == 0)
    {
        m_frequency = wreq.u.freq.m;
    }
    else
    {
        m_frequency = (wreq.u.freq.m * (std::pow(10, wreq.u.freq.e)));
    }
}

void Wireless::InitQuality()
{
    iwreq wreq_stats;
    std::memset(&wreq_stats, 0, sizeof(iwreq));
    std::strcpy(wreq_stats.ifr_name, m_interface.c_str());

    iw_statistics stats;
    std::memset(&stats, 0, sizeof(iw_statistics));

    wreq_stats.u.data.pointer = &stats;
    wreq_stats.u.data.length = sizeof(iw_statistics);

    if (ioctl(m_sockfd, SIOCGIWSTATS, &wreq_stats) == -1)
    {
        throw Network::NetworkError{ "cannot acquire quality information" };
    }

    unsigned char quality{ stats.qual.qual };

    iwreq wreq_range;
    std::memset(&wreq_range, 0, sizeof(iwreq));
    std::strcpy(wreq_range.ifr_name, m_interface.c_str());

    iw_range range;
    std::memset(&range, 0, sizeof(iw_range));

    wreq_range.u.data.pointer = &range;
    wreq_range.u.data.length = sizeof(iw_range);

    if (ioctl(m_sockfd, SIOCGIWRANGE, &wreq_range) == -1)
    {
        throw Network::NetworkError{ "cannot acquire quality information" };
    }

    unsigned char max_quality{ range.max_qual.qual };

    m_quality = ((static_cast<double>(quality) / static_cast<double>(max_quality)) * 100);
}
