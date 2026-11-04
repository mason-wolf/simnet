#include "process_commands.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

static std::vector<std::string> split_csv(const std::string &line)
{
    std::vector<std::string> values;
    std::stringstream ss(line);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        values.push_back(item);
    }

    return values;
}

static std::string trim(const std::string &value)
{
    size_t start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";

    size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

static bool is_pid_only(const std::string &value)
{
    if (value.empty())
        return false;

    for (char ch : value)
    {
        if (ch < '0' || ch > '9')
            return false;
    }

    return true;
}

void handle_tasklist(const std::string &machine_name)
{
    std::ifstream file(machine_name + "/C/Windows/System32/tasklist.txt");
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve process information.\n";
        return;
    }

    std::cout << "\nImage Name                     PID Session Name        Session#    Mem Usage\n";
    std::cout << "========================= ===== ================ ========== ============\n";

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        if (line.back() == '\r')
            line.pop_back();

        std::vector<std::string> fields = split_csv(line);
        if (fields.size() < 5)
        {
            std::cout << line << "\n";
            continue;
        }

        std::string image_name = fields[0];
        std::string pid = fields[1];
        std::string session_name = fields[2];
        std::string session_id = fields[3];
        std::string mem_usage = fields[4];

        if (fields.size() > 5)
        {
            mem_usage += "," + fields[5];
        }

        std::cout << std::left
                  << std::setw(28) << image_name
                  << std::setw(6) << pid
                  << std::setw(18) << session_name
                  << std::setw(11) << session_id
                  << mem_usage << "\n";
    }

    std::cout << "\n";
    file.close();
}

void handle_taskkill(const std::string &machine_name, const std::string &input)
{
    std::string trimmed = trim(input);
    if (trimmed.empty())
    {
        std::cout << "ERROR: Invalid syntax. Type \"taskkill /?\" for usage.\n";
        return;
    }

    std::string command = trimmed;
    std::string target;
    std::string options;

    if (command.rfind("taskkill ", 0) == 0)
    {
        command = command.substr(9);
    }

    std::string path = machine_name + "/C/Windows/System32/tasklist.txt";
    if (!fs::exists(path))
    {
        std::cout << "ERROR: The process " << target << " could not be terminated.\n";
        return;
    }

    std::vector<std::string> args;
    std::stringstream ss(command);
    std::string part;
    while (std::getline(ss, part, ' '))
    {
        if (!part.empty())
            args.push_back(part);
    }

    if (args.empty())
    {
        std::cout << "ERROR: Invalid syntax. Type \"taskkill /?\" for usage.\n";
        return;
    }

    bool found = false;
    bool has_pid = false;
    bool is_force = false;
    bool is_imagename = false;
    bool is_filter = false;

    for (size_t i = 0; i < args.size(); ++i)
    {
        std::string arg = args[i];
        if (arg == "/PID" || arg == "/pid")
        {
            if (i + 1 < args.size())
            {
                target = args[++i];
                has_pid = true;
            }
            else
            {
                std::cout << "ERROR: The parameter is incorrect.\n";
                return;
            }
            continue;
        }

        if (arg == "/IM" || arg == "/IMAGENAME" || arg == "/im" || arg == "/imagename")
        {
            if (i + 1 < args.size())
            {
                target = args[++i];
                is_imagename = true;
            }
            else
            {
                std::cout << "ERROR: The parameter is incorrect.\n";
                return;
            }
            continue;
        }

        if (arg == "/F" || arg == "/f")
        {
            is_force = true;
            continue;
        }

        if (arg == "/FI" || arg == "/fi")
        {
            is_filter = true;
            continue;
        }

        if (arg == "/T" || arg == "/t")
        {
            continue;
        }

        if (arg == "/?")
        {
            std::cout << "ERROR: Invalid syntax. Type \"taskkill /?\" for usage.\n";
            return;
        }

        if (arg == "-f" || arg == "-F")
        {
            is_force = true;
            continue;
        }

        if (arg == "-pid" || arg == "-PID")
        {
            if (i + 1 < args.size())
            {
                target = args[++i];
                has_pid = true;
            }
            else
            {
                std::cout << "ERROR: The parameter is incorrect.\n";
                return;
            }
            continue;
        }

        if (arg == "-im" || arg == "-IM")
        {
            if (i + 1 < args.size())
            {
                target = args[++i];
                is_imagename = true;
            }
            else
            {
                std::cout << "ERROR: The parameter is incorrect.\n";
                return;
            }
            continue;
        }

        if (arg == "-t" || arg == "-T")
        {
            continue;
        }

        if (target.empty())
        {
            target = arg;
            is_imagename = true;
        }
    }

    if (target.empty())
    {
        std::cout << "ERROR: The process " << target << " could not be terminated.\n";
        return;
    }

    if (is_filter && !has_pid && !is_imagename)
    {
        std::cout << "ERROR: The parameter is incorrect.\n";
        return;
    }

    std::ifstream in_file(path);
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(in_file, line))
    {
        if (line.empty())
            continue;

        if (line.back() == '\r')
            line.pop_back();

        std::vector<std::string> fields = split_csv(line);
        if (fields.size() < 5)
        {
            lines.push_back(line);
            continue;
        }

        std::string image_name = trim(fields[0]);
        std::string pid = trim(fields[1]);

        bool matches = false;
        if (has_pid)
        {
            matches = (pid == target);
        }
        else if (is_imagename)
        {
            matches = (image_name == target || image_name == target + ".exe");
        }

        if (matches)
        {
            found = true;
            continue;
        }

        lines.push_back(line);
    }

    in_file.close();

    if (!found)
    {
        std::cout << "ERROR: The process \"" << target << "\" not found.\n";
        return;
    }

    std::ofstream out_file(path, std::ios::trunc);
    if (!out_file.is_open())
    {
        std::cout << "ERROR: Access is denied.\n";
        return;
    }

    for (const std::string &out_line : lines)
    {
        out_file << out_line << "\n";
    }
    out_file.close();

    std::cout << "SUCCESS: The process \"" << target << "\" was terminated successfully.\n";
    if (!is_force)
    {
        std::cout << "NOTE: Use /F to force termination if the process does not exit cleanly.\n";
    }
}
