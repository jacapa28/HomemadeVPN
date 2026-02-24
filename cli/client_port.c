#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include "cli_utils.h"


struct ether_header {
    uint8_t ether_destination_host[6];
    uint8_t ether_source_host[6];
    uint16_t ether_type;
};


struct virtual_port {
    HANDLE tap_handle;
    SOCKET port_socket;
    struct sockaddr_in switch_address;
};


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "INVALID ARGUMENTS. COMMAND USAGE:  virtual_port <SERVER_IP> <SERVER_PORT>\n");
        return 1;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WINSOCK FAILED TO INITIALIZE\n");
        return 1;
    }

    struct virtual_port vport;
    virtual_port_init(&vport, argv[1], argv[2]);

    
}


void virtual_port_init(struct virtual_port *vport, const char *ip, int port) {
    vport->tap_handle = open_tap_handle();
    if (vport->tap_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FAILED TO OPEN TAP DEVICE\n");
        ExitProcess(1);
    }

    vport->port_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (vport->port_socket == INVALID_SOCKET) {
        fprintf(stderr, "FAILED TO OPEN SOCKET: %d\n", WSAGetLastError());
        ExitProcess(1);
    }

    vport->switch_address.sin_family = AF_INET;
    vport->switch_address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &vport->switch_address.sin_addr) != 1) {
        fprintf(stderr, "FAILED TO SET DESTINATION IP WITH INET_PTON\n");
        ExitProcess(1);
    }

    printf("Switch-Connection> Connected to Switch at %s:%d\n", ip, port);
}