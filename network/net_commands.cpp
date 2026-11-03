#include "net_commands.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <iomanip>   
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

void handle_ipconfig(const std::string &machine_name)
{
    std::ifstream file(machine_name + "/ipconfig.txt");

    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve IP configuration.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::cout << line << "\n";
    }

    file.close();
}

void handle_getmac(const std::string &machine_name)
{
    std::ifstream file(machine_name + "/mac.txt");
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve MAC address.\n";
        return;
    }

    std::string mac;
    std::getline(file, mac);
    file.close();

    if (!mac.empty() && mac.back() == '\r')
        mac.pop_back();

    std::cout << "\n";
    std::cout << "Physical Address    Transport Name\n";
    std::cout << "=================== ==========================================================\n";
    std::cout << mac << "   \\Device\\Tcpip_{7A2F1C8E-4B91-4D3A-9E22-1F8C6D4A9B12}\n";
    std::cout << "\n";
}

void handle_whoami(const std::string &machine_name)
{
    std::ifstream file(machine_name + "/ssh");

    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve user details.\n";
        return;
    }

    std::string user;
    std::getline(file, user);
    file.close();

    if (!user.empty() && user.back() == '\r')
    {
        user.pop_back();
    }

    std::cout << machine_name << "\\" << user << "\n";
}

void handle_netstat(const std::string &machine_name, bool show_pid)
{
    std::ifstream file(machine_name + "/netstat.txt");
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve network statistics.\n";
        return;
    }

    std::cout << "\nActive Connections\n\n";

    if (show_pid)
    {
        std::cout << "  Proto  Local Address          Foreign Address        State           PID\n";
    }
    else
    {
        std::cout << "  Proto  Local Address          Foreign Address        State\n";
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (show_pid)
        {
            std::cout << "  " << line << "\n";
        }
        else
        {
            // Drop the trailing PID column (last token)
            size_t pos = line.find_last_of(" \t");
            if (pos != std::string::npos)
                line = line.substr(0, pos);
            std::cout << "  " << line << "\n";
        }
    }

    std::cout << "\n";
    file.close();
}

void handle_net_user(const std::string &machine_name)
{
    std::ifstream file(machine_name + "/users");
    if (!file.is_open())
    {
        std::cout << "The command failed to complete successfully.\n";
        return;
    }

    std::cout << "\nUser accounts for \\\\" << machine_name << "\n\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    std::string user;
    int count = 0;
    std::string line;

    while (std::getline(file, user))
    {
        if (!user.empty() && user.back() == '\r')
            user.pop_back();

        if (user.empty())
            continue;

        std::cout << std::left << std::setw(25) << user;
        count++;

        if (count % 3 == 0)
            std::cout << "\n";
    }

    if (count % 3 != 0)
        std::cout << "\n";

    std::cout << "The command completed successfully.\n\n";
    file.close();
}

void handle_arp(const std::string &machine_name, bool show_table)
{
    if (!show_table)
    {
        std::cout << "\nDisplays and modifies the IP-to-Physical address translation tables used by address resolution protocol (ARP).\n\n";
        std::cout << "ARP -s inet_addr eth_addr [if_addr]\n";
        std::cout << "ARP -d inet_addr [if_addr]\n";
        std::cout << "ARP -a [inet_addr] [-N if_addr] [-v]\n\n";
        return;
    }

    std::ifstream file(machine_name + "/arp.txt");
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve ARP table.\n";
        return;
    }

    std::cout << "\nInterface: 192.168.10.10 --- 0x5\n";
    std::cout << "  Internet Address      Physical Address      Type\n";

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::cout << "  " << line << "\n";
    }

    std::cout << "\n";
    file.close();
}

void handle_get_cim_useraccount(const std::string &machine_name)
{
    std::ifstream file(machine_name + "/users");
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve user accounts.\n";
        return;
    }

    std::cout << "\n";

    std::string user;
    
    while (std::getline(file, user))
    {
        if (!user.empty() && user.back() == '\r')
            user.pop_back();

        if (user.empty())
            continue;

        std::cout << "Name              : " << user << "\n";
        std::cout << "Caption           : " << machine_name << "\\" << user << "\n";
        std::cout << "AccountType       : 512\n";
        std::cout << "Domain            : " << machine_name << "\n";
        std::cout << "Disabled          : False\n";
        std::cout << "LocalAccount      : True\n";
        std::cout << "Lockout           : False\n";
        std::cout << "PasswordChangeable: True\n";
        std::cout << "PasswordExpires   : False\n";
        std::cout << "PasswordRequired  : True\n";
        std::cout << "SID               : S-1-5-21-1234567890-1234567890-1234567890-500\n";
        std::cout << "FullName          : \n";
        std::cout << "Description       : \n";
        std::cout << "Status            : OK\n";
        std::cout << "\n";
    }

    file.close();
}

void handle_query_user(const std::string &machine_name)
{
    std::ifstream file(machine_name + "/sessions");
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve session information.\n";
        return;
    }

    std::cout << "\n USERNAME              SESSIONNAME        ID  STATE   IDLE TIME  LOGON TIME\n";

    std::string line;
    bool first = true;

    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::string user, session, id, state, idle, date, time, ampm;

        iss >> user >> session >> id >> state >> idle >> date >> time >> ampm;

        if (first)
        {
            std::cout << ">" << std::left
                      << std::setw(22) << user
                      << std::setw(19) << session
                      << std::setw(4)  << id
                      << std::setw(8)  << state
                      << std::setw(10) << idle
                      << date << " " << time << " " << ampm << "\n";
            first = false;
        }
        else
        {
            std::cout << " " << std::left
                      << std::setw(22) << user
                      << std::setw(19) << session
                      << std::setw(4)  << id
                      << std::setw(8)  << state
                      << std::setw(10) << idle
                      << date << " " << time << " " << ampm << "\n";
        }
    }

    std::cout << "\n";
    file.close();
}