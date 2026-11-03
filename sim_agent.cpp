#include "sim_agent.h"
#include "fs/fs_commands.h"
#include "registry/reg_commands.h"
#include "network/net_commands.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void SimAgent::login(const std::string &machine_name)
{
    if (!fs::exists(machine_name))
    {
        std::cout << "Machine '" << machine_name << "' does not exist.\n";
        return;
    }

    std::string current_dir = "C:\\";
    std::string input;

    std::cout << "\nMicrosoft Windows [Version 10.0.19045.4355]\n";
    std::cout << "(c) Microsoft Corporation. All rights reserved.\n\n";

    while (true)
    {
        std::cout << machine_name << " " << current_dir << ">";
        std::getline(std::cin, input);

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
            std::ifstream file(machine_name + "/systeminfo.txt");
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
            continue;
        }

        if (input == "ipconfig" || input == "ipconfig /all")
        {
            handle_ipconfig(machine_name);
            continue;
        }

        if (input == "getmac")
        {
            handle_getmac(machine_name);
            continue;
        }

        if (input == "whoami")
        {
            handle_whoami(machine_name);
            continue;
        }

        if (input == "netstat")
        {
            handle_netstat(machine_name, false);
            continue;
        }

        if (input == "netstat -ano")
        {
            handle_netstat(machine_name, true);
            continue;
        }

        if (input == "net user")
        {
            handle_net_user(machine_name);
            continue;
        }

        if (input == "arp")
        {
            handle_arp(machine_name, false);
            continue;
        }

        if (input == "arp -a")
        {
            handle_arp(machine_name, true);
            continue;
        }

        // Get-CimInstance -ClassName Win32_UserAccount -Filter "LocalAccount='True'"
        if (input == "Get-CimInstance -ClassName Win32_UserAccount -Filter \"LocalAccount='True'\"")
        {
            handle_get_cim_useraccount(machine_name);
            continue;
        }

        if (input == "query user" || input == "quser")
        {
            handle_query_user(machine_name);
            continue;
        }
    }
}