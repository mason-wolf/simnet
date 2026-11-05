#include "net_commands.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
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

static std::string service_inventory_path(const std::string &machine_name)
{
    return machine_root(machine_name) + "/services/sc_query.txt";
}

static std::string format_service_name_for_table(const std::string &name)
{
    if (name.size() <= 18)
        return name;
    if (name.size() > 18)
        return name.substr(0, 15) + "...";
    return name;
}

struct ServiceEntry
{
    std::string name;
    std::string display_name;
    std::string type;
    std::string state;
};

static void write_default_service_inventory(const std::string &machine_name);
static void save_service_entries(const std::string &machine_name, const std::vector<ServiceEntry> &services);

static std::vector<ServiceEntry> default_services()
{
    return {
        {"Appinfo", "Application Information", "30  WIN32", "4  RUNNING"},
        {"AudioEndpointBuilder", "Windows Audio Endpoint Builder", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Audiosrv", "Windows Audio", "10  WIN32_OWN_PROCESS", "4  RUNNING"},
        {"BFE", "Base Filtering Engine", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"BITS", "Background Intelligent Transfer Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"BrokerInfrastructure", "Background Tasks Infrastructure Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"camsvc", "Capability Access Manager Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"CDPSvc", "Connected Devices Platform Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"CertPropSvc", "Certificate Propagation", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"ClipSVC", "Client License Service (ClipSVC)", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"CoreMessagingRegistrar", "CoreMessaging", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"CryptSvc", "Cryptographic Services", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"DcomLaunch", "DCOM Server Process Launcher", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"DeviceAssociationService", "Device Association Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Dhcp", "DHCP Client", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"DiagTrack", "Connected User Experiences and Telemetry", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Dnscache", "DNS Client", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"DPS", "Diagnostic Policy Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"EventLog", "Windows Event Log", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"EventSystem", "COM+ Event System", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"FontCache", "Windows Font Cache Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"gpsvc", "Group Policy Client", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"iphlpsvc", "IP Helper", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"LanmanServer", "Server", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"LanmanWorkstation", "Workstation", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"LSM", "Local Session Manager", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"mpssvc", "Windows Defender Firewall", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"NlaSvc", "Network Location Awareness", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"nsi", "Network Store Interface Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Power", "Power", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"ProfSvc", "User Profile Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"RpcEptMapper", "RPC Endpoint Mapper", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"RpcSs", "Remote Procedure Call (RPC)", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Schedule", "Task Scheduler", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"SecurityHealthService", "Windows Security Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"SENS", "System Event Notification Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"ShellHWDetection", "Shell Hardware Detection", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Spooler", "Print Spooler", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"StateRepository", "State Repository Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"StorSvc", "Storage Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"SysMain", "SysMain", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"SystemEventsBroker", "System Events Broker", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Themes", "Themes", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"UserManager", "User Manager", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"UsoSvc", "Update Orchestrator Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Wcmsvc", "Windows Connection Manager", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"WinDefend", "Microsoft Defender Antivirus Service", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"Winmgmt", "Windows Management Instrumentation", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"WlanSvc", "WLAN AutoConfig", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"WSearch", "Windows Search", "20  WIN32_SHARE_PROCESS", "4  RUNNING"},
        {"wuauserv", "Windows Update", "20  WIN32_SHARE_PROCESS", "4  RUNNING"}
    };
}

static std::vector<ServiceEntry> read_service_entries(const std::string &machine_name)
{
    std::vector<ServiceEntry> services = default_services();
    const std::string path = service_inventory_path(machine_name);
    std::ifstream file(path);
    if (!file.is_open())
        return services;

    ServiceEntry current;
    bool in_entry = false;
    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.rfind("SERVICE_NAME: ", 0) == 0)
        {
            if (in_entry && !current.name.empty())
                services.push_back(current);
            current = {};
            current.name = trim(line.substr(std::string("SERVICE_NAME: ").size()));
            in_entry = true;
            continue;
        }

        if (line.rfind("DISPLAY_NAME: ", 0) == 0)
        {
            if (in_entry)
                current.display_name = trim(line.substr(std::string("DISPLAY_NAME: ").size()));
            continue;
        }

        if (line.rfind("        TYPE               : ", 0) == 0)
        {
            if (in_entry)
                current.type = trim(line.substr(std::string("        TYPE               : ").size()));
            continue;
        }

        if (line.rfind("        STATE              : ", 0) == 0)
        {
            if (in_entry)
                current.state = trim(line.substr(std::string("        STATE              : ").size()));
            continue;
        }

        if (line.empty() && in_entry && !current.name.empty())
        {
            services.push_back(current);
            current = {};
            in_entry = false;
        }
    }

    if (in_entry && !current.name.empty())
        services.push_back(current);

    file.close();
    return services;
}

