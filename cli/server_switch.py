

import sys
import socket
from print_mac_table import print_mac_table


# check arguments
if len(sys.argv) != 2:
    print("INVALID ARGUMENTS. COMMAND USAGE:  python3 server_switch.py <PORT_NUMBER>")
    sys.exit(1)

# create server port and address with arguments
server_port = None
try:
    server_port = int(sys.argv[1])
except ValueError:
    print("INVALID ARGUMENT. PORT NUMBER MUST BE AN INTEGER.")
    sys.exit(1)
server_address = ("0.0.0.0", server_port)


# start listening at provided port
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind(server_address)
print(f"Switch> Accepting traffic at port {server_address[1]}")

# set MAC table
mac_table = {}


while True:
    # blocks until an ethernet frame is received
    frame, sender_address = server_socket.recvfrom(1518)

    # gets source and destination mac addresses from the frame
    destination_mac = ":".join(f"{byte:02x}" for byte in frame[0:6])
    source_mac = ":".join(f"{byte:02x}" for byte in frame[6:12])

    # notify a frame was received
    print(f"Switch> Frame RECEIVED at port {server_port}.\nSwitch> Source={source_mac}  Destination={destination_mac}")


    # update mac table with new mac information
    if source_mac not in mac_table or mac_table[source_mac] != sender_address:
        mac_table[source_mac] = sender_address
        print("Switch> MAC Table updated by ARP:\n")
        print_mac_table(mac_table)


    # broadcast ethernet frame to all other ports if its a broadcast
    if destination_mac == "ff:ff:ff:ff:ff:ff":
        for mac in list(mac_table.keys()):
            if (mac == source_mac):
                pass
            else:
                server_socket.sendto(frame, mac_table[mac])
        print(f"Switch> Frame BROADCASTED to all connected ports except {mac_table[source_mac]}")
    # send frame to address based on mac table
    elif destination_mac in mac_table:
        server_socket.sendto(frame, mac_table[destination_mac])
        print(f"Switch> Frame SENT to {mac_table[destination_mac]}")
    # otherwise drop the frame
    else:
        print("Switch> Frame DROPPED")