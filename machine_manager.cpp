#include "machine_manager.h"
#include <iostream>
#include <filesystem>
#include "sim_agent.h"
#include <fstream>

namespace fs = std::filesystem;

/**
 * Handles exit command.
 */
bool handle_exit(const std::string &input)
{
    if (input == "exit" || input == "quit")
    {
        return true;
    }
    return false;
}

/**
 * Handles clear command.
 */
bool handle_clear(const std::string &input)
{
    if (input == "clear")
    {
        std::cout << "\033[2J\033[H";
        return true;
    }
    return false;
}

/**
 * Builds simulated file system.
 */
void scaffold_fs(const std::string &machine_name)
{
    fs::create_directories(machine_name + "/C/Windows/System32");
    fs::create_directories(machine_name + "/C/Windows/System32/config");
    fs::create_directories(machine_name + "/C/Program Files");
    fs::create_directories(machine_name + "/C/Program Files (x86)");
    fs::create_directories(machine_name + "/C/Users/Administrator/Desktop");
    fs::create_directories(machine_name + "/C/Users/Administrator/Documents");
    fs::create_directories(machine_name + "/C/Users/Public");
    fs::create_directories(machine_name + "/C/PerfLogs");
    fs::create_directories(machine_name + "/C/Windows/Temp");
    std::cout << "machine '" << machine_name << "' added\n";
}

/**
 * Builds simulated network configuration.
 */
void scaffold_netconfig(const std::string &machine_name)
{
    std::ofstream ipconfig_file(machine_name + "/ipconfig.txt");
    if (ipconfig_file.is_open())
    {
        ipconfig_file << "Windows IP Configuration\n\n";
        ipconfig_file << "Ethernet adapter Ethernet:\n\n";
        ipconfig_file << "   Connection-specific DNS Suffix  . : lab.local\n";
        ipconfig_file << "   IPv4 Address. . . . . . . . . . . : 192.168.10.10\n";
        ipconfig_file << "   Subnet Mask . . . . . . . . . . . : 255.255.255.0\n";
        ipconfig_file << "   Default Gateway . . . . . . . . . : 192.168.10.1\n";
        ipconfig_file.close();
    }
}

/**
 * Builds a simulated MAC.
 */
void scaffold_mac(const std::string &machine_name)
{
    std::ofstream file(machine_name + "/mac.txt");
    if (file.is_open())
    {
        srand(static_cast<unsigned int>(time(nullptr)) + static_cast<unsigned int>(machine_name.size()));

        file << std::hex << std::uppercase << std::setfill('0');
        file << std::setw(2) << (rand() % 256) << "-";
        file << std::setw(2) << (rand() % 256) << "-";
        file << std::setw(2) << (rand() % 256) << "-";
        file << std::setw(2) << (rand() % 256) << "-";
        file << std::setw(2) << (rand() % 256) << "-";
        file << std::setw(2) << (rand() % 256);
        file << std::dec << "\n";

        file.close();
    }
}

/**
 * Builds simulated registry.
 */
