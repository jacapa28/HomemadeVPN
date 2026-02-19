

import sys
import socket


# check arguments
if len(sys.argv) != 2:
    print("INVALID ARGUMENTS. COMMAND USAGE:  python3 server_switch.py <PORT_NUMBER>")
    sys.exit(1)

# create server port and address with arguments
server_port = None
try:
    server_port = int(sys.argv[1])
except TypeError:
    print("INVALID ARGUMENT. PORT NUMBER MUST BE AN INTEGER.")
    sys.exit(1)
server_address = ("0.0.0.0", server_port)


# start listening at provided port
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind(server_address)
print(f"Switch accepting traffic at port {server_address[1]}")

# set MAC table
mac_table = {}

