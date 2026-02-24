#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "cli_utils.h"

#define ADAPTER_CLASS_KEY \
    "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}"

int is_tap_adapter(const char *component_id) {
    return strcmp(component_id, "tap0901") == 0 || 
    strcmp(component_id, "wintun") == 0 ||
    strcmp(component_id, "tap_ovpnconnect") == 0;
}

int find_tap_guid(char *tap_guid) {
    HKEY hKey;
    if (RegOpenKeyExA(
        HKEY_LOCAL_MACHINE, 
        ADAPTER_CLASS_KEY, 
        0, 
        KEY_READ, 
        &hKey) != ERROR_SUCCESS)
    {
        fprintf(stderr, "FAILED TO OPEN ADAPTER CLASS REGISTRY KEY\n");
        return 1;
    }

    char subkey_name[256];
    DWORD subkey_len;
    DWORD index = 0;

    while (1) {
        subkey_len = sizeof(subkey_name);
        if (RegEnumKeyExA(
            hKey,
            index++,
            subkey_name,
            &subkey_len,
            NULL,
            NULL,
            NULL,
            NULL) != ERROR_SUCCESS)
        {
            break;
        }

        char full_key_path[512];
        snprintf(full_key_path, sizeof(full_key_path), "%s\\%s", ADAPTER_CLASS_KEY, subkey_name);

        HKEY hSubKey;
        if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            full_key_path,
            0,
            KEY_READ,
            &hSubKey) != ERROR_SUCCESS)
        {
            continue;
        }

        char component_id[256];
        DWORD data_len = sizeof(component_id);
        DWORD type;

        if (RegQueryValueExA(
            hSubKey,
            "ComponentID",
            NULL,
            &type,
            (LPBYTE)component_id,
            &data_len) == ERROR_SUCCESS)
        {
            if (type == REG_SZ && is_tap_adapter(component_id)) {
                char guid[256];
                data_len = sizeof(guid);

                if (RegQueryValueExA(
                    hSubKey,
                    "NetCfgInstanceId",
                    NULL,
                    &type,
                    (LPBYTE)guid,
                    &data_len) == ERROR_SUCCESS)
                {
                    strcpy(tap_guid, guid);
                }
            }
        }

        RegCloseKey(hSubKey);
    }

    RegCloseKey(hKey);
    return 0;
}