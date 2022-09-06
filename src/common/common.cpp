#include "common.hpp"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::string> common::ReadWholeFile(const std::string& file_name)
{
    std::vector<std::string> file_contents;

    std::ifstream input_file{ file_name };

    if (!input_file)
    {
        throw std::runtime_error{ "unable to open " + file_name + " for reading" };
    }

    while (input_file)
    {
        file_contents.push_back(std::string{});
        std::getline(input_file, file_contents.back());
    }

    return file_contents;
}

std::string common::ReadFirstLineOfFile(const std::string& file_name)
{
    std::string file_contents;

    std::ifstream input_file{ file_name };

    if (!input_file)
    {
        throw std::runtime_error{ "unable to open " + file_name + " for reading" };
    }

    std::getline(input_file, file_contents);

    return file_contents;
}

std::vector<std::string> common::ExecCmdReadWholeOut(const std::string& cmd)
{
    std::FILE* fp;
    char path[128];

    std::vector<std::string> cmd_output;

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
    {
        throw std::runtime_error{ "cannot open pipe" };
    }

    while (std::fgets(path, sizeof(path), fp) != NULL)
    {
        cmd_output.push_back(std::string{ path });
        cmd_output.back().pop_back();
    }

    if (pclose(fp) == -1)
    {
        throw std::runtime_error{ "cannot close pipe" };
    }

    return cmd_output;
}

std::string common::ExecCmdReadFirstLine(const std::string& cmd)
{
    std::FILE* fp;
    char path[128];

    std::string cmd_output;

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
    {
        throw std::runtime_error{ "cannot open pipe" };
    }

    std::fgets(path, sizeof(path), fp);

    cmd_output = path;

    if (pclose(fp) == -1)
    {
        throw std::runtime_error{ "cannot close pipe" };
    }

    return cmd_output;
}

void common::ExecCmdDontRead(const std::string& cmd)
{
    std::FILE* fp;

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
    {
        throw std::runtime_error{ "cannot open pipe" };
    }

    if (pclose(fp) == -1)
    {
        throw std::runtime_error{ "cannot close pipe" };
    }
}

void common::ReplaceAll(std::string& str, const std::string& to_replace, const std::string& replace_with)
{
    std::string buf;
    std::size_t pos{ 0 };
    std::size_t prevPos;

    buf.reserve(str.size());

    while (true)
    {
        prevPos = pos;
        pos = str.find(to_replace, pos);
        if (pos == std::string::npos)
        {
            break;
        }
        buf.append(str.substr(prevPos, pos - prevPos));
        buf.append(replace_with);
        pos += to_replace.size();
    }

    buf.append(str.substr(prevPos));
    str = buf;
}

std::string common::GetEnvWrapper(const std::string& env, const std::string& default_value)
{
    const char* result{ std::getenv(env.c_str()) };

    if (result == nullptr)
    {
        return default_value;
    }
    else
    {
        return result;
    }
}

std::string common::GetEnvWrapperExtraEmpties(const std::string& env, const std::string& default_value, const std::vector<std::string>& considered_as_empty)
{
    const char* result{ std::getenv(env.c_str()) };

    if (result == nullptr)
    {
        return default_value;
    }

    for (size_t i{ 0 }; i < considered_as_empty.size(); ++i)
    {
        if (result == considered_as_empty[i])
        {
            return default_value;
        }
    }

    return result;
}

void common::ReplacePangoSpecialChars(std::string& str)
{
    std::vector<common::Substitution> pango_special_chars{
        { "&", "&amp;" },
        { "<", "&lt;" },
        { ">", "&gt;"}
    };
    for (size_t i{ 0 }; i < pango_special_chars.size(); ++i)
    {
        common::ReplaceAll(str, pango_special_chars[i].to_replace, pango_special_chars[i].replace_with);
    }
}

void common::PrintPangoMarkup(std::string full_text, std::string color)
{
    common::ReplacePangoSpecialChars(full_text);
    common::ReplacePangoSpecialChars(color);

    std::string pango_output{ "<span color='" + color + "'>" + full_text + "</span>" };

    std::cout << pango_output << std::endl;
}

bool common::CheckIfProgramExistsInPath(const std::string& program)
{
    namespace sfs = std::filesystem;

    const std::string env_path{ common::GetEnvWrapper("PATH", "") };

    std::vector<std::string> dirs_in_path;
    std::string cur_dir;

    for (size_t i{ 0 }; i < env_path.size(); ++i)
    {
        if (env_path[i] == ':')
        {
            dirs_in_path.push_back(cur_dir);
            cur_dir.clear();
        }
        else if ((i + 1) == (env_path.size()))
        {
            cur_dir.push_back(env_path[i]);
            dirs_in_path.push_back(cur_dir);
        }
        else
        {
            cur_dir.push_back(env_path[i]);
        }
    }

    for (size_t i{ 0 }; i < dirs_in_path.size(); ++i)
    {
        sfs::directory_iterator di{ dirs_in_path[i] };
        for (auto p{ sfs::begin(di) }; p != sfs::end(di); ++p)
        {
            if ((p->path().filename().string()) == (program))
            {
                return true;
            }
        }
    }

    return false;
}
