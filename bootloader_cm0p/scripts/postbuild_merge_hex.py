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
import platform
import sys, argparse
import subprocess
import shutil
from intelhex import IntelHex

if platform.system().lower().startswith('win'):
    # import windows specific modules
    from msilib.schema import CustomAction

# This script signs an output hex file using cysecuretools based
# on the input parameters
# Example Usage:
# postbuild_hex_merge.py \
# --target CY8CKIT-062-WIFI_BT \
# --config Debug
# --toolchain-path ModusToolbox/tools_2.0/gcc-7.2.1 \
# --output-dir ./cm4_app/build/CY8CKIT-062-WIFI_BT/Debug/ \
# --output-name production \
# --user-dual-app-path ./cm4_app \
# --user-dual-app-name primary_app
# --bootloader-app-path ./bootloader_cm0p \
# --bootloader-app-name bootloader_cm0p


def my_args(argv):
    """
    Creates an argument parser with different options

    Args:
        argv: list of arguments to be parsed

    Returns:
        options: Parsed list of arguments
    """

    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('-h', '--help',
                        dest='show_help',
                        action='help',
                        help='Print this help message and exit')

    parser.add_argument('-t', '--target',
                        dest='target_name',
                        action='store',
                        type=str,
                        help="Target BSP name",
                        required=True)

    parser.add_argument('-c', '--config',
                        dest='config',
                        action='store',
                        type=str,
                        help="Build configuration. Values: Debug/Release/Custom",
                        required=True)

    parser.add_argument('--toolchain-path',
                        dest='toolchain_path',
                        action='store',
                        type=str,
                        help="Path to the toolchain tools to use for .hex file generation. Only supports GCC",
                        required=True)

    parser.add_argument('-o', '--output-dir',
                        dest='output_dir',
                        action='store',
                        type=str,
                        help="Path for storing generated merged hex file",
                        required=True)

    parser.add_argument('-n', '--output-name',
                        dest='output_name',
                        action='store',
                        type=str,
                        help="Name of generated hex file",
                        required=True)

    parser.add_argument('--user-dual-app-path',
                        dest='user_dual_app_path',
                        action='store',
                        type=str,
                        help="Path where CM0+/CM4 combined hex file is located",
                        required=True)

    parser.add_argument('--user-dual-app-name',
                        dest='user_dual_app_name',
                        action='store',
                        type=str,
                        help="Path where CM0+/CM4 combined hex file is located",
                        required=True)

    parser.add_argument('--bootloader-app-path',
                        dest='bootloader_app_path',
                        action='store',
                        type=str,
                        help="Path where bootloader hex file is located",
                        required=True)

    parser.add_argument('--bootloader-app-name',
                        dest='bootloader_app_name',
                        action='store',
                        type=str,
                        help="Name of the bootloader application",
                        required=True)

    options = parser.parse_args(argv)
    return options


def main(argv):
    """
    Merges the hex files of different applications into a single
    hex file.

    Args:
        argv: Arguments provided by the user
    """
    options = None

    # Parse the arguments
    options = my_args(argv)
    print("options: {}".format(options))

    # Get path to all the applications
    dual_app_hex_file = options.user_dual_app_path + "/build" + "/" + options.target_name + "/" + options.config + "/" + options.user_dual_app_name + ".hex"
    bootloader_app_hex_file = options.bootloader_app_path + "/build" + "/" + options.target_name + "/" + options.config + "/" + options.bootloader_app_name + ".hex"
    output_hex_file = options.output_dir + "/" + options.output_name + ".hex"

    # Check if gcc tools path is valid
    if os.path.isdir(options.toolchain_path) is False:
        print("ERROR: GCC tools folder not found in path: {}".format(options.toolchain_path))
        exit(-1)

    # Check if dual app elf is valid
    if os.path.isfile(dual_app_hex_file) is False:
        print("ERROR: Dual app HEX file not found in path: {}\r\n".format(dual_app_hex_file))
        exit(-1)

    # Check if bootloader elf is valid
    if os.path.isfile(bootloader_app_hex_file) is False:
        print("ERROR: bootloader app hex file not found in path: {}\r\n".format(bootloader_app_hex_file))
        exit(-1)

    print("\r\nMerging hex files")

    # Merge bootloader with the CM0, CM4 into a single hex file
    ihex = IntelHex()
    ihex.padding = 0x00
    ihex.loadfile(bootloader_app_hex_file, 'hex')
    ihex.merge(IntelHex(dual_app_hex_file), 'ignore')
    ihex.write_hex_file(output_hex_file, write_start_addr=False, byte_count=16)

    exit(0)


if __name__ == "__main__":
    main(sys.argv[1:])
