//app_config.cpp - Defines the global app-side configuration object.
#include "mitigation_config.h"

// App-side global config, accessible by app.cpp
MitigationConfig g_app_config;

// Use a constructor attribute to ensure this runs before main()
// This initializes the global config with default values (all false).
__attribute__((constructor))
void init_app_config() {
    init_mitigation_config(&g_app_config);
}