/******************************************************************************
* File Name:   custom_memory_map.h
*
* Description: Flash memory descriptors for MCUBoot operation.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company) or
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

#ifndef CUSTOM_MEMORY_MAP_H
#define CUSTOM_MEMORY_MAP_H

#include "flash_map_backend/flash_map_backend.h"
#include "sysflash/sysflash.h"

#ifdef MCUBOOT_SWAP_USING_STATUS
#include "swap_status.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef CY_FLASH_MAP_EXT_DESC

/* Declare the flash partitions structures */
static struct flash_area bootloader;
static struct flash_area primary_1;
static struct flash_area secondary_1;

static struct flash_area bootloader =
{
    .fa_id = FLASH_AREA_BOOTLOADER,
    .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
    .fa_off = CY_START_OF_FLASH,
    .fa_size = CY_BOOT_BOOTLOADER_SIZE
};

static struct flash_area primary_1 =
{
    .fa_id = FLASH_AREA_IMAGE_PRIMARY(0),
    .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
    .fa_off = CY_BOOT_PRIMARY_1_START_ADDRESS,
    .fa_size = CY_BOOT_PRIMARY_1_SIZE
};

static struct flash_area secondary_1 =
{
    .fa_id = FLASH_AREA_IMAGE_SECONDARY(0),
    .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
    .fa_off = CY_BOOT_SECONDARY_1_START_ADDRESS,
    .fa_size = CY_BOOT_SECONDARY_1_SIZE
};

/* Use external Flash Map Descriptors */
struct flash_area *boot_area_descs[] =
{
    &bootloader,
    &primary_1,
    &secondary_1,
#ifdef MCUBOOT_SWAP_USING_SCRATCH
    &scratch,
#endif
#ifdef MCUBOOT_SWAP_USING_STATUS
    &status,
#endif
    NULL
};
#endif  /* CY_FLASH_MAP_EXT_DESC */

#if defined(__cplusplus)
}

#endif

#endif /* CUSTOM_MEMORY_MAP_H */

/* [] END OF FILE */
