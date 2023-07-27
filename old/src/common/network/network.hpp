#pragma once

#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <exception>
#include <stdexcept>
#include <string>

class Network
{
    protected:

        std::string m_interface;
        bool m_is_up;
        std::string m_ip;

    public:

        Network();
        Network(const std::string& interface);
        Network(const Network& other);

        virtual ~Network();

        Network& operator= (const Network& other);

        std::string GetInterface() const;
        bool GetIsUp() const;
        std::string GetIP() const;

        virtual void Update() = 0;

    protected:

        virtual void Init() = 0;

        void InitInterface(bool is_wireless);
        void InitIsUp();
        void InitIP();

    public:

        class NetworkError : public std::runtime_error
        {
            public:

                NetworkError(const std::string& what_arg);
                NetworkError(const char* what_arg);
                NetworkError(const NetworkError& other) noexcept;

                virtual ~NetworkError();

                NetworkError& operator= (const NetworkError& other) noexcept;

                virtual const char* what() const noexcept;
        };
};

#endif
