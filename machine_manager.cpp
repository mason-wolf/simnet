#include "machine_manager.h"
#include <iostream>
#include <filesystem>
#include "sim_agent.h"
#include <fstream>
#include "tasks/tasks_commands.h"

namespace fs = std::filesystem;

static std::string machine_root(const std::string &machine_name)
{
    return "devices/" + machine_name;
}

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
    const std::string root = machine_root(machine_name);
    fs::create_directories(root + "/C/Windows/System32");
    fs::create_directories(root + "/C/Windows/System32/config");
    fs::create_directories(root + "/C/Program Files");
    fs::create_directories(root + "/C/Program Files (x86)");
    fs::create_directories(root + "/C/Users/Administrator/Desktop");
    fs::create_directories(root + "/C/Users/Administrator/Documents");
    fs::create_directories(root + "/C/Users/Public");
    fs::create_directories(root + "/C/PerfLogs");
    fs::create_directories(root + "/C/Windows/Temp");
    std::cout << "machine '" << machine_name << "' added\n";
}

/**
 * Builds simulated network configuration.
 */
void scaffold_netconfig(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    std::ofstream ipconfig_file(root + "/ipconfig.txt");
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
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/mac.txt");
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
    const std::string root = machine_root(machine_name);
    fs::create_directories(root + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/Run");
    fs::create_directories(root + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/RunOnce");
    std::ofstream run_value(root + "/registry/HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion/Run/SecurityHealth");
    if (run_value.is_open())
    {
        run_value << "REG_SZ\n";
        run_value << "C:\\Program Files\\Windows Defender\\MSASCuiL.exe\n";
        run_value.close();
    }

    std::string services_base = root + "/registry/HKLM/SYSTEM/CurrentControlSet/Services";

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
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/systeminfo.txt");
    if (file.is_open())
    {
        file << "Host Name:                 " << machine_name << "\n";
        file << "OS Name:                  Microsoft Windows 11 Pro\n";
        file << "OS Version:               10.0.19045 N/A Build 19045\n";
        file << "System Manufacturer:      WinSim Virtual\n";
        file << "System Model:             WinSim-PC\n";
        file << "System Type:              x64-based PC\n";
        file << "Processor(s):             1 Processor(s) Installed.\n";
        file << "                          [01]: Intel64 Family 6 Model 158 Stepping 10 GenuineIntel ~3600 Mhz\n";
        file << "BIOS Version:             WinSim BIOS 1.0, 1/1/2024\n";
        file << "Windows Directory:        C:\\Windows\n";
        file << "System Directory:         C:\\Windows\\System32\n";
        file << "Boot Device:              \\Device\\HarddiskVolume1\n";
        file << "System Locale:            en-us;English (United States)\n";
        file << "Input Locale:             en-us;English (United States)\n";
        file << "Time Zone:                (UTC-05:00) Eastern Time (US & Canada)\n";
        file << "Total Physical Memory:    8,192 MB\n";
        file << "Available Physical Memory: 5,124 MB\n";
        file << "Virtual Memory: Max Size: 9,472 MB\n";
        file << "Virtual Memory: Available: 6,201 MB\n";
        file << "Virtual Memory: In Use:   3,271 MB\n";
        file << "Page File Location(s):    C:\\pagefile.sys\n";
        file << "Domain:                   WORKGROUP\n";
        file << "Logon Server:             \\" << machine_name << "\n";
        file << "Hotfix(s):                7 Hotfix(s) Installed.\n";
        file << "Network Card(s):          1 NIC(s) Installed.\n";
        file << "                          [01]: WinSim Virtual Ethernet Adapter\n";
        file << "                                Connection Name: Ethernet\n";
        file << "                                DHCP Enabled:    Yes\n";
        file << "                                DHCP Server:     192.168.10.1\n";
        file << "                                IP address(es)\n";
        file << "                                [01]: 192.168.10.10\n";
        file.close();
    }
}

void scaffold_ssh(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/ssh");
    if (file.is_open())
    {
        file << "Administrator\n";
        file << "ssl_enabled=true\n";
        file.close();
    }
}

void scaffold_netstat(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/netstat.txt");
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
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/users");
    if (file.is_open())
    {
        file << "Administrator\n";
        file << "Guest\n";
        file.close();
    }
}

void scaffold_arp(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/arp.txt");
    if (file.is_open())
    {
        file << "Interface: 192.168.10.10 --- 0x5\n";
        file << "192.168.10.1     00-11-22-33-44-55     dynamic\n";
        file << "192.168.10.5     66-77-88-99-aa-bb     dynamic\n";
        file.close();
    }
}

void scaffold_sessions(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/sessions");
    if (file.is_open())
    {
        file << "Administrator console 1 Active none 10/8/2026 8:01 AM\n";
        file.close();
    }
}

void scaffold_processes(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    fs::create_directories(root + "/C/Windows/System32");
    std::ofstream file(root + "/C/Windows/System32/tasklist.txt");
    if (file.is_open())
    {
        file << "System Idle Process,0,Services,0,8 K\n";
        file << "System,4,Services,0,136 K\n";
        file << "Registry,112,Services,0,48,210 K\n";
        file << "smss.exe,444,Services,0,1,048 K\n";
        file << "csrss.exe,568,Services,0,5,212 K\n";
        file << "wininit.exe,644,Services,0,2,104 K\n";
        file << "csrss.exe,652,Console,1,6,152 K\n";
        file << "services.exe,712,Services,0,12,416 K\n";
        file << "lsass.exe,728,Services,0,18,924 K\n";
        file << "svchost.exe,812,Services,0,21,412 K\n";
        file << "winlogon.exe,832,Console,1,7,420 K\n";
        file << "svchost.exe,912,Services,0,14,816 K\n";
        file << "svchost.exe,1014,Services,0,8,912 K\n";
        file << "fontdrvhost.exe,1056,Services,0,3,114 K\n";
        file << "fontdrvhost.exe,1064,Console,1,4,118 K\n";
        file << "Memory Compression,1108,Services,0,82,416 K\n";
        file << "svchost.exe,1204,Services,0,19,214 K\n";
        file << "spoolsv.exe,1340,Services,0,9,420 K\n";
        file << "svchost.exe,1412,Services,0,11,204 K\n";
        file << "SearchIndexer.exe,2148,Services,0,34,812 K\n";
        file << "SecurityHealthService,2210,Services,0,10,214 K\n";
        file << "userinit.exe,2468,Console,1,4,912 K\n";
        file << "explorer.exe,2496,Console,1,98,124 K\n";
        file << "smartscreen.exe,3104,Console,1,14,104 K\n";
        file << "taskhostw.exe,3120,Console,1,12,408 K\n";
        file << "OneDrive.exe,3892,Console,1,28,416 K\n";
        file << "cmd.exe,4112,Console,1,5,116 K\n";
        file << "tasklist.exe,4160,Console,1,7,824 K\n";
        file.close();
    }
}

void scaffold_security_events(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    std::ofstream file(root + "/security_events.txt");
    if (!file.is_open())
        return;

    file << "TimeCreated       : 11/4/2026 2:18:45 PM\n";
    file << "ProviderName      : Microsoft-Windows-Security-Auditing\n";
    file << "Id                : 4624\n";
    file << "LevelDisplayName  : Information\n";
    file << "Message           : An account was successfully logged on.\n\n";
    file << "                      Creator Subject:\n";
    file << "                        Security ID:            S-1-5-18\n";
    file << "                        Account Name:           DT01$\n";
    file << "                        Account Domain:         WORKGROUP\n";
    file << "                        Logon ID:               0x3E7\n\n";
    file << "                      Target Subject:\n";
    file << "                        Security ID:            S-1-0-0\n";
    file << "                        Account Name:           -\n";
    file << "                        Account Domain:         -\n";
    file << "                        Logon ID:               0x0\n\n";
    file << "                      Process Information:\n";
    file << "                        New Process ID:         0x1a4c\n";
    file << "                        New Process Name:       C:\\Windows\\System32\\winlogon.exe\n";
    file << "                        Token Elevation Type:   %%1936\n";
    file << "                        Mandatory Label:        S-1-16-16384\n";
    file << "                        Creator Process ID:     0x2f8\n";
    file << "                        Creator Process Name:   C:\\Windows\\System32\\services.exe\n";
    file << "                        Process Command Line:   C:\\Windows\\System32\\winlogon.exe\n\n";
    file << "Keywords          : {Audit Success}\n";
    file << "Task              : 13312\n";
    file << "Opcode            : Info\n";
    file << "RecordId          : 1839\n";
    file << "---\n";

    file << "TimeCreated       : 11/4/2026 2:17:58 PM\n";
    file << "ProviderName      : Microsoft-Windows-Security-Auditing\n";
    file << "Id                : 4672\n";
    file << "LevelDisplayName  : Information\n";
    file << "Message           : Special privileges assigned to new logon.\n\n";
    file << "                      Creator Subject:\n";
    file << "                        Security ID:            S-1-5-18\n";
    file << "                        Account Name:           DT01$\n";
    file << "                        Account Domain:         WORKGROUP\n";
    file << "                        Logon ID:               0x3E7\n\n";
    file << "                      Target Subject:\n";
    file << "                        Security ID:            S-1-5-21-1234567890-1234567890-1234567890-1001\n";
    file << "                        Account Name:           Administrator\n";
    file << "                        Account Domain:         DT01\n";
    file << "                        Logon ID:               0x3E7\n\n";
    file << "Keywords          : {Audit Success}\n";
    file << "Task              : 13312\n";
    file << "Opcode            : Info\n";
    file << "RecordId          : 1840\n";
    file << "---\n";

    file.close();
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
            const std::string root = machine_root(machine_name);
            fs::create_directories(root);
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
            scaffold_security_events(machine_name);
            scaffold_processes(machine_name);
            scaffold_tasks(machine_name);
        }
        else
        {
            std::cout << "usage: add machine <machine_name>\n";
        }
    }
}

