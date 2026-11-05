#include "sim_agent.h"
#include "fs/fs_commands.h"
#include "registry/reg_commands.h"
#include "network/net_commands.h"
#include "process/process_commands.h"
#include "events/handle_get_security_events.h"
#include "tasks/tasks_commands.h"
#include <cctype>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <termios.h>
#include <unistd.h>

namespace fs = std::filesystem;

static void render_command_line(const std::string &prompt, const std::string &line, size_t previous_size)
{
    std::cout << "\r" << prompt << line;
    if (previous_size > line.size())
    {
        std::cout << std::string(previous_size - line.size(), ' ');
    }
    std::cout << "\r" << prompt << line << std::flush;
}

static std::string read_command_with_history(const std::string &prompt, std::vector<std::string> &history)
{
    struct termios original;
    if (tcgetattr(STDIN_FILENO, &original) != 0)
    {
        std::string result;
        std::getline(std::cin, result);
        if (!result.empty() && (history.empty() || history.back() != result))
        {
            history.push_back(result);
        }
        return result;
    }

    struct termios raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    std::string current;
    std::string draft;
    size_t history_cursor = history.size();
    size_t previous_size = 0;

    std::cout << prompt << std::flush;

    while (true)
    {
        char ch = 0;
        if (read(STDIN_FILENO, &ch, 1) <= 0)
        {
            continue;
        }

        if (ch == '\n' || ch == '\r')
        {
            std::cout << "\n";
            break;
        }

        if (ch == 0x1b)
        {
            char seq1 = 0;
            char seq2 = 0;
            if (read(STDIN_FILENO, &seq1, 1) <= 0)
            {
                continue;
            }
            if (read(STDIN_FILENO, &seq2, 1) <= 0)
            {
                continue;
            }

            if (seq1 == '[')
            {
                switch (seq2)
                {
                    case 'A':
                        if (history_cursor > 0)
                        {
                            if (history_cursor == history.size())
                            {
                                draft = current;
                            }
                            --history_cursor;
                            current = history[history_cursor];
                            render_command_line(prompt, current, previous_size);
                            previous_size = current.size();
                        }
                        break;
                    case 'B':
                        if (history_cursor < history.size())
                        {
                            ++history_cursor;
                            current = (history_cursor == history.size()) ? draft : history[history_cursor];
                            render_command_line(prompt, current, previous_size);
                            previous_size = current.size();
                        }
                        break;
                    default:
                        break;
                }
            }
            continue;
        }

        if (ch == 127 || ch == 8)
        {
            if (!current.empty())
            {
                current.pop_back();
            }
            render_command_line(prompt, current, previous_size);
            previous_size = current.size();
            continue;
        }

        if (ch >= 32)
        {
            current.push_back(ch);
            std::cout << ch << std::flush;
            previous_size = current.size();
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &original);

    if (!current.empty() && (history.empty() || history.back() != current))
    {
        history.push_back(current);
    }

    return current;
}

static std::string machine_root(const std::string &machine_name)
{
    return "devices/" + machine_name;
}

void SimAgent::login(const std::string &machine_name)
{
    if (!fs::exists(machine_root(machine_name)))
    {
        std::cout << "Machine '" << machine_name << "' does not exist.\n";
        return;
    }

    std::string current_dir = "C:\\";
    std::string input;
    std::vector<std::string> command_history;

    std::cout << "\nMicrosoft Windows [Version 10.0.19045.4355]\n";
    std::cout << "(c) Microsoft Corporation. All rights reserved.\n\n";

    while (true)
    {
        input = read_command_with_history(machine_name + " " + current_dir + ">", command_history);

        if (input == "exit" || input == "logout")
        {
            std::cout << "Logged out from " << machine_name << "\n";
            break;
        }

        if (input == "cls" || input == "clear")
        {
            std::cout << "\033[2J\033[H";
            continue;
        }

        if (input == "dir" || input == "ls")
        {
            handle_list_directory(machine_name, current_dir);
            continue;
        }

        if (handle_change_directory(machine_name, current_dir, input))
        {
            continue;
        }

        if (input.substr(0, 6) == "mkdir " || input == "mkdir")
        {
            handle_make_directory(machine_name, current_dir, input);
            continue;
        }

        if (input.substr(0, 4) == "move")
        {
            handle_move(machine_name, current_dir, input);
            continue;
        }

        if (input.substr(0, 6) == "rename" || input.substr(0, 3) == "ren")
        {
            handle_rename(machine_name, current_dir, input);
            continue;
        }

        if (input.substr(0, 5) == "echo ")
        {
            handle_echo(machine_name, current_dir, input);
            continue;
        }

        if (input.substr(0, 5) == "type ")
        {
            handle_type(machine_name, current_dir, input);
            continue;
        }

        if (input.substr(0, 4) == "reg ")
        {
            std::string args = input.substr(4);

            if (args.size() >= 6 && args.substr(0, 6) == "query ")
            {
                std::string reg_path = args.substr(6);
                handle_reg_query(machine_name, reg_path);
            }
            else
            {
                std::cout << "ERROR: Invalid syntax.\n";
            }
            continue;
        }

        if (input == "systeminfo")
        {
            std::ifstream file(machine_root(machine_name) + "/systeminfo.txt");
            if (file.is_open())
            {
                std::string line;
                while (std::getline(file, line))
                {
                    std::cout << line << "\n";
                }
                file.close();
            }
            else
            {
                std::cout << "ERROR: Unable to retrieve system information.\n";
            }
            log_command_security_event(machine_name, "systeminfo.exe", input);
            continue;
        }

        if (input == "ipconfig" || input == "ipconfig /all")
        {
            handle_ipconfig(machine_name);
            log_command_security_event(machine_name, "ipconfig.exe", input == "ipconfig /all" ? "ipconfig /all" : "ipconfig");
            continue;
        }

        if (input == "hostname")
        {
            handle_hostname(machine_name);
            log_command_security_event(machine_name, "hostname.exe", input);
            continue;
        }

        if (input == "getmac")
        {
            handle_getmac(machine_name);
            log_command_security_event(machine_name, "getmac.exe", input);
            continue;
        }

        if (input == "whoami")
        {
            handle_whoami(machine_name);
            log_command_security_event(machine_name, "whoami.exe", input);
            continue;
        }

        if (input == "netstat")
        {
            handle_netstat(machine_name, false);
            log_command_security_event(machine_name, "netstat.exe", input);
            continue;
        }

        if (input == "netstat -ano")
        {
            handle_netstat(machine_name, true);
            log_command_security_event(machine_name, "netstat.exe", input);
            continue;
        }

        if (input == "net user")
        {
            handle_net_user(machine_name);
            log_command_security_event(machine_name, "net.exe", "net user");
            continue;
        }

        if (input == "arp")
        {
            handle_arp(machine_name, false);
            log_command_security_event(machine_name, "arp.exe", input);
            continue;
        }

        if (input == "arp -a")
        {
            handle_arp(machine_name, true);
            log_command_security_event(machine_name, "arp.exe", input);
            continue;
        }

        if (input == "Get-CimInstance -ClassName Win32_UserAccount -Filter \"LocalAccount='True'\"")
        {
            handle_get_cim_useraccount(machine_name);
            continue;
        }

        if (input == "query user" || input == "quser")
        {
            handle_query_user(machine_name);
            log_command_security_event(machine_name, "quser.exe", input);
            continue;
        }

        if (input == "schtasks")
        {
            handle_schtasks(machine_name, input);
            continue;
        }

        if (input.find("schtasks") == 0)
        {
            handle_schtasks(machine_name, input);
            continue;
        }

        if (input == "tasklist")
        {
            handle_tasklist(machine_name);
            log_command_security_event(machine_name, "tasklist.exe", input);
            continue;
        }

        if (input.substr(0, 8) == "taskkill" || input == "taskkill")
        {
            handle_taskkill(machine_name, input);
            log_command_security_event(machine_name, "taskkill.exe", input);
            continue;
        }

        if (input == "events" || input == "get events")
        {
            handle_get_security_events(machine_name);
            continue;
        }

        if (input == "Get-WinEvent -LogName System -MaxEvents 20")
        {
            handle_get_security_events(machine_name);
            continue;
        }

        if (input == "Get-WinEvent -LogName Security -MaxEvents 20")
        {
            handle_get_security_log_events(machine_name);
            continue;
        }

        if (input.find("Get-WinEvent -FilterHashtable @{LogName='Security'; Id=4688} -MaxEvents 1") == 0 ||
            input.find("Get-WinEvent -FilterHashtable @{LogName='Security'; Id=4688} -MaxEvents 1 | Format-List *") == 0)
        {
            handle_get_security_event_filter_by_id(machine_name, 4688, 1);
            continue;
        }

        if (input.find("Get-WinEvent -FilterHashtable @{LogName='Security'; Id=4688} -MaxEvents ") == 0)
        {
            size_t max_pos = input.find(" -MaxEvents ");
            size_t pipe_pos = input.find(" | ");
            std::string max_value = input.substr(max_pos + std::string(" -MaxEvents ").size());
            if (pipe_pos != std::string::npos)
            {
                max_value = max_value.substr(0, pipe_pos - max_pos - std::string(" -MaxEvents ").size());
            }

            try
            {
                int max_events = std::stoi(max_value);
                bool select_last_one = (input.find("Select-Object -Last 1") != std::string::npos);
                handle_get_security_event_filter_by_id(machine_name, 4688, max_events, select_last_one);
            }
            catch (...) {}
            continue;
        }

        if (input.find("Get-WinEvent -FilterHashtable @{LogName='Security';") == 0 ||
            input.find("Get-WinEvent -FilterHashtable @{LogName=\"Security\";") == 0)
        {
            size_t id_pos = input.find("Id");
            if (id_pos != std::string::npos)
            {
                size_t eq_pos = input.find('=', id_pos);
                if (eq_pos != std::string::npos)
                {
                    size_t val_start = eq_pos + 1;
                    while (val_start < input.size() && (std::isspace(static_cast<unsigned char>(input[val_start])) || input[val_start] == ';'))
                    {
                        ++val_start;
                    }

                    size_t val_end = val_start;
                    while (val_end < input.size() && std::isdigit(static_cast<unsigned char>(input[val_end])))
                    {
                        ++val_end;
                    }

                    if (val_end > val_start)
                    {
                        try
                        {
                            int event_id = std::stoi(input.substr(val_start, val_end - val_start));
                            size_t max_pos = input.find(" -MaxEvents ");
                            if (max_pos != std::string::npos)
                            {
                                size_t max_start = max_pos + std::string(" -MaxEvents ").size();
                                size_t max_end = input.find(" ", max_start);
                                if (max_end == std::string::npos)
                                    max_end = input.size();

                                if (max_start < input.size())
                                {
                                    int max_events = std::stoi(input.substr(max_start, max_end - max_start));
                                    bool select_last_one = (input.find("Select-Object -Last 1") != std::string::npos);
                                    handle_get_security_event_filter_by_id(machine_name, event_id, max_events, select_last_one);
                                    continue;
                                }
                            }
                        }
                        catch (...) {}
                    }
                }
            }
        }

        if (input.find("sc create") == 0)
        {
            handle_sc_create(machine_name, input);
            continue;
        }

        if (input == "sc query")
        {
            handle_sc_query(machine_name);
            continue;
        }

        if (input == "Get-Service")
        {
            handle_get_service(machine_name);
            continue;
        }

        if (input == "net start")
        {
            handle_net_start(machine_name);
            continue;
        }
    }
}