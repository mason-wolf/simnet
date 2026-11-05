#include "fs_commands.h"
#include <chrono>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string machine_root(const std::string &machine_name)
{
    return "devices/" + machine_name;
}

static std::string normalize_windows_path(const std::string &value)
{
    std::string out = value;
    for (char &ch : out)
    {
        if (ch == '\\') ch = '/';
    }
    return out;
}

static std::string resolve_windows_path(const std::string &machine_name, const std::string &current_dir, const std::string &path)
{
    std::string root = machine_root(machine_name) + "/C";
    std::string current = current_dir;

    if (current.size() >= 2 && current[1] == ':')
        current = current.substr(2);
    current = normalize_windows_path(current);

    if (!current.empty() && current.back() != '/')
        current += "/";

    std::string candidate = path;
    if (candidate.size() >= 2 && candidate[1] == ':')
    {
        candidate = candidate.substr(2);
    }
    candidate = normalize_windows_path(candidate);
    if (!candidate.empty() && candidate[0] == '/')
    {
        candidate = candidate.substr(1);
    }

    if (candidate.empty())
    {
        return root + current;
    }

    if (candidate[0] == '.')
    {
        return root + current + candidate.substr(1);
    }

    return root + current + candidate;
}

static std::vector<std::string> parse_windows_tokens(const std::string &input)
{
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;

    for (char ch : input)
    {
        if (ch == '"')
        {
            in_quotes = !in_quotes;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) && !in_quotes)
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty())
        tokens.push_back(current);

    return tokens;
}

void handle_move(const std::string &machine_name, const std::string &current_dir, const std::string &input)
{
    if (input.substr(0, 4) != "move")
        return;

    std::string args = input.substr(4);
    std::vector<std::string> tokens = parse_windows_tokens(args);
    if (tokens.size() != 2)
    {
        std::cout << "The syntax of the command is incorrect.\n";
        return;
    }

    std::string source = tokens[0];
    std::string destination = tokens[1];
    std::string from_path = resolve_windows_path(machine_name, current_dir, source);
    std::string to_path = resolve_windows_path(machine_name, current_dir, destination);

    if (!fs::exists(from_path))
    {
        std::cout << "The system cannot find the file specified.\n";
        return;
    }

    std::error_code ec;
    fs::path target_path(to_path);
    if (fs::exists(target_path) && fs::is_directory(target_path))
    {
        std::string leaf = fs::path(from_path).filename().string();
        to_path = (target_path / leaf).string();
    }

    fs::path parent = fs::path(to_path).parent_path();
    if (!parent.empty() && !fs::exists(parent))
    {
        std::cout << "The system cannot find the path specified.\n";
        return;
    }

    fs::rename(from_path, to_path, ec);
    if (ec)
    {
        std::cout << "The system cannot find the path specified.\n";
    }
}

void handle_rename(const std::string &machine_name, const std::string &current_dir, const std::string &input)
{
    if (input.substr(0, 6) != "rename" && input.substr(0, 3) != "ren")
        return;

    std::string args = input.substr(input.substr(0, 3) == "ren" ? 3 : 6);
    std::vector<std::string> tokens = parse_windows_tokens(args);
    if (tokens.size() != 2)
    {
        std::cout << "The syntax of the command is incorrect.\n";
        return;
    }

    std::string source = tokens[0];
    std::string destination = tokens[1];
    std::string from_path = resolve_windows_path(machine_name, current_dir, source);
    std::string to_path = resolve_windows_path(machine_name, current_dir, destination);

    if (!fs::exists(from_path))
    {
        std::cout << "The system cannot find the file specified.\n";
        return;
    }

    if (fs::exists(to_path))
    {
        std::cout << "A duplicate file name exists, or the file cannot be found.\n";
        return;
    }

    std::error_code ec;
    fs::rename(from_path, to_path, ec);
    if (ec)
    {
        std::cout << "The system cannot find the path specified.\n";
    }
}

void handle_list_directory(const std::string &machine_name, const std::string &current_dir)
{
    std::string real_path = machine_root(machine_name) + "/C";

    std::string win_path = current_dir;
    if (win_path.size() >= 2 && win_path[1] == ':')
        win_path = win_path.substr(2);

    for (char &c : win_path)
        if (c == '\\') c = '/';

    real_path += win_path;

    std::cout << " Directory of " << current_dir << "\n\n";

    try
    {
        bool has_entries = false;
        for (const auto &entry : fs::directory_iterator(real_path))
        {
            has_entries = true;
            auto last_write = fs::last_write_time(entry);
            std::time_t cftime = std::chrono::system_clock::to_time_t(
                std::chrono::file_clock::to_sys(last_write));

            char time_buf[20];
            std::strftime(time_buf, sizeof(time_buf), "%m/%d/%Y  %I:%M %p", std::localtime(&cftime));

            std::string name = entry.path().filename().string();

            if (entry.is_directory())
            {
                std::cout << time_buf << "    <DIR>          " << name << "\n";
            }
            else
            {
                std::cout << time_buf << "            " << entry.file_size()
                          << " " << name << "\n";
            }
        }

        if (!has_entries)
        {
            std::cout << " File Not Found\n";
        }
    }
    catch (...)
    {
        std::cout << "The system cannot find the path specified.\n";
    }

    std::cout << "               0 File(s)                 0 bytes\n";
    std::cout << "               0 Dir(s)   99,999,999,999 bytes free\n\n";
}

