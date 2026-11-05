#include "handle_get_security_events.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct EventRecord {
    std::string time_created;
    int id = 0;
    std::string level;
    std::string message;
};

struct ProviderRecord {
    std::string provider_name;
    std::vector<EventRecord> events;
};

std::vector<ProviderRecord> build_default_system_events()
{
    return {
        {"Microsoft-Windows-Kernel-General", {
            {"11/4/2026 2:18:41 PM", 16, "Information", "The access history in hive \\??\\C:\\Windows\\System32\\config\\SYSTEM..."},
            {"11/4/2026 2:18:41 PM", 16, "Information", "The access history in hive \\??\\C:\\Windows\\System32\\config\\SOFTWARE..."},
            {"11/4/2026 2:18:40 PM", 16, "Information", "The access history in hive \\??\\C:\\Windows\\System32\\config\\SAM..."}
        }},
        {"Microsoft-Windows-Kernel-Power", {
            {"11/4/2026 2:18:12 PM", 109, "Information", "The kernel power manager has initiated a shutdown transition."},
            {"11/4/2026 2:17:58 PM", 107, "Information", "The system has resumed from sleep."}
        }},
        {"Service Control Manager", {
            {"11/4/2026 2:17:45 PM", 7036, "Information", "The Windows Update service entered the running state."},
            {"11/4/2026 2:17:44 PM", 7040, "Information", "The start type of the Background Intelligent Transfer Service service was changed from demand start to auto start."},
            {"11/4/2026 2:17:12 PM", 7036, "Information", "The DHCP Client service entered the running state."},
            {"11/4/2026 2:16:58 PM", 7036, "Information", "The Windows Defender Antivirus Service service entered the running state."}
        }},
        {"Microsoft-Windows-Winlogon", {
            {"11/4/2026 2:16:33 PM", 7001, "Information", "User Logon Notification for Customer Experience Improvement Program"}
        }},
        {"Microsoft-Windows-WindowsUpdateClient", {
            {"11/4/2026 2:15:02 PM", 19, "Information", "Installation Successful: Windows successfully installed the following update: Security Intelligence Update for Microsoft Defender Antivirus - KB2267602"},
            {"11/4/2026 2:14:41 PM", 43, "Information", "Installation Started: Windows has started installing the following update: Security Intelligence Update for Microsoft Defender Antivirus - KB2267602"}
        }},
        {"Microsoft-Windows-FilterManager", {
            {"11/4/2026 2:14:12 PM", 1, "Information", "File System Filter 'FileInfo' (Version 10.0, ‎2024‎-‎01‎-‎01T00:00:00.000000000Z) unloaded successfully."},
            {"11/4/2026 2:14:11 PM", 6, "Information", "File System Filter 'FileInfo' (Version 10.0, ‎2024‎-‎01‎-‎01T00:00:00.000000000Z) has successfully loaded and registered with Filter Manager."}
        }}
    };
}

std::vector<ProviderRecord> build_default_security_events()
{
    return {
        {"Microsoft-Windows-Security-Auditing", {
            {"11/4/2026 2:18:45 PM", 4624, "Information", "An account was successfully logged on."},
            {"11/4/2026 2:18:45 PM", 4672, "Information", "Special privileges assigned to new logon."},
            {"11/4/2026 2:18:44 PM", 4624, "Information", "An account was successfully logged on."},
            {"11/4/2026 2:18:12 PM", 4634, "Information", "An account was logged off."},
            {"11/4/2026 2:17:58 PM", 4624, "Information", "An account was successfully logged on."},
            {"11/4/2026 2:17:58 PM", 4672, "Information", "Special privileges assigned to new logon."},
            {"11/4/2026 2:17:12 PM", 4648, "Information", "A logon was attempted using explicit credentials."},
            {"11/4/2026 2:16:33 PM", 4624, "Information", "An account was successfully logged on."},
            {"11/4/2026 2:16:33 PM", 4672, "Information", "Special privileges assigned to new logon."},
            {"11/4/2026 2:15:02 PM", 4688, "Information", "A new process has been created."},
            {"11/4/2026 2:14:41 PM", 4689, "Information", "A process has exited."},
            {"11/4/2026 2:14:12 PM", 4624, "Information", "An account was successfully logged on."},
            {"11/4/2026 2:14:12 PM", 4672, "Information", "Special privileges assigned to new logon."},
            {"11/4/2026 2:13:55 PM", 4798, "Information", "A user's local group membership was enumerated."},
            {"11/4/2026 2:13:40 PM", 4907, "Information", "Auditing settings on object were changed."},
            {"11/4/2026 2:12:18 PM", 4625, "Information", "An account failed to log on."},
            {"11/4/2026 2:11:02 PM", 4624, "Information", "An account was successfully logged on."},
            {"11/4/2026 2:11:02 PM", 4672, "Information", "Special privileges assigned to new logon."},
            {"11/4/2026 2:10:45 PM", 4688, "Information", "A new process has been created."},
            {"11/4/2026 2:10:33 PM", 4634, "Information", "An account was logged off."}
        }}
    };
}

