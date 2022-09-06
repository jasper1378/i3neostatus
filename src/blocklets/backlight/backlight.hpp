#pragma once

#ifndef BACKLIGHT_HPP
#define BACKLIGHT_HPP

#include <string>

class Backlight
{
    private:

        double m_brightness;

    public:

        Backlight();
        Backlight(const Backlight& other);

        Backlight& operator= (const Backlight& other);

        double GetBrightness() const;
        std::string GetFormattedBrightness() const;
        void SetBrightness(double brightness);
        void Update();
};

#endif
