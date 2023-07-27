#pragma once

#ifndef MISSING_DEPENDENCY_ERROR_HPP
#define MISSING_DEPENDENCY_ERROR_HPP

#include <exception>
#include <stdexcept>
#include <string>

namespace common
{
    class MissingDependencyError : public std::runtime_error
    {
        private:

            std::string m_missing_dependency;

        public:

            MissingDependencyError(const std::string& missing_dependency);
            MissingDependencyError(const char* missing_dependency);
            MissingDependencyError(const MissingDependencyError& other) noexcept;

            virtual ~MissingDependencyError();

            MissingDependencyError& operator= (const MissingDependencyError& other) noexcept;

            virtual const char* what() const noexcept override;

            virtual const char* GetMissingDependency() const noexcept;

    };
}

#endif
