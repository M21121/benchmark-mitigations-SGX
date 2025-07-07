// app/config_parser.cpp
#include "config_parser.h"
#include "mitigation_config.h"
#include <iostream>

extern MitigationConfig g_app_config;

static void set_mitigation_flag(const std::string& flag) {
    if (flag == "lfence") g_app_config.lfence_barrier = true;
    else if (flag == "mfence") g_app_config.mfence_barrier = true;
    else if (flag == "cache") g_app_config.cache_flushing = true;
    else if (flag == "constant") g_app_config.constant_time_ops = true;
    else if (flag == "memory") g_app_config.memory_barriers = true;
}

void parse_mitigations(const std::string& mitigation_str) {
    init_mitigation_config(&g_app_config);

    if (mitigation_str.empty() || mitigation_str == "none") return;
    if (mitigation_str == "all") {
        g_app_config.lfence_barrier = true;
        g_app_config.mfence_barrier = true;
        g_app_config.cache_flushing = true;
        g_app_config.constant_time_ops = true;
        g_app_config.memory_barriers = true;
        return;
    }

    std::string current_token;
    std::string remaining = mitigation_str + ",";
    size_t pos = 0;
    while ((pos = remaining.find(',')) != std::string::npos) {
        current_token = remaining.substr(0, pos);
        if (!current_token.empty()) {
            set_mitigation_flag(current_token);
        }
        remaining.erase(0, pos + 1);
    }
}

void print_config() {
    std::cout << "Current mitigation configuration:\n";
    std::cout << "  LFENCE barrier:       " << (g_app_config.lfence_barrier ? "ON" : "OFF") << "\n";
    std::cout << "  MFENCE barrier:       " << (g_app_config.mfence_barrier ? "ON" : "OFF") << "\n";
    std::cout << "  Cache flushing:       " << (g_app_config.cache_flushing ? "ON" : "OFF") << "\n";
    std::cout << "  Constant time ops:    " << (g_app_config.constant_time_ops ? "ON" : "OFF") << "\n";
    std::cout << "  Memory barriers:      " << (g_app_config.memory_barriers ? "ON" : "OFF") << "\n";
}
