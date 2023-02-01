/******************************************************************************
* File Name: cy_ps_efuse.c
*
* Description: eFuse configuration for lifecycle and access restrictions.
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

/* Include header files */
#include "cy_ps_efuse.h"
#include "cy_pdl.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Includes eFuse data if set */
#if (CY_EFUSE_AVAILABLE == 1)
    
/* eFuse configuration for PSoC6ABLE2 devices */
#if defined(CY_DEVICE_PSOC6ABLE2)
CY_SECTION(".cy_efuse") __USED const cy_stc_efuse_data_t cy_efuse_data =
{
    .RESERVED = CY_EFUSE_RESERVED0,       /* Reserved bits ignored */
    .DEAD_ACCESS_RESTRICT0 =
    {
        CY_EFUSE_STATE_SET,               /* Disable CM0+ access port */
        CY_EFUSE_STATE_SET,               /* Disable CM4 access port */
        CY_EFUSE_STATE_SET,               /* Disable System access port */
        CY_EFUSE_STATE_SET,               /* Enable the system access port MPU */
        CY_EFUSE_SFLASH_ALLOWED_ENTIRE,   /* SYS AP MPU protection of SFlash */
        CY_EFUSE_MMIO_ALLOWED_ENTIRE,     /* SYS AP MPU protection of MMIO */
    },
    .DEAD_ACCESS_RESTRICT1 =
    {
        CY_EFUSE_FLASH_ALLOWED_ENTIRE,    /* SYS AP MPU protection of Flash */
        CY_EFUSE_SRAM_ALLOWED_ENTIRE,     /* SYS AP MPU protection of SRAM */
        CY_EFUSE_STATE_IGNORE,            /* Unused */
        CY_EFUSE_STATE_SET                /* Disable "direct execute" system call */
    },
    .SECURE_ACCESS_RESTRICT0 =
    {
        CY_EFUSE_STATE_SET,               /* Disable CM0+ access port */
        CY_EFUSE_STATE_SET,               /* Disable CM4 access port */
        CY_EFUSE_STATE_SET,               /* Disable System access port */
        CY_EFUSE_STATE_SET,               /* Enable the system access port MPU */
        CY_EFUSE_SFLASH_ALLOWED_ENTIRE,   /* SYS AP MPU protection of SFlash */
        CY_EFUSE_MMIO_ALLOWED_ENTIRE,     /* SYS AP MPU protection of MMIO */
    },
    .SECURE_ACCESS_RESTRICT1 =
    {
        CY_EFUSE_FLASH_ALLOWED_ENTIRE,    /* SYS AP MPU protection of Flash */
        CY_EFUSE_SRAM_ALLOWED_ENTIRE,     /* SYS AP MPU protection of SRAM */
        CY_EFUSE_STATE_IGNORE,            /* Unused */
        CY_EFUSE_STATE_SET                /* Disable "direct execute" system call */
    },
    .LIFECYCLE_STAGE =
    {
        CY_EFUSE_STATE_IGNORE,            /* Normal lifecycle already set - ignore */
        CY_EFUSE_STATE_IGNORE,            /* Transition to "Secure with Debug" lifecycle */
        CY_EFUSE_STATE_SET,               /* Transition to "Secure" lifecycle */
        CY_EFUSE_STATE_IGNORE,            /* Transition to "RMA" lifecycle. Cypress use only - ignore */
        CY_EFUSE_LIFECYCLE_RESERVED0      /* Reserved bits ignored */
    },
    /* Reserved bits ignored */
    .RESERVED1 = CY_EFUSE_RESERVED1,
    /* All user EFUSE data ignored */
    .CUSTOMER_DATA = CY_EFUSE_CUSTOMER_IGNORE_PSOC6ABLE2
};
#endif /* defined(CY_DEVICE_PSOC6ABLE2) */