static void save_service_entries(const std::string &machine_name, const std::vector<ServiceEntry> &services)
{
    const std::string path = service_inventory_path(machine_name);
    fs::create_directories(fs::path(path).parent_path());

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open())
        return;

    for (const auto &svc : services)
    {
        file << "SERVICE_NAME: " << svc.name << "\n";
        file << "DISPLAY_NAME: " << svc.display_name << "\n";
        file << "        TYPE               : " << svc.type << "\n";
        file << "        STATE              : " << svc.state << "\n";
        file << "                                (STOPPABLE, NOT_PAUSABLE, IGNORES_SHUTDOWN)\n";
        file << "        WIN32_EXIT_CODE    : 0  (0x0)\n";
        file << "        SERVICE_EXIT_CODE  : 0  (0x0)\n";
        file << "        CHECKPOINT         : 0x0\n";
        file << "        WAIT_HINT          : 0x0\n\n";
    }

    file.close();
}

static std::string read_or_default_service_file(const std::string &machine_name)
{
    if (!fs::exists(service_inventory_path(machine_name)))
        write_default_service_inventory(machine_name);
    return service_inventory_path(machine_name);
}

static void append_service_install_event(const std::string &machine_name,
                                         const std::string &service_name,
                                         const std::string &display_name,
                                         const std::string &bin_path,
                                         const std::string &start_type)
{
    const std::string log_path = machine_root(machine_name) + "/security_events.txt";
    std::ofstream file(log_path, std::ios::app);
    if (!file.is_open())
        return;

    file << "TimeCreated       : " << current_time_string() << "\n";
    file << "ProviderName      : Service Control Manager\n";
    file << "Id                : 7045\n";
    file << "LevelDisplayName  : Information\n";
    file << "Message           : A service was installed in the system.\n\n";
    file << "                      Service Name: " << service_name << "\n";
    file << "                      Service File Name: " << bin_path << "\n";
    file << "                      Service Display Name: " << display_name << "\n";
    file << "                      Service Start Type: " << start_type << "\n";
    file << "                      Service Account: LocalSystem\n\n";
    file << "Keywords          : {Audit Success}\n";
    file << "Task              : 0\n";
    file << "Opcode            : Info\n";
    file << "RecordId          : 3010\n";
    file << "---\n";
    file.close();
}

static bool contains_service(const std::vector<ServiceEntry> &services, const std::string &service_name)
{
    for (const auto &svc : services)
    {
        if (to_lower(svc.name) == to_lower(service_name))
            return true;
    }
    return false;
}

static void create_registry_service_entry(const std::string &machine_name, const std::string &service_name)
{
    const std::string root = machine_root(machine_name);
    fs::create_directories(root + "/registry/HKLM/SYSTEM/CurrentControlSet/Services/" + service_name);
}

static std::string parse_service_display_name(const std::string &machine_name, const std::string &service_name)
{
    (void)machine_name;
    return service_name;
}

static void add_service_entry(const std::string &machine_name,
                              const std::string &service_name,
                              const std::string &display_name,
                              const std::string &bin_path,
                              const std::string &start_type)
{
    std::vector<ServiceEntry> services = read_service_entries(machine_name);
    if (contains_service(services, service_name))
        return;

    services.push_back({service_name, display_name.empty() ? parse_service_display_name(machine_name, service_name) : display_name,
        "20  WIN32_SHARE_PROCESS", "4  RUNNING"});
    save_service_entries(machine_name, services);
    create_registry_service_entry(machine_name, service_name);
    append_service_install_event(machine_name, service_name, display_name.empty() ? service_name : display_name, bin_path, start_type.empty() ? "auto" : start_type);
}

static void write_default_service_inventory(const std::string &machine_name)
{
    const std::string path = service_inventory_path(machine_name);
    fs::create_directories(fs::path(path).parent_path());

    std::ofstream file(path);
    if (!file.is_open())
        return;

    const auto services = default_services();
    for (const auto &svc : services)
    {
        file << "SERVICE_NAME: " << svc.name << "\n";
        file << "DISPLAY_NAME: " << svc.display_name << "\n";
        file << "        TYPE               : " << svc.type << "\n";
        file << "        STATE              : " << svc.state << "\n";
        file << "                                (STOPPABLE, NOT_PAUSABLE, IGNORES_SHUTDOWN)\n";
        file << "        WIN32_EXIT_CODE    : 0  (0x0)\n";
        file << "        SERVICE_EXIT_CODE  : 0  (0x0)\n";
        file << "        CHECKPOINT         : 0x0\n";
        file << "        WAIT_HINT          : 0x0\n\n";
    }

    file.close();
}

