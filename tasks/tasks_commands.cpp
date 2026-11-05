#include "tasks_commands.h"

#include <cctype>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

static std::string machine_root(const std::string &machine_name)
{
    return "devices/" + machine_name;
}

static std::string trim(const std::string &value)
{
    const std::string whitespace = " \t\r\n";
    const size_t start = value.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return "";

    const size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

static std::string to_lower(std::string value)
{
    for (char &ch : value)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return value;
}

static std::vector<std::string> split_tokens(const std::string &input)
{
    std::vector<std::string> tokens;
    std::string current;
    bool in_quote = false;
    char quote_char = '\0';

    for (char ch : input)
    {
        if (ch == '"' || ch == '\'')
        {
            if (!in_quote)
            {
                in_quote = true;
                quote_char = ch;
            }
            else if (ch == quote_char)
            {
                in_quote = false;
                quote_char = '\0';
            }
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) && !in_quote)
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

static std::string current_time_string()
{
    time_t now = time(nullptr);
    tm local = {};
    localtime_r(&now, &local);

    int hour = local.tm_hour;
    std::string suffix = (hour >= 12) ? "PM" : "AM";
    if (hour > 12) hour -= 12;
    if (hour == 0) hour = 12;

    std::ostringstream ss;
    ss << (local.tm_mon + 1) << "/" << local.tm_mday << "/" << (local.tm_year + 1900)
       << " " << hour << ":" << std::setw(2) << std::setfill('0') << local.tm_min
       << ":" << std::setw(2) << std::setfill('0') << local.tm_sec << " " << suffix;
    return ss.str();
}

static std::string task_file_path(const std::string &machine_name)
{
    return machine_root(machine_name) + "/tasks/schtasks.txt";
}

static std::string format_task_name(const std::string &name)
{
    const size_t width = 39;
    std::string out = name;
    if (out.size() > width)
    {
        out = out.substr(0, width - 3) + "...";
    }
    return out;
}

static std::string format_task_line(const std::string &task_name, const std::string &next_run = "N/A", const std::string &status = "Ready")
{
    std::ostringstream line;
    line << std::left << std::setw(39) << format_task_name(task_name)
         << std::setw(22) << next_run
         << status;
    return line.str();
}

static bool task_exists_in_file(const std::string &machine_name, const std::string &task_name)
{
    const std::string path = task_file_path(machine_name);
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line.rfind("Folder:", 0) == 0 || line.find("TaskName") == 0 || line.find("====") == 0)
            continue;

        std::string candidate = trim(line.substr(0, 39));
        if (candidate == task_name)
            return true;
    }

    return false;
}

static void append_task_event_log(const std::string &machine_name, const std::string &task_name, const std::string &command)
{
    const std::string log_path = machine_root(machine_name) + "/security_events.txt";
    std::ofstream file(log_path, std::ios::app);
    if (!file.is_open())
        return;

    file << "TimeCreated       : " << current_time_string() << "\n";
    file << "ProviderName      : Microsoft-Windows-Security-Auditing\n";
    file << "Id                : 4698\n";
    file << "LevelDisplayName  : Information\n";
    file << "Message           : A scheduled task was created.\n\n";
    file << "                      TaskName: " << task_name << "\n";
    file << "                      TaskContent: " << command << "\n\n";
    file << "Keywords          : {Audit Success}\n";
    file << "Task              : 12804\n";
    file << "Opcode            : Info\n";
    file << "RecordId          : 2000\n";
    file << "---\n";
    file.close();
}

static void append_new_task_to_file(const std::string &machine_name, const std::string &task_name, const std::string &command)
{
    const std::string path = task_file_path(machine_name);
    std::ofstream file(path, std::ios::app);
    if (!file.is_open())
        return;

    file << format_task_line(task_name, "N/A", "Ready") << "\n";
    file.close();
    (void)command;
}

