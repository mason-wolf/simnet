#pragma once

#include <string>

void handle_get_security_events(const std::string &machine_name);
void handle_get_security_log_events(const std::string &machine_name);
void handle_get_security_event_filter_by_id(const std::string &machine_name, int event_id, int max_events, bool select_last_one = false);
void log_command_security_event(const std::string &machine_name, const std::string &process_name, const std::string &command_line);
