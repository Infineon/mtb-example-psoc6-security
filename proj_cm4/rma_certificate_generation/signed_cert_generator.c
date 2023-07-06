/******************************************************************************
* File Name:   signed_cert_generator.c
*
* Description: This file appends the signature generated in
               signed_certificate.bin to RMA certificate and creates
               certificate.c file to store the RMA certificate
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

char file_to_cer_pt1[] = {"\n\
typedef struct {\n\
    uint32_t opcode;\n\
    uint32_t obj_size;\n\
    uint32_t cmd_id;\n\
    uint32_t unique_id_0;\n\
    uint32_t unique_id_1;\n\
    uint32_t unique_id_2;\n\
    uint8_t signature[256];\n\
} transit_rma_param_t;\n\
\n\
/* RMA Certificate to be sent to the device */ \n\
transit_rma_param_t rmaParam = \n\
{\n\
    .opcode = 0x28000000,\n\
    .obj_size = 0x00000014,\n\
    .cmd_id = 0x120028F0,\n\
    .unique_id_0 = "};

char file_to_cer_pt2[] = {",\n\
    .unique_id_1 = "};

char file_to_cer_pt3[] = {",\n\
    .unique_id_2 = "};

char file_to_cer_pt4[] = {",\n\
    .signature = {"};

unsigned char digital_signature[256];
char temp_string[20];

unsigned int count = 0;

int main(int argc, char** argv) {
    /* Create the file */

    if(argc == 0)
    {
        printf("Error: Unique ID not found\r\n");
    }
    else
    {
        FILE *fh = fopen ("certificate.c", "w");
        FILE *fSign = fopen("signed_certificate.bin", "r");

        if ((fh != NULL) && (fSign != NULL))
        {
            fwrite (&file_to_cer_pt1, 1, sizeof (file_to_cer_pt1)-1, fh);
            printf("Input param: %s\r\n", argv[1]);

            sprintf(temp_string, "%s", argv[1]);
            fwrite(&temp_string, 1, 10, fh);

            fwrite (&file_to_cer_pt2, 1, sizeof (file_to_cer_pt2)-1, fh);
            printf("Input param: %s\r\n", argv[2]);
            sprintf(temp_string, "%s", argv[2]);
            fwrite(&temp_string, 1, 10, fh);

            fwrite (&file_to_cer_pt3, 1, sizeof (file_to_cer_pt3)-1, fh);
            printf("Input param: %s\r\n", argv[3]);
            sprintf(temp_string, "%s", argv[3]);
            fwrite(&temp_string, 1, 10, fh);

            fwrite (&file_to_cer_pt4, 1, sizeof (file_to_cer_pt4)-1, fh);

            fread ((void *)digital_signature, 1, 256, fSign);

            for (count = 0; count < 255; count++)
            {
                sprintf(temp_string, "0x%02x, ", digital_signature[count]);
                fwrite(&temp_string, 1, 6, fh);
                if((count % 16) == 15)
                {
                    fwrite("\n", 1, 1, fh);
                }
            }

        sprintf(temp_string, "0x%02x}\n};\n", digital_signature[count]);
            fwrite(&temp_string, 1, 8, fh);

            fclose(fSign);
            fclose (fh);
        }
    }
    return 0;
}