void scaffold_tasks(const std::string &machine_name)
{
    const std::string root = machine_root(machine_name);
    fs::create_directories(root + "/tasks");

    std::ofstream file(root + "/tasks/schtasks.txt");
    if (!file.is_open())
        return;

    file << "Folder: \\\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("OneDrive Reporting Task-S-1-5-21-1234567890-1234567890-1234567890-1001") << "\n";
    file << format_task_line("OneDrive Standalone Update Task-S-1-5-21-1234567890-1234567890-1234567890-1001") << "\n";
    file << format_task_line("MicrosoftEdgeUpdateTaskMachineCore") << "\n";
    file << format_task_line("MicrosoftEdgeUpdateTaskMachineUA") << "\n\n";

    file << "Folder: \\Microsoft\\Windows\\AppID\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("PolicyConverter") << "\n";
    file << format_task_line("VerifiedPublisherCertStoreCheck") << "\n\n";

    file << "Folder: \\Microsoft\\Windows\\Application Experience\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("Microsoft Compatibility Appraiser", "11/5/2026 3:00:00 AM") << "\n";
    file << format_task_line("ProgramDataUpdater") << "\n";
    file << format_task_line("StartupAppTask") << "\n\n";

    file << "Folder: \\Microsoft\\Windows\\Customer Experience Improvement Program\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("Consolidator", "11/5/2026 12:00:00 AM") << "\n";
    file << format_task_line("UsbCeip") << "\n\n";

    file << "Folder: \\Microsoft\\Windows\\Defrag\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("ScheduledDefrag") << "\n\n";

    file << "Folder: \\Microsoft\\Windows\\DiskDiagnostic\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("Microsoft-Windows-DiskDiagnosticDataCollector") << "\n";
    file << format_task_line("Microsoft-Windows-DiskDiagnosticResolver") << "\n\n";

    file << "Folder: \\Microsoft\\Windows\\Maintenance\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("WinSAT") << "\n\n";

    file << "Folder: \\Microsoft\\Windows\\WindowsUpdate\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << format_task_line("Scheduled Start") << "\n";

    file.close();
}

static std::string extract_process_name(const std::string &command_line)
{
    std::string cleaned = command_line;
    if (cleaned.size() >= 2 && cleaned.front() == '"' && cleaned.back() == '"')
    {
        cleaned = cleaned.substr(1, cleaned.size() - 2);
    }

    size_t slash = cleaned.find_last_of("\\/");
    if (slash == std::string::npos)
        return cleaned;

    std::string name = cleaned.substr(slash + 1);
    size_t space = name.find(' ');
    if (space != std::string::npos)
        name = name.substr(0, space);
    return name;
}

static void append_process_event_log(const std::string &machine_name, const std::string &command_line)
{
    const std::string log_path = machine_root(machine_name) + "/security_events.txt";
    std::ofstream file(log_path, std::ios::app);
    if (!file.is_open())
        return;

    std::string process_name = extract_process_name(command_line);

    file << "TimeCreated       : " << current_time_string() << "\n";
    file << "ProviderName      : Microsoft-Windows-Security-Auditing\n";
    file << "Id                : 4688\n";
    file << "LevelDisplayName  : Information\n";
    file << "Message           : A new process has been created.\n\n";
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
    file << "                        New Process Name:       C:\\Windows\\System32\\" << process_name << "\n";
    file << "                        Token Elevation Type:   %%1936\n";
    file << "                        Mandatory Label:        S-1-16-16384\n";
    file << "                        Creator Process ID:     0x2f8\n";
    file << "                        Creator Process Name:   C:\\Windows\\System32\\cmd.exe\n";
    file << "                        Process Command Line:   " << command_line << "\n\n";
    file << "Keywords          : {Audit Success}\n";
    file << "Task              : 13312\n";
    file << "Opcode            : Info\n";
    file << "RecordId          : 2001\n";
    file << "---\n";
    file.close();
}

static std::vector<std::pair<std::string, std::string>> load_task_store(const std::string &machine_name)
{
    const std::string path = task_file_path(machine_name);
    std::ifstream file(path);
    std::vector<std::pair<std::string, std::string>> tasks;
    if (!file.is_open())
        return tasks;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line.rfind("Folder:", 0) == 0 || line.find("TaskName") == 0 || line.find("====") == 0)
            continue;

        std::string trimmed = trim(line);
        if (trimmed.empty())
            continue;

        size_t first_space = trimmed.find(' ');
        if (first_space == std::string::npos)
            continue;

        std::string task_name = trimmed.substr(0, first_space);
        tasks.push_back({task_name, ""});
    }

    file.close();
    return tasks;
}

