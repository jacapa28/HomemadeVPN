#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "cli_utils.h"

// the registry key for network adapters
#define ADAPTER_CLASS_KEY \
    "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}"


// checks if the ComponentId value corresponds to a TAP driver
int is_tap_adapter(const char *component_id) {
    return strcmp(component_id, "tap0901") == 0 || 
    strcmp(component_id, "wintun") == 0 ||
    strcmp(component_id, "tap_ovpnconnect") == 0;
}


// finds the TAP driver GUID and reads it to the string argument tap_guid
int find_tap_guid(char *tap_guid) {
    // opens the network adapter registry key
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

    // initializes variables to handle each network adapter subkey
    char subkey_name[256];
    DWORD subkey_len;
    DWORD index = 0;

    while (1) {
        // iterates through the network adapter subkeys until
        // there are no more to read
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

        // variable to hold the path to the subkey
        char full_key_path[512];
        snprintf(full_key_path, sizeof(full_key_path), "%s\\%s", ADAPTER_CLASS_KEY, subkey_name);

        // opens the subkey
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

        // reads the ComponentId field
        if (RegQueryValueExA(
            hSubKey,
            "ComponentID",
            NULL,
            &type,
            (LPBYTE)component_id,
            &data_len) == ERROR_SUCCESS)
        {
            // checks if ComponentId corresponds to a TAP driver
            if (type == REG_SZ && is_tap_adapter(component_id)) {
                char guid[256];
                data_len = sizeof(guid);

                // reads the GUID to the function argument
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


// opens the kernal device that is exposed by the TAP driver
HANDLE open_tap_handle(void) {
    char tap_guid[256];
    
    if (find_tap_guid(tap_guid) == 1) {
        fprintf(stderr, "FAILED TO FIND TAP GUID. COULD NOT OPEN TAP DEVICE\n");
        return NULL;
    }

    char path[256];

    snprintf(path, sizeof(path), "\\\\.\\Global\\%s.tap", tap_guid);

    return CreateFileA(
        path,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_SYSTEM,
        NULL
    );
}