static std::string normalize_service_name(const std::string &value)
{
    std::string cleaned = trim(value);
    if (cleaned.size() >= 2 && cleaned.front() == '"' && cleaned.back() == '"')
        cleaned = cleaned.substr(1, cleaned.size() - 2);
    return cleaned;
}

static std::string normalize_assignment_value(const std::string &value)
{
    std::string result = trim(value);
    if (result.size() >= 2 && result.front() == '"' && result.back() == '"')
    {
        result = result.substr(1, result.size() - 2);
    }
    return result;
}

void handle_sc_create(const std::string &machine_name, const std::string &input)
{
    if (input.rfind("sc create", 0) != 0 && input.rfind("sc  create", 0) != 0)
        return;

    std::string args = input.substr(std::string("sc create").size());
    std::vector<std::string> tokens = split_tokens(args);
    if (tokens.empty())
    {
        std::cout << "The command failed to complete successfully.\n";
        return;
    }

    std::string service_name;
    std::string bin_path;
    std::string display_name;
    std::string start_type = "auto";

    for (const auto &token : tokens)
    {
        const std::string original = trim(token);
        if (original.empty())
            continue;

        const size_t eq = original.find('=');
        if (eq == std::string::npos)
        {
            if (service_name.empty())
                service_name = normalize_service_name(original);
            continue;
        }

        const std::string key = to_lower(trim(original.substr(0, eq)));
        const std::string value = normalize_assignment_value(original.substr(eq + 1));

        if (key == "binpath")
            bin_path = value;
        else if (key == "displayname")
            display_name = value;
        else if (key == "start")
            start_type = value.empty() ? "auto" : value;
        else if (service_name.empty())
            service_name = normalize_service_name(original);
    }

    if (service_name.empty() || bin_path.empty())
    {
        std::cout << "The command failed to complete successfully.\n";
        return;
    }

    if (display_name.empty())
        display_name = service_name;

    add_service_entry(machine_name, service_name, display_name, bin_path, start_type);
    std::cout << "[SC] CreateService SUCCESS\n";
}

void handle_ipconfig(const std::string &machine_name)
{
    std::ifstream file(machine_root(machine_name) + "/ipconfig.txt");

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
    std::ifstream file(machine_root(machine_name) + "/mac.txt");
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

void handle_hostname(const std::string &machine_name)
{
    std::cout << machine_name << "\n";
}

void handle_whoami(const std::string &machine_name)
{
    std::ifstream file(machine_root(machine_name) + "/ssh");

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
    std::ifstream file(machine_root(machine_name) + "/netstat.txt");
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
    std::ifstream file(machine_root(machine_name) + "/users");
    if (!file.is_open())
    {
        std::cout << "The command failed to complete successfully.\n";
        return;
    }

    std::cout << "\nUser accounts for \\" << machine_name << "\n\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    std::string user;
    int count = 0;
    while (std::getline(file, user))
    {
        if (!user.empty() && user.back() == '\r')
            user.pop_back();
        if (user.empty())
            continue;

        std::cout << std::left << std::setw(25) << user;
        ++count;
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

    std::ifstream file(machine_root(machine_name) + "/arp.txt");
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
    std::ifstream file(machine_root(machine_name) + "/users");
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
        std::cout << "Status            : OK\n\n";
    }

    file.close();
}

void handle_query_user(const std::string &machine_name)
{
    std::ifstream file(machine_root(machine_name) + "/sessions");
    if (!file.is_open())
    {
        std::cout << "ERROR: Unable to retrieve session information.\n";
        return;
    }

    std::cout << "\n USERNAME              SESSIONNAME        ID  STATE   IDLE TIME  LOGON TIME\n";

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        std::cout << " " << line << "\n";
    }
    file.close();
}

void handle_sc_query(const std::string &machine_name)
{
    if (!fs::exists(service_inventory_path(machine_name)))
        write_default_service_inventory(machine_name);

    std::ifstream file(service_inventory_path(machine_name));
    if (!file.is_open())
    {
        std::cout << "The specified service does not exist as an installed service.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        std::cout << line << "\n";
    }

    file.close();
}

void handle_get_service(const std::string &machine_name)
{
    std::vector<ServiceEntry> services = read_service_entries(machine_name);
    std::cout << "Status   Name               DisplayName\n";
    std::cout << "------   ----               -----------\n";

    for (const auto &svc : services)
    {
        std::cout << std::left << std::setw(7) << "Running"
                  << std::setw(18) << format_service_name_for_table(svc.name)
                  << svc.display_name << "\n";
    }
}

void handle_net_start(const std::string &machine_name)
{
    std::vector<ServiceEntry> services = read_service_entries(machine_name);
    std::cout << "These Windows services are started:\n\n";

    for (const auto &svc : services)
    {
        std::cout << "   " << svc.display_name << "\n";
    }

    std::cout << "\nThe command completed successfully.\n";
}