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

import subprocess
import sys

# This script converts an RSA public key in .pub format
# to a C-array
# Example Usage:
# rsa_to_c.py \
# <path to public key>/rsa_public_generated.txt \
# -out <path to output directory>/rsa_to_c_generated.txt

integer_types = (int,)


def main():
    """ Main function

    Build the strings to print out the public key modulus and exponent.
    """
    if len(sys.argv) < 2:
        print("Usage: %s <public_key_file_name> [-norev] [-out <file_name>]" % sys.argv[0])
        return 1
    is_reverse = True
    out_file_name = ''
    for idx in range(len(sys.argv)):
        if "-norev" == sys.argv[idx]:
            is_reverse = False
        if "-out" == sys.argv[idx]:
            out_file_name = sys.argv[idx + 1]

    # list to collect bytes of modulus
    modulus_list = []

    # string that will contain the parsed RSA exponent
    rsa_exp = ""

    # containt the length in bits of an RSA modulus
    key_len = 0

    try:
        # build openssl command line
        cmd_line = ['openssl', 'rsa', '-text', '-pubin', '-in',
                    sys.argv[1],
                    '-noout']

        output, error = subprocess.Popen(
            cmd_line, universal_newlines=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

        # check for errors (warnings ignored)
        lines = error.split("\n")
        error_lines = []
        for line in lines:
            if (len(line) != 0) and (("WARNING:" in line) == False):
                error_lines.append(line)
        if len(error_lines) != 0:
            print("OpenSSL call failed" + "\n" + " ".join(cmd_line) + "\n" + str(error_lines))
            return 1

        modulus_found = False
        for line in output.split("\n"):
            if "Public-Key" in line:
                # get length of RSA modulus
                key_len = int(''.join(filter(str.isdigit, line)))
            if "Modulus" in line:
                # modulus record is found
                modulus_found = True
                continue
            if "Exponent" in line:
                modulus_found = False
                # Exponent record is found
                rsa_exp = line.split(" ")[2][1:-1]
            if modulus_found:
                # Collect bytes of modulus to list
                modulus_list = modulus_list + line.strip().split(":")
    except subprocess.CalledProcessError as err:
        print("OpenSSL call failed with errorcode=" + str(err.returncode) \
              + "\n" + str(err.cmd) + "\n" + str(err.output))
        return 1

    # Normalize data
    # Remove empty strings from modulus_list
    modulus_list = [i for i in modulus_list if i]
    if (len(modulus_list) == key_len // 8 + 1) and (int(modulus_list[0]) == 0):
        # remove first zero byte
        modulus_list.pop(0)

    # Check parsed data
    if not key_len:
        print("Key length was not gotten by parsing.")
        return 1
    if len(modulus_list) != (key_len // 8):
        print("Length of parsed Modulus (%s) is not equal to Key length (%s)." % (key_len, len(modulus_list) * 8))
        return 1

    modulus_hex_str = "".join(modulus_list)
    (barret, inv_modulo, r_bar) = calculate_additional_rsa_key_coefs(modulus_hex_str)

    barret_list = convert_hexstr_to_list(barret, is_reverse)
    # add three zero bytes
    barret_list = ([0] * 3 + barret_list) if not is_reverse else (barret_list + [0] * 3)

    barret_str = build_returned_string(barret_list)
    barret_str = ".barrettData =\n{\n%s\n}," % barret_str

    inv_modulo_list = convert_hexstr_to_list(inv_modulo, is_reverse)
    inv_modulo_str = build_returned_string(inv_modulo_list)
    inv_modulo_str = ".inverseModuloData =\n{\n%s\n}," % inv_modulo_str

    r_bar_list = convert_hexstr_to_list(r_bar, is_reverse)
    r_bar_str = build_returned_string(r_bar_list)
    r_bar_str = ".rBarData =\n{\n%s\n}," % r_bar_str

    rsa_exp_list = convert_hexstr_to_list(rsa_exp, is_reverse)
    rsa_exp_list_len = len(rsa_exp_list)
    if rsa_exp_list_len % 4 != 0:
        rsa_exp_list = ([0] * (4 - (rsa_exp_list_len % 4)) + rsa_exp_list) if not is_reverse \
            else (rsa_exp_list + [0] * (4 - (rsa_exp_list_len % 4)))

    rsa_exp_str = build_returned_string(rsa_exp_list)
    rsa_exp_str = ".expData =\n{\n%s\n}," % rsa_exp_str

    # Check and apply is_reverse flag
    if is_reverse:
        modulus_list.reverse()
    modulus_str = build_returned_string(modulus_list)
    modulus_str = ".moduloData =\n{\n%s\n}," % modulus_str

    if not out_file_name:
        print(modulus_str)
        print(rsa_exp_str)
        print(barret_str)
        print(inv_modulo_str)
        print(r_bar_str)
    else:
        with open(out_file_name, 'w') as outfile:
            outfile.write(modulus_str + "\n")
            outfile.write(rsa_exp_str + "\n")
            outfile.write(barret_str + "\n")
            outfile.write(inv_modulo_str + "\n")
            outfile.write(r_bar_str + "\n")
    return 0


def extended_euclid(modulo):
    ''' Calculate greatest common divisor (GCD) of two values.
        Link: https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
            formula to calculate: ax + by - gcd(a,b)
        parameters:
            a, b - two values witch is calculated GCD for.
        return:
            absolute values of x and y coefficients

        NOTE: pseudo-code of operation:
            x, lastX = 0, 1
            y, lastY = 1, 0
            while (b != 0):
                q = a // b
                a, b = b, a % b
                x, lastX = lastX - q * x, x
                y, lastY = lastY - q * y, y
            return (abs(lastX), abs(lastY))
    '''

    r_inv = 1
    n_inv = 0
    modulo_bit_size = modulo.bit_length()

    for i in range(modulo_bit_size):
        if not (r_inv % 2):
            r_inv = r_inv // 2
            n_inv = n_inv // 2
        else:
            r_inv = r_inv + modulo
            r_inv = r_inv // 2
            n_inv = n_inv // 2
            n_inv = n_inv + (1 << (modulo_bit_size - 1))
    return r_inv, n_inv


def calculate_additional_rsa_key_coefs(modulo):
    ''' Calculate three additional coefficients for modulo value of RSA key
        1. barret_coef - Barrett coefficient. Equation is: barretCoef = floor((2 << (2 * k)) / n);
            Main article is here: https://en.wikipedia.org/wiki/Barrett_reduction
        2. r_bar  - pre-calculated value. Equation is: r_bar = (1 << k) mod n;
        3. inverse_modulo - coefficient. It satisfying  rr' - nn' = 1, where r = 1 << k;
            Main article is here:  https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm
        parameter:
            modulo - part of RSA key
        return:
            tuple( barret_coef, r_bar, inverse_modulo ) as reversed byte arrays;
    '''
    if isinstance(modulo, str):
        modulo = int(modulo, 16)
    if modulo <= 0:
        raise ValueError("Modulus must be positive")
    if modulo & (modulo - 1) == 0:
        raise ValueError("Modulus must not be a power of 2")

    modulo_len = modulo.bit_length()
    barret_coef = (1 << (modulo_len * 2)) // modulo
    r_bar = (1 << modulo_len) % modulo
    inverse_modulo = extended_euclid(modulo)
    ret_arrays = (
        barret_coef,
        inverse_modulo[1],
        r_bar
    )

    return ret_arrays


def convert_hexstr_to_list(s, reversed=False):
    '''Converts a string likes '0001aaff...' to list [0, 1, 170, 255].
        Also an input parameter can be an integer, in this case it will be
        converted to a hex string.
    parameter:
        s - string to convert
        reversed - a returned list have to be reversed
    return:
        a list of an integer values
    '''
    if isinstance(s, integer_types):
        s = hex(s)
    s = s[2 if s.lower().startswith("0x") else 0: -1 if s.upper().endswith("L") else len(s)]
    if len(s) % 2 != 0:
        s = '0' + s
    l = [int("0x%s" % s[i:i + 2], 16) for i in range(0, len(s), 2)]
    if reversed:
        l.reverse()
    return l


def build_returned_string(inp_list):
    """Converts a list to a C-style array of hexadecimal numbers string
    """
    if isinstance(inp_list[0], int):
        inp_list = ['%02X' % x for x in inp_list]

    tmp_str = "    "
    for idx in range(0, len(inp_list)):
        if (idx % 8 == 0) and (idx != 0):
            tmp_str = tmp_str + "\n    "
        tmp_str = tmp_str + ("0x%02Xu," % int(inp_list[idx], base=16))
        if (idx % 8 != 7) and (idx != len(inp_list) - 1):
            tmp_str = tmp_str + " "

    return tmp_str


if __name__ == "__main__":
    main()

