/******************************************************************************
* File Name:   unsigned_cert_generator.c
*
* Description: This file creates an unsigned_certificate.bin that consists of
               object size, command id and unique id of the device. This binary
               is passed as an argument to openssl to create a signed data
*
*******************************************************************************
* Copyright 2020-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define RMA_OBJECT_SIZE    0x14
#define RMA_CMD_ID    0x120028F0

typedef struct {
    unsigned int obj_size;
    unsigned int cmd_id;
    unsigned int unique_id_0;
    unsigned int unique_id_1;
    unsigned int unique_id_2;
} certificate;

certificate unsigned_cert = 
{
    .obj_size = RMA_OBJECT_SIZE,
    .cmd_id = RMA_CMD_ID,
};

int main(int argc, char** argv) {
    /* Create the file */
    FILE *fh = fopen ("unsigned_certificate.bin", "wb");
    if(argc == 0)
    {
        printf("Error: Unique ID not found\r\n");
    }
    else
    {
        if (fh != NULL)
        {
            unsigned_cert.unique_id_0 = strtol(argv[1], NULL, 0);
            unsigned_cert.unique_id_1 = strtol(argv[2], NULL, 0);
            unsigned_cert.unique_id_2 = strtol(argv[3], NULL, 0);
            printf("Input param: %s .. %s .. %s\r\nUnique ID0: %08x Unique ID1: %08x Unique ID2: %08x Certificate size: %lu\r\n", argv[1], argv[2], argv[3], unsigned_cert.unique_id_0, unsigned_cert.unique_id_1, unsigned_cert.unique_id_2, sizeof (unsigned_cert) - sizeof(unsigned int));

            fwrite (&unsigned_cert, 1, sizeof (unsigned_cert), fh);
            fclose (fh);
        }
    }
    return 0;
}
