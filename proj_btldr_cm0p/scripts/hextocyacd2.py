#!/usr/bin/env python3

"""
Copyright (c) 2022 Cypress Semiconductor Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import os
import argparse
from intelhex import IntelHex

# This script generates a CYACD2 file from an Intel Hex file based
# on the input parameters
# Example Usage:
# hextocyacd2.py  \
# -row=512 \
# -chk=sum \
# -id=0x1020304  \
# -size=0x10000  \
# input.hex \
# output.cyacd2


PACKET_CHECKSUM_TYPE = {
    "sum": 0,
    "crc": 1
}

APPID_DEFAULT = 1


def generate_appinfo(intel_hex=IntelHex()):
    """
    Generate application info string for CYACD2 format file

    Args:
        intel_hex: Intel hex object of IntelHex library

    Returns:
        str: Generated application info string
    """
    start_address = intel_hex.minaddr()
    app_length = intel_hex.maxaddr() - intel_hex.minaddr()
    return f"@APPINFO:0x{start_address:X},0x{app_length:X}"


def generate_empty_row(address=int, length=int, row_size=int):
    """Generate list of cyacd empty rows. Start and end is aligned to row_size
    to be inside address to address+length range

    Args:
        address: start address
        length: length
        row_size: size of cyacd2 row

    Returns:
        list: cyacd empty rows
    """
    cyacd_rows = list()
    offset = address % row_size
    if offset != 0:
        start_address = address + (row_size - offset)
    else:
        start_address = address

    end_address = address + length
    if end_address % row_size != 0:
        end_address = end_address - (end_address % row_size)

    for row_address in range(start_address, end_address, row_size):
        add_cyacd_row(cyacd_rows, row_address, "", row_size)

    return cyacd_rows


def add_cyacd_row(rows=list(), addr=int, data=int, size=int):
    """Adds cyacd2 string to the list of stings. Data part of the cyacd string
    is l-justified to the cyacd string size and filled with 0.

    Args:
        rows: List of cyacd2 strings
        addr: 4-byte cyacd2 address part
        data: data part of the cyacd2 string, should fit into row_size
        size: cyacd2 row size
    """

    rows.append(f":{change_endian_in_str(addr)}{data:0<{size*2}}")


def main(intel_hex_file=str, flash_row=int, cyacd_row_list=list):
    """Generates main part of the cyacd2 file and write in the file

    Args:
        intel_hex_file: file name and path to the input intel hex
        flash_row: size of the output file in bytes
        cyacd_row_list: list of string of the cyacd2 file,
                        should include header and application info
    """

    print("Starting Script ...")
    print("Current dir:"+os.getcwd())
    ihex = IntelHex(intel_hex_file)
    print(ihex.segments())

    if cyacd_row_list[1] == "":
        cyacd_row_list[1] = generate_appinfo(ihex)

    second_segment = False
    prev_segment_end = None

    for start_addr, end_addr in ihex.segments():
        if second_segment:
            cyacd_row_list.extend(generate_empty_row(prev_segment_end,
                                                     start_addr-prev_segment_end,
                                                     flash_row))

        addr_offset = start_addr % flash_row
        cyacd_row_address = start_addr - addr_offset

        row_end = min(start_addr + flash_row - addr_offset, end_addr)
        cyacd_row = ("".zfill(addr_offset*2)
                     + "".join(f"{ihex[addr_hex]:02X}" for addr_hex in
                               range(start_addr, row_end)))
        add_cyacd_row(cyacd_row_list, cyacd_row_address, cyacd_row, flash_row)
        cyacd_row_address = cyacd_row_address + flash_row

        for address in range(cyacd_row_address, end_addr, flash_row):
            row_end = min(address + flash_row, end_addr)
            cyacd_row = "".join(f"{ihex[addr_hex]:02X}"
                                for addr_hex in range(address, row_end))
            add_cyacd_row(cyacd_row_list, address, cyacd_row, flash_row)

        second_segment = True
        prev_segment_end = end_addr


def write_cyacd2_file(cyacd2_file=str, cyacd_row_list=list):
    """Write file in the cyacd2 format

    Args:
        cyacd2_file: File name and path
        cyacd_row_list: String list of the cyacd2 rows
    """
    print(f"Start write file {cyacd2_file}")
    with open(cyacd2_file, 'w', encoding='ascii') as cyacd2_f:
        for item in cyacd_row_list:
            cyacd2_f.write(f"{item}\n")
    print("Write finished.")


def generate_header(file_version=int, checksum=PACKET_CHECKSUM_TYPE, app_id=int, product_id=int):
    """
    Generate first header line of the cyacd2 file

    Args:
        file_version: number of the file version
        checksum: Type of the packet checksum
        app_id: 1-byte integer value of the application ID
        product_id: 4-byte value of the product ID

    Returns:
        str: Header string of the cyacd2 file
    """
    product_id_reversed = change_endian_in_str(product_id)
    header_str = (
        f"{file_version:02X}0000000000"
        f"{PACKET_CHECKSUM_TYPE[checksum]:02X}"
        f"{app_id:02X}{product_id_reversed}"
    )
    return header_str


def generate_app_info(start_address=int, app_size=int):
    """Generates application info header string for the cyacd2 file

    Args:
        start_address: 4-byte address of the application start
        app_size: 4-byte value of the application size

    Returns:
        str: application info string of the cyacd2 file
    """
    if start_address == 0:
        app_info = ""
    else:
        app_info = f"@APPINFO:0x{start_address:X},0x{app_size:X}"
    return app_info


def change_endian_in_str(int32value=int):
    """Change endian of the 4-byte value in string

    Args:
        int32value: 4-byte value

    Returns:
        str: string with 4-byte value with changed endian
    """
    int32str = f"{int32value:08X}"
    return f"{int32str[6:]}{int32str[4:6]}{int32str[2:4]}{int32str[:2]}"


def auto_int(var):
    """Internal function to define type for different int input values

    Args:
        var (int): Any int input value

    Returns:
        int : integer type
    """
    return int(var, 0)


if __name__ == '__main__':

    # Create the command-line argument options
    parser = argparse.ArgumentParser()
    parser.add_argument("in_intel_hex", type=open,
                            help="Path to the input intel hex file")
    parser.add_argument("out_cyacd2",
                            help="Path to the output CYACD2 file")
    parser.add_argument("-row", "--fileRowSize", type=int,
                            action='store', required=True,
                            help="Row size of the CYACD2 file")
    parser.add_argument("-id", "--productID", type=auto_int,
                            action='store', required=True,
                            help="Product ID - 4-byte value")
    parser.add_argument("-chk", "--packetChecksum", choices=list(PACKET_CHECKSUM_TYPE),
                            default='sum',
                            action='store', required=False,
                            help="Packet checksum type selection")
    parser.add_argument("-ver", "--fileVersion", type=int, default=1,
                            action='store', required=False,
                            help="File version number 1-byte value")
    parser.add_argument("-addr", "--startAddress", type=auto_int, default=0,
                            action='store', required=False,
                            help="File version number 1-byte value")
    parser.add_argument("-size", "--applicationSize", type=auto_int, default=0,
                            action='store', required=False,
                            help="File version number 1-byte value")

    # Parse arguments
    options = parser.parse_args()

    # Generate header string
    header_string = generate_header(options.fileVersion, options.packetChecksum,
                                 APPID_DEFAULT, options.productID)

    # Generate application info string
    app_info_string = generate_app_info(options.startAddress, options.applicationSize)

    # Create CYACD2 row
    cyacd2_rows = [header_string, app_info_string]

    # Generates main part of the CYACD2 file and write in the file
    main(options.in_intel_hex, options.fileRowSize, cyacd2_rows)

    # Write row data into the CYACD2 file
    write_cyacd2_file(options.out_cyacd2, cyacd2_rows)
