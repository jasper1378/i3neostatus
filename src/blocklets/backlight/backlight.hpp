#pragma once

#ifndef BACKLIGHT_HPP
#define BACKLIGHT_HPP

#include <exception>
#include <stdexcept>
#include <string>

class Backlight
{
    private:

        static const std::string m_s_c_backlight_sys_path;

        std::string m_device_name;
        std::string m_device_path;
        double m_brightness;

    public:

        Backlight();
        Backlight(const std::string& device_name);
        Backlight(const Backlight& other);

        Backlight& operator= (const Backlight& other);

        double GetBrightness() const;
        std::string GetFormattedBrightness() const;
        void SetBrightness(double brightness);
        void Update();

    private:

        void CheckThatDeviceExists();
        void ReadAndCalcBrightness();

    public:

        class NoBacklightDeviceError : public std::runtime_error
        {
            private:

                static const std::string m_s_c_what_arg;

            public:

                NoBacklightDeviceError();
                NoBacklightDeviceError(const NoBacklightDeviceError& other) noexcept;

                virtual ~NoBacklightDeviceError();

            public:

                NoBacklightDeviceError& operator=(const NoBacklightDeviceError& other) noexcept;

            public:

                virtual const char* what() const noexcept override;
        };
};

#endif
