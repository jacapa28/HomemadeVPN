# function that prints out a stylized MAC table for the switch


def print_mac_table(mac_table: dict[str, (str, int)]) -> None:
    # prints the header
    print(
    " +---------------------------------------------+\n" \
    " |                  MAC Table                  |\n" \
    " |---------------------------------------------|")
    
    # for each table entry, it formats the row
    for mac, port in mac_table.items():
        print(f" |  {mac}  |  {port[0]}:{port[1]}", end="")
        # this gets the correct amount of spacing between the port and row end
        for i in range(0, 20-(len(port[0])+len(str(port[1])))):
            print(" ", end="")
        print("|")
        
        # prints the pretty ending if its the last entry
        keys = list(mac_table.keys())
        if mac == mac_table[keys[-1]]:
            print(" +---------------------------------------------+")
        else:
            print(" |---------------------------------------------|")