#ifndef CLI_UTILS_H
#define CLI_UTILS_H

int is_tap_adapter(const char *component_id);
int find_tap_guid(char *tap_guid);
HANDLE open_tap_handle(void);

#endif