void scaffold_registry(const std::string &machine_name)
{
    fs::create_directories(machine_name + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/Run");
    fs::create_directories(machine_name + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/RunOnce");
    std::ofstream run_value(machine_name + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/Run/SecurityHealth");
    if (run_value.is_open())
    {
        run_value << "REG_SZ\n";
        run_value << "C:\\Program Files\\Windows Defender\\MSASCuiL.exe\n";
        run_value.close();
    }

    std::string services_base = machine_name + "/registry/HKLM/SYSTEM/CurrentControlSet/Services";

    fs::create_directories(services_base + "/ACPI");
    fs::create_directories(services_base + "/AFD");
    fs::create_directories(services_base + "/Appinfo");
    fs::create_directories(services_base + "/BFE");
    fs::create_directories(services_base + "/CryptSvc");
    fs::create_directories(services_base + "/DcomLaunch");
    fs::create_directories(services_base + "/Dhcp");
    fs::create_directories(services_base + "/Dnscache");
    fs::create_directories(services_base + "/EventLog");
    fs::create_directories(services_base + "/gpsvc");
    fs::create_directories(services_base + "/LanmanServer");
    fs::create_directories(services_base + "/LanmanWorkstation");
    fs::create_directories(services_base + "/RpcEptMapper");
    fs::create_directories(services_base + "/RpcSs");
    fs::create_directories(services_base + "/SamSs");
    fs::create_directories(services_base + "/Schedule");
    fs::create_directories(services_base + "/Spooler");
    fs::create_directories(services_base + "/WinDefend");
    fs::create_directories(services_base + "/Winmgmt");
    fs::create_directories(services_base + "/W32Time");
}

/**
 * Builds simulated system information.
 */
void scaffold_systeminfo(const std::string &machine_name)
{
    std::ofstream file(machine_name + "/systeminfo.txt");
    if (file.is_open())
    {
        file << "Host Name:                 " << machine_name << "\n";
        file << "OS Name:                   Microsoft Windows 10 Pro\n";
        file << "OS Version:                10.0.19045 N/A Build 19045\n";
        file << "OS Manufacturer:           Microsoft Corporation\n";
        file << "OS Configuration:          Standalone Workstation\n";
        file << "OS Build Type:             Multiprocessor Free\n";
        file << "Registered Owner:          Administrator\n";
        file << "Registered Organization:  \n";
        file << "Product ID:                00330-80000-00000-AA123\n";
        file << "Original Install Date:     3/15/2024, 9:22:11 AM\n";
        file << "System Boot Time:          10/8/2026, 8:01:44 AM\n";
        file << "System Manufacturer:       WinSim Virtual\n";
        file << "System Model:              WinSim-PC\n";
        file << "System Type:               x64-based PC\n";
        file << "Processor(s):              1 Processor(s) Installed.\n";
        file << "                           [01]: Intel64 Family 6 Model 158 Stepping 10 GenuineIntel ~3600 Mhz\n";
        file << "BIOS Version:              WinSim BIOS 1.0, 1/1/2024\n";
        file << "Windows Directory:         C:\\Windows\n";
        file << "System Directory:          C:\\Windows\\System32\n";
        file << "Boot Device:               \\Device\\HarddiskVolume1\n";
        file << "System Locale:             en-us;English (United States)\n";
        file << "Input Locale:              en-us;English (United States)\n";
        file << "Time Zone:                 (UTC-05:00) Eastern Time (US & Canada)\n";
        file << "Total Physical Memory:     8,192 MB\n";
        file << "Available Physical Memory: 5,124 MB\n";
        file << "Virtual Memory: Max Size:  9,472 MB\n";
        file << "Virtual Memory: Available: 6,201 MB\n";
        file << "Virtual Memory: In Use:    3,271 MB\n";
        file << "Page File Location(s):     C:\\pagefile.sys\n";
        file << "Domain:                    WORKGROUP\n";
        file << "Logon Server:              \\\\" << machine_name << "\n";
        file << "Hotfix(s):              7 Hotfix(s) Installed.\n";
        file << "Network Card(s):           1 NIC(s) Installed.\n";
        file << "                           [01]: WinSim Virtual Ethernet Adapter\n";
        file << "                                 Connection Name: Ethernet\n";
        file << "                                 DHCP Enabled:    Yes\n";
        file << "                                 DHCP Server:     192.168.10.1\n";
        file << "                                 IP address(es)\n";
        file << "                                 [01]: 192.168.10.10\n";
        file.close();
    }
}

void scaffold_ssh(const std::string &machine_name)
{
    std::ofstream file(machine_name + "/ssh");
    if (file.is_open())
    {
        file << "Administrator\n";
        file << "ssl_enabled=true\n";
        file.close();
    }
}

void scaffold_netstat(const std::string &machine_name)
{
    std::ofstream file(machine_name + "/netstat.txt");
    if (file.is_open())
    {
        file << "TCP  0.0.0.0:135            0.0.0.0:0              LISTENING       912\n";
        file << "TCP  0.0.0.0:445            0.0.0.0:0              LISTENING       4\n";
        file << "TCP  0.0.0.0:3389           0.0.0.0:0              LISTENING       1204\n";
        file << "TCP  192.168.10.10:139      0.0.0.0:0              LISTENING       4\n";
        file << "TCP  127.0.0.1:49715        0.0.0.0:0              LISTENING       2148\n";
        file << "UDP  0.0.0.0:123            *:*                                    1108\n";
        file << "UDP  0.0.0.0:500            *:*                                    1204\n";
        file << "UDP  192.168.10.10:137      *:*                                    4\n";
        file << "UDP  192.168.10.10:138      *:*                                    4\n";
        file.close();
    }
}

void scaffold_users(const std::string &machine_name)
{
    std::ofstream file(machine_name + "/users");
    if (file.is_open())
    {
        file << "Administrator\n";
        file << "DefaultAccount\n";
        file << "Guest\n";
        file << "WDAGUtilityAccount\n";
        file.close();
    }
}

void scaffold_arp(const std::string &machine_name)
{
    std::ofstream file(machine_name + "/arp.txt");
    if (file.is_open())
    {
        file << "192.168.10.1          00-11-22-33-44-55     dynamic\n";
        file << "192.168.10.255        ff-ff-ff-ff-ff-ff     static\n";
        file << "224.0.0.22            01-00-5e-00-00-16     static\n";
        file << "224.0.0.251           01-00-5e-00-00-fb     static\n";
        file << "224.0.0.252           01-00-5e-00-00-fc     static\n";
        file << "239.255.255.250       01-00-5e-7f-ff-fa     static\n";
        file.close();
    }
}

void scaffold_sessions(const std::string &machine_name)
{
    std::ofstream file(machine_name + "/sessions");
    if (file.is_open())
    {
        file << "Administrator console 1 Active none 10/8/2026 8:01 AM\n";
        file.close();
    }
}

/**
 * Adds a machine.
 */
void handle_add_machine(const std::string &input)
{
    if (input.substr(0, 11) == "add machine")
    {
        if (input.length() > 12)
        {
            std::string machine_name = input.substr(12);
            scaffold_fs(machine_name);
            scaffold_netconfig(machine_name);
            scaffold_registry(machine_name);
            scaffold_systeminfo(machine_name);
            scaffold_ssh(machine_name);
            scaffold_mac(machine_name);
            scaffold_netstat(machine_name);
            scaffold_users(machine_name);
            scaffold_arp(machine_name);
            scaffold_sessions(machine_name);
        }
        else
        {
            std::cout << "usage: add machine <machine_name>\n";
        }
    }
}

void handle_login(const std::string &input)
{
    if (input.substr(0, 4) == "ssh ")
    {
        std::string target = input.substr(4);

        size_t at_pos = target.find('@');
        if (at_pos == std::string::npos)
        {
            std::cout << "usage: ssh <user>@<machine>\n";
            return;
        }

        std::string user = target.substr(0, at_pos);
        std::string machine_name = target.substr(at_pos + 1);

        if (!fs::exists(machine_name))
        {
            std::cout << "ssh: Could not resolve hostname " << machine_name << "\n";
            return;
        }

        std::ifstream ssh_file(machine_name + "/ssh");

        if (!ssh_file.is_open())
        {
            std::cout << "ssh: Connection refused\n";
            return;
        }

        std::string stored_user;
        std::string ssl_line;
        std::getline(ssh_file, stored_user);
        std::getline(ssh_file, ssl_line);
        ssh_file.close();

        if (!stored_user.empty() && stored_user.back() == '\r')
            stored_user.pop_back();

        bool ssl_enabled = (ssl_line.find("ssl_enabled=true") != std::string::npos);

        if (!ssl_enabled)
        {
            std::cout << "ssh: Connection refused\n";
            return;
        }

        if (user != stored_user)
        {
            std::cout << "Permission denied, please try again.\n";
            return;
        }

        SimAgent agent;
        agent.login(machine_name);
        return;
    }
}

void MachineManager::run()
{
    std::string input;

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (handle_exit(input))
        {
            break;
        }

        if (handle_clear(input))
        {
            continue;
        }

        handle_add_machine(input);
        handle_login(input);
    }
}