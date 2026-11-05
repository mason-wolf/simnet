#pragma once
#include <string>

void handle_ipconfig(const std::string &machine_name);
void handle_getmac(const std::string &machine_name);
void handle_hostname(const std::string &machine_name);
void handle_whoami(const std::string &machine_name);
void handle_netstat(const std::string &machine_name, bool show_id);
void handle_net_user(const std::string &machine_name);
void handle_arp(const std::string &machine_name, bool show_table);
void handle_get_cim_useraccount(const std::string &machine_name);
void handle_query_user(const std::string &machine_name);
void handle_sc_query(const std::string &machine_name);
void handle_get_service(const std::string &machine_name);
void handle_net_start(const std::string &machine_name);
void handle_sc_create(const std::string &machine_name, const std::string &input);