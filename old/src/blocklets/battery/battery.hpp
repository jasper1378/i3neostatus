#pragma once

#ifndef BATTERY_HPP
#define BATTERY_HPP

#include <exception>
#include <stdexcept>
#include <string>

class Battery
{
    private:

        enum class AttrsForTimeCalc
        {
            charge_and_current,
            energy_and_power,
            max,
        };

    private:

        std::string m_name;
        std::string m_state;
        double m_percent;
        double m_time;

    public:

        Battery(const std::string& name = "BAT0");
        Battery(const Battery& b);

        ~Battery();

        Battery& operator= (const Battery& b);

        std::string GetName() const;
        std::string GetState() const;
        double GetPercent() const;
        std::string GetFormattedPercent() const;
        double GetTime() const;
        std::string GetFormattedTime() const;

        void UpdateStatus();

    private:

        AttrsForTimeCalc FindAvailableAttrs(const std::string& bat_path);

    public:

        class BatteryDoesNotExistError : public std::runtime_error
        {
            public:

                BatteryDoesNotExistError(const std::string& bat_name);
                BatteryDoesNotExistError(const char* bat_name);
                BatteryDoesNotExistError(const BatteryDoesNotExistError& other) noexcept;

                virtual ~BatteryDoesNotExistError();

                BatteryDoesNotExistError& operator= (const BatteryDoesNotExistError& other) noexcept;

                virtual const char* what() const noexcept;

                virtual const char* GetBatName() const noexcept;

            private:

                std::string m_bat_name;
        };
};

#endif
