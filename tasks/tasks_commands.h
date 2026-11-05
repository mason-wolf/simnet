#pragma once

#include <string>

void scaffold_tasks(const std::string &machine_name);
void handle_schtasks(const std::string &machine_name, const std::string &input = "");