bool handle_change_directory(const std::string &machine_name, std::string &current_dir, const std::string &input)
{
    if (input.substr(0, 3) != "cd " && input != "cd")
        return false;

    if (input == "cd")
    {
        std::cout << current_dir << "\n";
        return true;
    }

    std::string target = input.substr(3);

    if (target == "\\" || target == "/" || target == ".")
    {
        current_dir = "C:\\";
        return true;
    }

    if (target == "..")
    {
        if (current_dir != "C:\\")
        {
            size_t pos = current_dir.find_last_of("\\", current_dir.length() - 2);
            current_dir = (pos != std::string::npos) ? current_dir.substr(0, pos + 1) : "C:\\";
        }
        return true;
    }

    std::string new_dir = current_dir;
    if (new_dir.back() != '\\') new_dir += "\\";
    new_dir += target;
    if (new_dir.back() != '\\') new_dir += "\\";

    std::string real_path = machine_root(machine_name) + "/C";
    std::string win_path = new_dir;
    if (win_path.size() >= 2 && win_path[1] == ':')
        win_path = win_path.substr(2);
    for (char &c : win_path)
        if (c == '\\') c = '/';
    real_path += win_path;

    if (fs::exists(real_path) && fs::is_directory(real_path))
    {
        current_dir = new_dir;
        return true;
    }
    else
    {
        std::cout << "The system cannot find the path specified.\n";
        return false;
    }
}

void handle_make_directory(const std::string &machine_name, const std::string &current_dir, const std::string &input)
{
    if (input.substr(0, 6) != "mkdir ")
        return;

    std::string dirname = input.substr(6);
    size_t start = dirname.find_first_not_of(" \t");
    if (start == std::string::npos)
    {
        std::cout << "The syntax of the command is incorrect.\n";
        return;
    }
    dirname = dirname.substr(start);

    if (dirname.empty())
    {
        std::cout << "The syntax of the command is incorrect.\n";
        return;
    }

    std::string real_path = machine_root(machine_name) + "/C";
    std::string win_path = current_dir;
    if (win_path.size() >= 2 && win_path[1] == ':')
        win_path = win_path.substr(2);
    for (char &c : win_path) if (c == '\\') c = '/';
    real_path += win_path;
    if (real_path.back() != '/') real_path += "/";
    real_path += dirname;

    try
    {
        if (fs::create_directories(real_path))
        {
        }
        else
        {
            std::cout << "A subdirectory or file " << dirname << " already exists.\n";
        }
    }
    catch (...)
    {
        std::cout << "The system cannot find the path specified.\n";
    }
}

void handle_echo(const std::string &machine_name, const std::string &current_dir, const std::string &input)
{
    if (input.substr(0, 5) != "echo ")
        return;

    std::string args = input.substr(5);

    size_t redirect_pos = args.find(" >");
    if (redirect_pos == std::string::npos)
        redirect_pos = args.find(" >>");

    if (redirect_pos == std::string::npos)
    {
        std::cout << args << "\n";
        return;
    }

    bool append = (args.substr(redirect_pos, 3) == " >>");
    std::string text = args.substr(0, redirect_pos);
    std::string filename = args.substr(redirect_pos + (append ? 3 : 2));

    size_t start = filename.find_first_not_of(" \t");
    if (start != std::string::npos)
        filename = filename.substr(start);

    if (filename.empty())
    {
        std::cout << "The syntax of the command is incorrect.\n";
        return;
    }

    std::string real_path = machine_root(machine_name) + "/C";

    std::string win_path = current_dir;
    if (win_path.size() >= 2 && win_path[1] == ':')
        win_path = win_path.substr(2);
    for (char &c : win_path)
        if (c == '\\') c = '/';

    real_path += win_path;
    if (real_path.back() != '/') real_path += "/";
    real_path += filename;

    try
    {
        std::ofstream file(real_path, append ? std::ios::app : std::ios::out);
        if (file.is_open())
        {
            file << text << "\n";
            file.close();
        }
        else
        {
            std::cout << "Error: Could not open file.\n";
        }
    }
    catch (...)
    {
        std::cout << "Error writing to file.\n";
    }
}

void handle_type(const std::string &machine_name, const std::string &current_dir, const std::string &input)
{
    if (input.substr(0, 5) != "type ") return;

    std::string filename = input.substr(5);
    size_t start = filename.find_first_not_of(" \t");
    if (start != std::string::npos)
        filename = filename.substr(start);

    if (filename.empty())
    {
        std::cout << "The syntax of the command is incorrect.\n";
        return;
    }

    std::string real_path = machine_root(machine_name) + "/C";
    std::string win_path = current_dir;
    if (win_path.size() >= 2 && win_path[1] == ':')
        win_path = win_path.substr(2);
    for (char &c : win_path) if (c == '\\') c = '/';
    real_path += win_path;
    if (real_path.back() != '/') real_path += "/";
    real_path += filename;

    try
    {
        std::ifstream file(real_path);
        if (!file.is_open())
        {
            std::cout << "The system cannot find the file " << filename << ".\n";
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            std::cout << line << "\n";
        }
        file.close();
    }
    catch (...)
    {
        std::cout << "The system cannot find the file " << filename << ".\n";
    }
}