std::string current_windows_time_string()
{
    time_t now = time(nullptr);
    tm local = {};
    localtime_r(&now, &local);

    int hour = local.tm_hour;
    std::string suffix = (hour >= 12) ? "PM" : "AM";
    if (hour > 12)
        hour -= 12;
    if (hour == 0)
        hour = 12;

    std::ostringstream ss;
    ss << (local.tm_mon + 1) << "/" << local.tm_mday << "/" << (local.tm_year + 1900)
       << " " << hour << ":" << std::setw(2) << std::setfill('0') << local.tm_min
       << ":" << std::setw(2) << std::setfill('0') << local.tm_sec << " " << suffix;
    return ss.str();
}

std::string security_log_path(const std::string &machine_name)
{
    return "devices/" + machine_name + "/security_events.txt";
}

std::vector<std::string> split_event_blocks(const std::string &content)
{
    std::vector<std::string> blocks;
    std::stringstream ss(content);
    std::string line;
    std::string current;

    while (std::getline(ss, line))
    {
        if (line == "---")
        {
            if (!current.empty())
            {
                blocks.push_back(current);
                current.clear();
            }
            continue;
        }

        current += line;
        current += "\n";
    }

    if (!current.empty())
    {
        blocks.push_back(current);
    }

    return blocks;
}

std::string extract_log_value(const std::string &block, const std::string &label)
{
    const size_t start = block.find(label);
    if (start == std::string::npos)
        return "";

    const size_t value_start = start + label.size();
    const size_t value_end = block.find('\n', value_start);
    if (value_end == std::string::npos)
        return block.substr(value_start);

    return block.substr(value_start, value_end - value_start);
}

std::string build_process_event_record(const std::string &process_name, const std::string &command_line)
{
    std::ostringstream event;
    event << "TimeCreated       : " << current_windows_time_string() << "\n";
    event << "ProviderName      : Microsoft-Windows-Security-Auditing\n";
    event << "Id                : 4688\n";
    event << "LevelDisplayName  : Information\n";
    event << "Message           : A new process has been created.\n\n";
    event << "                      Creator Subject:\n";
    event << "                        Security ID:            S-1-5-18\n";
    event << "                        Account Name:           DT01$\n";
    event << "                        Account Domain:         WORKGROUP\n";
    event << "                        Logon ID:               0x3E7\n\n";
    event << "                      Target Subject:\n";
    event << "                        Security ID:            S-1-0-0\n";
    event << "                        Account Name:           -\n";
    event << "                        Account Domain:         -\n";
    event << "                        Logon ID:               0x0\n\n";
    event << "                      Process Information:\n";
    event << "                        New Process ID:         0x1a4c\n";
    event << "                        New Process Name:       C:\\Windows\\System32\\" << process_name << "\n";
    event << "                        Token Elevation Type:   %%1936\n";
    event << "                        Mandatory Label:        S-1-16-16384\n";
    event << "                        Creator Process ID:     0x2f8\n";
    event << "                        Creator Process Name:   C:\\Windows\\System32\\cmd.exe\n";
    event << "                        Process Command Line:   " << command_line << "\n\n";
    event << "                      Token Elevation Type indicates the type of token that was assigned to the new process in accordance with User Account Control policy.\n\n";
    event << "                      Type 1 is a full token with no privileges removed or groups disabled.  A full token is only used if User Account Control is disabled or if the user is the built-in Administrator account or a service account.\n\n";
    event << "                      Type 2 is an elevated token with no privileges removed or groups disabled.  An elevated token is used when User Account Control is enabled and the user chooses to start the program using Run as administrator.  An elevated token is also used when an application is configured to always require administrative privilege or to always require maximum privilege, and the user is a member of the Administrators group.\n\n";
    event << "                      Type 3 is a limited token with administrative privileges removed and administrative groups disabled.  The limited token is used when User Account Control is enabled, the application does not require administrative privilege, and the user does not choose to start the program using Run as administrator.\n";
    event << "Keywords          : {Audit Success}\n";
    event << "Task              : 13312\n";
    event << "Opcode            : Info\n";
    event << "RecordId          : 1842\n";
    event << "---\n";
    return event.str();
}

struct LoggedSecurityEvent {
    std::string time_created;
    int id = 0;
    std::string level;
    std::string message;
};

