#include <curlwrapper.h>

int is_blacklisted_mm(const char* ip, const char* app_key, const char* endpoint, const char* ipblc_id);
int register_machine(const char* app_key, const char* endpoint, char ipbl_c[MAX_BODY]);
