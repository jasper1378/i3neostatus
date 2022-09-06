#include "missing_dependency_error.hpp"

common::MissingDependencyError::MissingDependencyError(const std::string& missing_dependency)
    : m_missing_dependency{ missing_dependency }, std::runtime_error{ std::string{ "missing dependency: " + missing_dependency } }
{
}

common::MissingDependencyError::MissingDependencyError(const char* missing_dependency)
    : m_missing_dependency{ missing_dependency }, std::runtime_error{ std::string{ "missing dependency: " + std::string{ missing_dependency } } }
{
}

common::MissingDependencyError::MissingDependencyError(const MissingDependencyError& other) noexcept
    : m_missing_dependency{ other.m_missing_dependency }, std::runtime_error{ other }
{
}

common::MissingDependencyError::~MissingDependencyError()
{
}

common::MissingDependencyError& common::MissingDependencyError::operator= (const MissingDependencyError& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    m_missing_dependency = other.m_missing_dependency;
    std::runtime_error::operator=(other);

    return *this;
}

const char* common::MissingDependencyError::what() const noexcept
{
    return common::MissingDependencyError::GetMissingDependency();
}

const char* common::MissingDependencyError::GetMissingDependency() const noexcept
{
    return m_missing_dependency.c_str();
}