std::vector<LoggedSecurityEvent> read_security_log_events(const std::string &machine_name)
{
    const std::string path = security_log_path(machine_name);
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::vector<LoggedSecurityEvent> events;
        for (const auto &provider : build_default_security_events())
        {
            for (const auto &record : provider.events)
            {
                events.push_back({record.time_created, record.id, record.level, record.message});
            }
        }
        return events;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::vector<LoggedSecurityEvent> events;
    std::vector<std::string> blocks = split_event_blocks(buffer.str());
    for (auto it = blocks.rbegin(); it != blocks.rend(); ++it)
    {
        const std::string &block = *it;
        const std::string time_created = extract_log_value(block, "TimeCreated       : ");
        const std::string raw_id = extract_log_value(block, "Id                : ");
        const std::string level = extract_log_value(block, "LevelDisplayName  : ");
        const std::string message = extract_log_value(block, "Message           : ");

        if (time_created.empty() || raw_id.empty())
            continue;

        try
        {
            events.push_back({time_created, std::stoi(raw_id), level, message});
        }
        catch (...) {}
    }

    if (!events.empty())
        return events;

    std::vector<LoggedSecurityEvent> fallback;
    for (const auto &provider : build_default_security_events())
    {
        for (const auto &record : provider.events)
        {
            fallback.push_back({record.time_created, record.id, record.level, record.message});
        }
    }
    return fallback;
}

void log_command_security_event(const std::string &machine_name, const std::string &process_name, const std::string &command_line)
{
    const std::string path = security_log_path(machine_name);
    std::ofstream file(path, std::ios::app);
    if (!file.is_open())
        return;

    file << build_process_event_record(process_name, command_line);
    file.close();
}

void handle_get_security_event_filter_by_id(const std::string &machine_name, int event_id, int max_events)
{
    const std::string path = security_log_path(machine_name);
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "No events were found to match the specified selection.\n";
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    const auto blocks = split_event_blocks(buffer.str());
    std::vector<std::string> matches;

    for (const auto &block : blocks)
    {
        if (block.find("Id                : " + std::to_string(event_id)) != std::string::npos)
        {
            matches.push_back(block);
        }
    }

    if (matches.empty())
    {
        std::cout << "No events were found to match the specified selection.\n";
        return;
    }

    int start_index = static_cast<int>(matches.size()) - std::min(static_cast<int>(matches.size()), max_events);
    if (start_index < 0)
        start_index = 0;

    std::vector<std::string> limited_matches;
    for (int i = start_index; i < static_cast<int>(matches.size()); ++i)
    {
        limited_matches.push_back(matches[i]);
    }

    for (const auto &block : limited_matches)
    {
        std::cout << block;
    }
}

void handle_get_security_event_filter_by_id(const std::string &machine_name, int event_id, int max_events, bool select_last_one)
{
    const std::string path = security_log_path(machine_name);
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "No events were found to match the specified selection.\n";
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    const auto blocks = split_event_blocks(buffer.str());
    std::vector<std::string> matches;

    for (const auto &block : blocks)
    {
        if (block.find("Id                : " + std::to_string(event_id)) != std::string::npos)
        {
            matches.push_back(block);
        }
    }

    if (matches.empty())
    {
        std::cout << "No events were found to match the specified selection.\n";
        return;
    }

    std::vector<std::string> output_matches = matches;
    if (select_last_one)
    {
        if (matches.size() > 1)
        {
            output_matches = {matches.back()};
        }
    }
    else if (max_events > 0)
    {
        int start_index = static_cast<int>(matches.size()) - std::min(static_cast<int>(matches.size()), max_events);
        if (start_index < 0)
            start_index = 0;

        output_matches.clear();
        for (int i = start_index; i < static_cast<int>(matches.size()); ++i)
        {
            output_matches.push_back(matches[i]);
        }
    }

    for (const auto &block : output_matches)
    {
        std::cout << block;
    }
}

void handle_get_security_events(const std::string &machine_name)
{
    (void)machine_name;

    const auto providers = build_default_system_events();
    for (const auto &provider : providers)
    {
        std::cout << "\n   ProviderName: " << provider.provider_name << "\n\n";
        std::cout << "TimeCreated                      Id LevelDisplayName Message\n";
        std::cout << "-----------                      -- ---------------- -------\n";

        for (const auto &event : provider.events)
        {
            std::cout << std::left << std::setw(32) << event.time_created
                      << std::right << std::setw(6) << event.id
                      << ' ' << std::left << std::setw(18) << event.level
                      << event.message << "\n";
        }
        std::cout << "\n";
    }
}

void handle_get_security_log_events(const std::string &machine_name)
{
    const auto events = read_security_log_events(machine_name);

    std::cout << "\n   ProviderName: Microsoft-Windows-Security-Auditing\n\n";
    std::cout << "TimeCreated                      Id LevelDisplayName Message\n";
    std::cout << "-----------                      -- ---------------- -------\n";

    for (const auto &event : events)
    {
        std::cout << std::left << std::setw(32) << event.time_created
                  << std::right << std::setw(6) << event.id
                  << ' ' << std::left << std::setw(18) << event.level
                  << event.message << "\n";
    }
    std::cout << "\n";
}
