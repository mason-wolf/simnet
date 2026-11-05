#include "tasks_commands.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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
    file << "OneDrive Reporting Task-S-1-5-21-1234567890-1234567890-1234567890-1001     N/A                    Ready\n";
    file << "OneDrive Standalone Update Task-S-1-5-21-1234567890-1234567890-1234567890-1001 N/A                    Ready\n";
    file << "MicrosoftEdgeUpdateTaskMachineCore       N/A                    Ready\n";
    file << "MicrosoftEdgeUpdateTaskMachineUA         N/A                    Ready\n\n";

    file << "Folder: \\Microsoft\\Windows\\AppID\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << "PolicyConverter                          N/A                    Ready\n";
    file << "VerifiedPublisherCertStoreCheck          N/A                    Ready\n\n";

    file << "Folder: \\Microsoft\\Windows\\Application Experience\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << "Microsoft Compatibility Appraiser        11/5/2026 3:00:00 AM   Ready\n";
    file << "ProgramDataUpdater                       N/A                    Ready\n";
    file << "StartupAppTask                           N/A                    Ready\n\n";

    file << "Folder: \\Microsoft\\Windows\\Customer Experience Improvement Program\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << "Consolidator                             11/5/2026 12:00:00 AM  Ready\n";
    file << "UsbCeip                                  N/A                    Ready\n\n";

    file << "Folder: \\Microsoft\\Windows\\Defrag\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << "ScheduledDefrag                          N/A                    Ready\n\n";

    file << "Folder: \\Microsoft\\Windows\\DiskDiagnostic\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << "Microsoft-Windows-DiskDiagnosticDataCollector N/A               Ready\n";
    file << "Microsoft-Windows-DiskDiagnosticResolver N/A                    Ready\n\n";

    file << "Folder: \\Microsoft\\Windows\\Maintenance\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << "WinSAT                                   N/A                    Ready\n\n";

    file << "Folder: \\Microsoft\\Windows\\WindowsUpdate\n";
    file << "TaskName                                 Next Run Time          Status\n";
    file << "======================================== ====================== ===============\n";
    file << "Scheduled Start                          N/A                    Ready\n";

    file.close();
}

void handle_schtasks(const std::string &machine_name)
{
    const std::string task_file = machine_root(machine_name) + "/tasks/schtasks.txt";
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
