#include "network.hpp"

#include "../../common/common.hpp"

#include <exception>
#include <filesystem>
#include <stdexcept>
#include <string>

#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

#define _GNU_SOURCE

Network::Network()
    : m_interface{ "" }, m_is_up{ false }, m_ip{ "" }
{
}

Network::Network(const std::string& interface)
    : m_interface{ interface }, m_is_up{ false }, m_ip{ "" }
{
}

Network::Network(const Network& other)
    : m_interface{ other.m_interface }, m_is_up{ other.m_is_up }, m_ip{ other.m_ip }
{
}

Network::~Network()
{
}

Network& Network::operator= (const Network& other)
{
    if (this == &other)
    {
        return *this;
    }

    m_interface = other.m_interface;
    m_is_up = other.m_is_up;
    m_ip = other.m_ip;

    return *this;
}

std::string Network::GetInterface() const
{
    return m_interface;
}

bool Network::GetIsUp() const
{
    return m_is_up;
}

std::string Network::GetIP() const
{
    return m_ip;
}

void Network::InitInterface(bool is_wireless)
{
    namespace sfs = std::filesystem;

    ifaddrs* ifaddr;
    int family;
    int s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        freeifaddrs(ifaddr);
        throw NetworkError{ "cannot acquire interface information" };
        return;
    }

    for (ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET)
        {
            if (!(strcmp(ifa->ifa_name, "lo") == 0))
            {
                if ((sfs::is_directory(std::string{ "/sys/class/net/" + std::string{ ifa->ifa_name } + "/wireless" })) == (is_wireless))
                {
                    m_interface = ifa->ifa_name;
                    freeifaddrs(ifaddr);
                    return;
                }
            }
        }
    }

    m_interface = "";
    return;
}

void Network::InitIsUp()
{
    namespace sfs = std::filesystem;

    std::string sys_path{ "/sys/class/net/" + m_interface };

    if ((sfs::is_directory(sys_path) == false) && (sfs::is_symlink(sys_path) == false))
    {
        throw NetworkError{ "cannot acquire state information" };
        return;
    }
    else
    {
        std::string operstate{ common::ReadFirstLineOfFile(std::string{ sys_path + "/operstate" }) };
        if (operstate == "down")
        {
            m_is_up = false;
            return;
        }
        else if (operstate == "up")
        {
            m_is_up = true;
            return;
        }
        else
        {
            throw NetworkError{ "cannot acquire state information" };
            return;
        }
    }
}

void Network::InitIP()
{
    ifaddrs* ifaddr;
    int family;
    int s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        freeifaddrs(ifaddr);
        throw NetworkError{ "cannot acquire IP information" };
        return;
    }

    for (ifaddrs* ifa{ ifaddr }; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if ((family == AF_INET) && (strcmp(ifa->ifa_name, m_interface.c_str()) == 0))
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if (s != 0)
            {
                freeifaddrs(ifaddr);
                throw NetworkError{ "cannot acquire IP information" };
                return;
            }

            m_ip = host;
            return;
        }
    }

    freeifaddrs(ifaddr);
    throw NetworkError{ "cannot acquire IP information" };
    return;
}

Network::NetworkError::NetworkError(const std::string& what_arg)
    : std::runtime_error{ what_arg }
{
}

Network::NetworkError::NetworkError(const char* what_arg)
    : std::runtime_error{ what_arg }
{
}

Network::NetworkError::NetworkError(const NetworkError& other) noexcept
    : std::runtime_error{ other }
{
}

Network::NetworkError::~NetworkError()
{
}

Network::NetworkError& Network::NetworkError::operator= (const NetworkError& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    std::runtime_error::operator=(other);

    return *this;
}

const char* Network::NetworkError::what() const noexcept
{
    return std::runtime_error::what();
}
