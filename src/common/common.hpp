#pragma once

#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>
#include <vector>

namespace common
{
    struct Substitution
    {
        const std::string& to_replace{};
        const std::string& replace_with{};
    };

    std::vector<std::string> ReadWholeFile(const std::string& file_name);
    std::string ReadFirstLineOfFile(const std::string& file_name);
    std::vector<std::string> ExecCmdReadWholeOut(const std::string& cmd);
    std::string ExecCmdReadFirstLine(const std::string& cmd);
    void ExecCmdDontRead(const std::string& cmd);
    void ReplaceAll(std::string& str, const std::string& to_replace, const std::string& replace_with);
    std::string GetEnvWrapper(const std::string& env, const std::string& default_value = "");
    std::string GetEnvWrapperExtraEmpties(const std::string& env, const std::string& default_value = "", const std::vector<std::string>& considered_as_empty = {});
    void ReplacePangoSpecialChars(std::string& str);
    void PrintPangoMarkup( std::string full_text,  std::string color);
    bool CheckIfProgramExistsInPath(const std::string& program);
}

#endif
