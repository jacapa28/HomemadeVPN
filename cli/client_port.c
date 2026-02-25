#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "cli_utils.h"

// ethernet frame length
#define ETHERNET_MAX_LEN 1518

// brings up the tap device after opening it
#define TAP_IOCTL_SET_MEDIA_STATUS CTL_CODE(FILE_DEVICE_UNKNOWN, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)


// hold data necessary for connection to server
struct virtual_port {
    HANDLE tap_handle;
    SOCKET port_socket;
    struct sockaddr_in switch_address;
};


void virtual_port_init(struct virtual_port *vport, const char *ip, int port);
DWORD WINAPI forward_traffic_to_switch(LPVOID arg);
int read_frame_from_tap(HANDLE tap, char *buffer, int buffer_size);
DWORD WINAPI forward_traffic_to_tap(LPVOID arg);
int write_frame_to_tap(HANDLE tap, char *buffer, int buffer_size);



int main(int argc, char **argv) {
    // check for valid argument length
    if (argc != 3) {
        fprintf(stderr, "INVALID ARGUMENTS. COMMAND USAGE:  virtual_port <SERVER_IP> <SERVER_PORT>\n");
        return 1;
    }

    // startup winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WINSOCK FAILED TO INITIALIZE\n");
        return 1;
    }

    struct virtual_port vport;
    virtual_port_init(&vport, argv[1], atoi(argv[2]));


    HANDLE send_frames = CreateThread(NULL, 0, forward_traffic_to_switch, &vport, 0, NULL);
    HANDLE receive_frames = CreateThread(NULL, 0, forward_traffic_to_tap, &vport, 0, NULL);

    WaitForSingleObject(send_frames, INFINITE);
    WaitForSingleObject(receive_frames, INFINITE);

    WSACleanup();
    return 0;
}


void virtual_port_init(struct virtual_port *vport, const char *ip, int port) {
    vport->tap_handle = open_tap_handle();
    if (vport->tap_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FAILED TO OPEN TAP DEVICE\n");
        ExitProcess(1);
    }

    ULONG status = 1;
    DWORD len;
    if (!DeviceIoControl(
        vport->tap_handle,
        TAP_IOCTL_SET_MEDIA_STATUS,
        &status,
        sizeof(status),
        NULL,
        0,
        &len,
        NULL))
    {
        DWORD err = GetLastError();
        fprintf(stderr, "FAILED TO SET TAP MEDIA STATUS: %lu\n", err);
        ExitProcess(1);
    }

    vport->port_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (vport->port_socket == INVALID_SOCKET) {
        fprintf(stderr, "FAILED TO OPEN SOCKET: %d\n", WSAGetLastError());
        ExitProcess(1);
    }

    // sets the family to IPv4
    vport->switch_address.sin_family = AF_INET;

    // sets the port and ip using big-endian for networking standards
    vport->switch_address.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &vport->switch_address.sin_addr) != 1) {
        fprintf(stderr, "FAILED TO SET DESTINATION IP WITH INET_PTON\n");
        ExitProcess(1);
    }

    printf("Switch-Connection> Connected to Switch at %s:%d\n", ip, port);
}


// thread function to continuously read frames from the tap device
// and send them if they contain actual ethernet data
DWORD WINAPI forward_traffic_to_switch(LPVOID arg) {
    struct virtual_port *vport = (struct virtual_port *)arg;
    char buffer[ETHERNET_MAX_LEN];

    while (1) {
        int len = read_frame_from_tap(vport->tap_handle, buffer, sizeof(buffer));
        if (len == -1) {
            fprintf(stderr, "FAILED TO READ FRAME FROM TAP DEVICE\n");
            ExitProcess(1);
        }

        // if the frame has actual data, ensures it has a header then sends to server
        if (len > 0) {
            if (len < 14) {
                fprintf(stderr, "IMPROPER HEADER FOR OUTGOING ETHERNET FRAME\n");
                ExitProcess(1);
            }

            sendto(
                vport->port_socket,
                buffer,
                len,
                0,
                (struct sockaddr *)&vport->switch_address,
                sizeof(vport->switch_address)
            );
        }
    }
}


// helper function to read frames from the tap device in order
// to send them to the server
int read_frame_from_tap(HANDLE tap, char *buffer, int buffer_size) {
    DWORD byte_len;
    if (!ReadFile(tap, buffer, buffer_size, &byte_len, NULL)) {
        DWORD err = GetLastError();
        fprintf(stderr, "ReadFile FAILED: %lu\n", err);
        return -1;
    }
    return (int)byte_len;
}


// thread function to continuously get frames sent over the port
// connection and forward them to the tap device
DWORD WINAPI forward_traffic_to_tap(LPVOID arg) {
    struct virtual_port *vport = (struct virtual_port *)arg;
    char buffer[ETHERNET_MAX_LEN];

    while (1) {
        // this gets traffic over the specified port
        int address_len = sizeof(vport->switch_address);
        int len = recvfrom(
            vport->port_socket,
            buffer,
            sizeof(buffer),
            0,
            (struct sockaddr *)&vport->switch_address,
            &address_len
        );

        // if the frame has actual data and a valid header, sends it to tap
        if (len > 0) {
            if (len < 14) {
                fprintf(stderr, "IMPROPER HEADER FOR INCOMING ETHERNET FRAME\n");
                ExitProcess(1);
            }

            if (write_frame_to_tap(vport->tap_handle, buffer, len) == -1) {
                fprintf(stderr, "FAILED TO WRITE FRAME TO TAP DEVICE\n");
                ExitProcess(1);
            }
        }
    }
}


// helper function to write frames to the tap device that have been
// received from the server
int write_frame_to_tap(HANDLE tap, char *buffer, int buffer_size) {
    DWORD written_len;
    if (!WriteFile(tap, buffer, buffer_size, &written_len, NULL)) {
        DWORD err = GetLastError();
        fprintf(stderr, "WriteFile FAILED: %lu\n", err);
        return -1;
    }
    return (int)written_len;
}