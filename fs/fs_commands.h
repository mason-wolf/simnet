#pragma once
#include <string>

void handle_list_directory(const std::string &machine_name, const std::string &current_dir);
bool handle_change_directory(const std::string &machine_name, std::string &current_dir, const std::string &input);
void handle_make_directory(const std::string &machine_name, const std::string &current_dir, const std::string &input);
void handle_echo(const std::string &machine_name, const std::string &current_dir, const std::string &input);
void handle_type(const std::string &machine_name, const std::string &current_dir, const std::string &input);