/* EFuse configuration for PSoC6A2M/PSoC6A512K devices */
#if defined(CY_DEVICE_PSOC6A2M) || defined(CY_DEVICE_PSOC6A512K)
CY_SECTION(".cy_efuse") __USED const cy_stc_efuse_data_t cy_efuse_data =
{
    .RESERVED = CY_EFUSE_RESERVED0,       /* Reserved bits ignored */
    .DEAD_ACCESS_RESTRICT0 =
    {
        CY_EFUSE_STATE_SET,               /* Disable CM0+ access port */
        CY_EFUSE_STATE_SET,               /* Disable CM4 access port */
        CY_EFUSE_STATE_SET,               /* Disable System access port */
        CY_EFUSE_STATE_SET,               /* Enable the system access port MPU */
        CY_EFUSE_SFLASH_ALLOWED_ENTIRE,   /* SYS AP MPU protection of SFlash */
        CY_EFUSE_MMIO_ALLOWED_ENTIRE,     /* SYS AP MPU protection of MMIO */
    },
    .DEAD_ACCESS_RESTRICT1 =
    {
        CY_EFUSE_FLASH_ALLOWED_ENTIRE,    /* SYS AP MPU protection of Flash */
        CY_EFUSE_SRAM_ALLOWED_ENTIRE,     /* SYS AP MPU protection of SRAM */
        CY_EFUSE_STATE_IGNORE,            /* Unused */
        CY_EFUSE_STATE_SET                /* Disable "direct execute" system call */
    },
    .SECURE_ACCESS_RESTRICT0 = 
    {
        CY_EFUSE_STATE_SET,               /* Disable CM0+ access port */
        CY_EFUSE_STATE_SET,               /* Disable CM4 access port */
        CY_EFUSE_STATE_SET,               /* Disable System access port */
        CY_EFUSE_STATE_SET,               /* Enable the system access port MPU */
        CY_EFUSE_SFLASH_ALLOWED_ENTIRE,   /* SYS AP MPU protection of SFlash */
        CY_EFUSE_MMIO_ALLOWED_ENTIRE,     /* SYS AP MPU protection of MMIO */
    },
    .SECURE_ACCESS_RESTRICT1 = 
    {
        CY_EFUSE_FLASH_ALLOWED_ENTIRE,    /* SYS AP MPU protection of Flash */
        CY_EFUSE_SRAM_ALLOWED_ENTIRE,     /* SYS AP MPU protection of SRAM */
        CY_EFUSE_STATE_IGNORE,            /* Unused */
        CY_EFUSE_STATE_SET                /* Disable "direct execute" system call */
    },
    .LIFECYCLE_STAGE =
    {
        CY_EFUSE_STATE_IGNORE,            /* Normal lifecycle already set - ignore */
        CY_EFUSE_STATE_IGNORE,            /* Transition to "Secure with Debug" lifecycle */
        CY_EFUSE_STATE_SET,               /* Transition to "Secure" lifecycle */
        CY_EFUSE_STATE_IGNORE,            /* Transition to "RMA" lifecycle. Cypress use only - ignore */
        CY_EFUSE_LIFECYCLE_RESERVED0      /* Reserved bits ignored */
    },
    /* Reserved bits ignored */
    .RESERVED1 = CY_EFUSE_RESERVED1,
    /* All asset hash data should be ignored */
    .CY_ASSET_HASH0 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH1 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH2 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH3 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH4 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH5 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH6 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH7 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH8 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH9 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH10 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH11 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH12 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH13 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH14 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH15 = CY_EFUSE_ASSET_HASH_IGNORE,
    .CY_ASSET_HASH_ZEROS = CY_EFUSE_ASSET_HASH_IGNORE,
    /* All user EFUSE data ignored */
#if defined(CY_DEVICE_PSOC6A2M)
    .CUSTOMER_DATA = CY_EFUSE_CUSTOMER_IGNORE_PSOC6A2M
#else
    .CUSTOMER_DATA = CY_EFUSE_CUSTOMER_IGNORE_PSOC6A512K
#endif /* defined(CY_DEVICE_PSOC6A2M) */
};
#endif /* defined(CY_DEVICE_PSOC6A2M) || defined(CY_DEVICE_PSOC6A512K) */

#endif /* (CY_EFUSE_AVAILABLE == 1) */

#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