static std::string find_task_command(const std::string &machine_name, const std::string &task_name)
{
    const std::string path = task_file_path(machine_name);
    std::ifstream file(path);
    if (!file.is_open())
        return "";

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line.rfind("Folder:", 0) == 0 || line.find("TaskName") == 0 || line.find("====") == 0)
            continue;

        std::string trimmed = trim(line);
        size_t first_space = trimmed.find(' ');
        if (first_space == std::string::npos)
            continue;

        std::string candidate = trimmed.substr(0, first_space);
        if (candidate == task_name)
            return "C:\\Windows\\System32\\cmd.exe /c calc.exe";
    }

    return "";
}

static std::string get_task_name_from_tokens(const std::vector<std::string> &tokens)
{
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string token = tokens[i];
        const std::string lower = to_lower(token);
        if ((lower == "/tn" || lower == "/tn") && i + 1 < tokens.size())
            return tokens[i + 1];
    }
    return "";
}

static std::string get_task_command_from_tokens(const std::vector<std::string> &tokens)
{
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string token = tokens[i];
        const std::string lower = to_lower(token);
        if ((lower == "/tr") && i + 1 < tokens.size())
            return tokens[i + 1];
    }
    return "";
}

static std::string get_task_name_from_run_tokens(const std::vector<std::string> &tokens)
{
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string token = tokens[i];
        const std::string lower = to_lower(token);
        if (lower == "/tn" && i + 1 < tokens.size())
            return tokens[i + 1];
    }
    return "";
}

void handle_schtasks(const std::string &machine_name, const std::string &input)
{
    const std::string task_file = task_file_path(machine_name);
    const std::string cmd = input.empty() ? "schtasks" : input;
    const std::vector<std::string> tokens = split_tokens(cmd);

    if (!tokens.empty())
    {
        bool is_create = false;
        bool is_run = false;
        for (const std::string &token : tokens)
        {
            const std::string lower = to_lower(token);
            if (lower == "/create")
                is_create = true;
            if (lower == "/run")
                is_run = true;
        }

        if (is_create)
        {
            std::string task_name;
            std::string command_line;

            for (size_t i = 0; i < tokens.size(); ++i)
            {
                const std::string token = tokens[i];
                const std::string lowered = to_lower(token);
                if (lowered == "/tn" && i + 1 < tokens.size())
                {
                    task_name = tokens[++i];
                }
                else if (lowered == "/tr" && i + 1 < tokens.size())
                {
                    command_line = tokens[++i];
                    while (i + 1 < tokens.size() && tokens[i + 1] != "/sc" && tokens[i + 1] != "/tn" && tokens[i + 1] != "/tr")
                    {
                        command_line += " " + tokens[++i];
                    }
                }
            }

            if (task_name.empty() || command_line.empty())
            {
                std::cout << "ERROR: The parameter is incorrect.\n";
                return;
            }

            if (task_exists_in_file(machine_name, task_name))
            {
                std::cout << "ERROR: Task already exists.\n";
                return;
            }

            append_new_task_to_file(machine_name, task_name, command_line);
            append_task_event_log(machine_name, task_name, command_line);

            std::cout << "SUCCESS: Scheduled task created.\n";
            return;
        }

        if (is_run)
        {
            std::string task_name = get_task_name_from_run_tokens(tokens);
            if (task_name.empty())
            {
                std::cout << "ERROR: The parameter is incorrect.\n";
                return;
            }

            std::string command_line = find_task_command(machine_name, task_name);
            if (command_line.empty())
            {
                std::cout << "ERROR: Task not found.\n";
                return;
            }

            append_process_event_log(machine_name, command_line);
            std::cout << "SUCCESS: Scheduled task executed.\n";
            return;
        }
    }

    std::ifstream file(task_file);
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve task information.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::cout << line << "\n";
    }

    file.close();
}
