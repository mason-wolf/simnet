#include "reg_commands.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

static bool iequals(const std::string &a, const std::string &b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

static std::string normalize_path(std::string path)
{
    if (path.rfind("HKLM\\", 0) == 0 || path.rfind("hkLM\\", 0) == 0 ||
        path.rfind("Hklm\\", 0) == 0 || path.rfind("hklm\\", 0) == 0)
    {
        path.replace(0, 4, "HKEY_LOCAL_MACHINE");
    }
    return path;
}

void handle_reg_query(const std::string &machine_name, const std::string &query)
{
    std::string path = normalize_path(query);

    auto start = path.find_first_not_of(" \t");
    auto end = path.find_last_not_of(" \t");
    if (start == std::string::npos)
    {
        std::cout << "ERROR: Invalid syntax.\n";
        return;
    }
    path = path.substr(start, end - start + 1);

    std::string reg_dir;
    std::string display_key;

    if (iequals(path, "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"))
    {
        reg_dir = machine_name + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/Run";
        display_key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    }
    else if (iequals(path, "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"))
    {
        reg_dir = machine_name + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/RunOnce";
        display_key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
    }
    else if (iequals(path, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services"))
    {
        reg_dir = machine_name + "/registry/HKLM/SYSTEM/CurrentControlSet/Services";
        display_key = "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services";
    }
    else
    {
        std::cout << "ERROR: The system was unable to find the specified registry key or value.\n";
        return;
    }

    if (!fs::exists(reg_dir) || !fs::is_directory(reg_dir))
    {
        std::cout << "ERROR: The system was unable to find the specified registry key or value.\n";
        return;
    }

    std::cout << "\n";

    for (const auto &entry : fs::directory_iterator(reg_dir))
    {
        if (entry.is_directory())
        {
            std::cout << display_key << "\\" << entry.path().filename().string() << "\n";
        }
        else if (entry.is_regular_file())
        {
            std::string value_name = entry.path().filename().string();
            std::ifstream file(entry.path());

            if (!file.is_open())
                continue;

            std::string type, data;
            std::getline(file, type);
            std::getline(file, data);

            if (!type.empty() && type.back() == '\r')
                type.pop_back();
            if (!data.empty() && data.back() == '\r')
                data.pop_back();

            std::cout << "    " << value_name << "    " << type << "    " << data << "\n";
        }
    }

    std::cout << "\n";
}