void handle_remove_machine(const std::string &input)
{
    std::string prefix = "remove machine ";
    std::string alt_prefix = "delete machine ";

    std::string command = input;
    std::string target_prefix = "";

    if (input.rfind(prefix, 0) == 0)
    {
        target_prefix = prefix;
    }
    else if (input.rfind(alt_prefix, 0) == 0)
    {
        target_prefix = alt_prefix;
    }
    else
    {
        return;
    }

    std::string machine_name = input.substr(target_prefix.size());
    if (machine_name.empty())
    {
        std::cout << "usage: remote machine <machine_name>\n";
        return;
    }

    const std::string root = machine_root(machine_name);
    if (!fs::exists(root))
    {
        std::cout << "machine '" << machine_name << "' does not exist\n";
        return;
    }

    std::error_code ec;
    std::uintmax_t removed = fs::remove_all(root, ec);

    if (ec || removed == 0)
    {
        std::cout << "failed to remove machine '" << machine_name << "'\n";
        return;
    }

    std::cout << "machine '" << machine_name << "' removed\n";
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
        const std::string root = machine_root(machine_name);

        if (!fs::exists(root))
        {
            std::cout << "ssh: Could not resolve hostname " << machine_name << "\n";
            return;
        }

        std::ifstream ssh_file(root + "/ssh");

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
        handle_remove_machine(input);
        handle_login(input);
    }
}