/******************************************************************************
* File Name: cy_ps_prot_units.c
*
* Description: This file contains the implementation of the protection units
*   configuration.
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

#include <source/cy_ps_prot_units.h>

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define PROT_UNITS_ALL_PC_MASK      (0x00007FFFUL)  /* Mask for all supported PC values */
#define PROT_UNITS_DEVICE_PC_MASK   (PROT_UNITS_ALL_PC_MASK & ~CY_PROT_SMPU_PC_LIMIT_MASK) /* Mask for device PC values */

#define PROT_UNITS_FLASH_SIZE_2MB   0x00200000UL
#define PROT_UNITS_FLASH_SIZE_1MB   0x00100000UL
#define PROT_UNITS_FLASH_SIZE_512KB 0x00080000UL
#define PROT_UNITS_FLASH_SIZE_256KB 0x00040000UL

#define PROT_UNITS_SRAM_SIZE_1MB    0x00100000UL
#define PROT_UNITS_SRAM_SIZE_512KB  0x00080000UL
#define PROT_UNITS_SRAM_SIZE_288KB  0x00048000UL
#define PROT_UNITS_SRAM_SIZE_256KB  0x00040000UL
#define PROT_UNITS_SRAM_SIZE_128KB  0x00020000UL

/*******************************************************************************
 * Structures
 *******************************************************************************/

/* ------------------------ FLASH Setup ---------------------------- */
/* Slave SMPU config for CM0+ Bootloader Flash region */
static const cy_stc_smpu_cfg_t cm0p_btldr_flash_prot_cfg_s = {
        .address = (uint32_t *)(CY_FLASH_BASE),          /* Start of CM0+ Btldr Flash */
        .regionSize = CY_PROT_SIZE_128KB,                /* Region size of 128KB */
        .subregions = (uint8_t)(CY_PROT_SUBREGION_DIS7), /* Exclude last sub-region */
        .userPermission = CY_PROT_PERM_RX,               /* Full access to PC=1,2 */
        .privPermission = CY_PROT_PERM_RX,               /* Full access to PC=1,2 */
        .secure = true,                                  /* Secure access only */
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1|CY_PROT_PCMASK2) /* Only allow PC=1,2 */
};

/* Slave SMPU config for Protected Storage region */
static const cy_stc_smpu_cfg_t protected_storage_prot_cfg_s = {
        .address = (uint32_t *)(PROTECTED_MEM_START),    /* Start of Protected Storage */
        .regionSize = CY_PROT_SIZE_16KB,                 /* Region size of 16KB */
        .subregions = (uint8_t)(0x00),                   /* Include all sub-regions */
        .userPermission = CY_PROT_PERM_RWX,              /* Full access to PC=2 */
        .privPermission = CY_PROT_PERM_RWX,              /* Full access to PC=2 */
        .secure = true,                                  /* Secure access only */
        .pcMatch = false,
        .pcMask = (uint16_t)CY_PROT_PCMASK2 /* Only allow PC=2 */
};

/* Slave SMPU config for CM0+ Application Flash region */
static const cy_stc_smpu_cfg_t cm0p_app_flash_prot_cfg_s = {
        .address = (uint32_t *)(CM0P_APP_FLASH_START),    /* Start of CM0+ App Flash */
#if CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_2MB
        .regionSize = CY_PROT_SIZE_128KB,                 /* Region size of 128KB */
#elif CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_1MB
        .regionSize = CY_PROT_SIZE_128KB,                 /* Region size of 128KB */
#elif CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_512KB
        .regionSize = CY_PROT_SIZE_64KB,                  /* Region size of 64KB */
#else
        .regionSize = CY_PROT_SIZE_32KB,                  /* Region size of 32KB */
#endif
        .subregions = (uint8_t)(0x00),                    /* Include all sub-regions */
        .userPermission = CY_PROT_PERM_RWX,               /* Full access to PC=1,2 */
        .privPermission = CY_PROT_PERM_RWX,               /* Full access to PC=1,2 */
        .secure = true,                                   /* Secure access only */
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1|CY_PROT_PCMASK2) /* Only allow PC=1,2 */
};

/* For 512K and 1M devices only */
#if (CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_1MB) || (CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_512KB)
/* Slave SMPU config for CM4 and Secondary Slot Application Flash region */
static const cy_stc_smpu_cfg_t cm4_secondary_flash_prot_cfg_s = {
        .address = (uint32_t *)(CY_START_OF_FLASH),         /* Start of Flash */
#if CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_1MB
        .regionSize = CY_PROT_SIZE_1MB,                     /* Region size 1M, subregion = 128K */
        .subregions = (uint8_t)(CY_PROT_SUBREGION_DIS0 \
                | CY_PROT_SUBREGION_DIS1),                  /* Disable subregions 0,1 only */
#elif CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_512KB
        .regionSize = CY_PROT_SIZE_512KB,                   /* Region size 512KB, subregion = 64K */
        .subregions = (uint8_t)(CY_PROT_SUBREGION_DIS0 \
                        | CY_PROT_SUBREGION_DIS1 \
                        | CY_PROT_SUBREGION_DIS2),          /* Disable subregions 0,1,2 only */
#endif
        .userPermission = CY_PROT_PERM_RWX,                 /* Access is RWX for PC=1,4 */
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1|CY_PROT_PCMASK4) /* Only allow PC=1,4 */
};
#endif

/* For 2M devices only */
#if CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_2MB
static const cy_stc_smpu_cfg_t cm4_app_flash_prot_cfg_s = {
        .address = (uint32_t *)(CY_START_OF_FLASH),       /* Start of Flash */
        .regionSize = CY_PROT_SIZE_2MB,                   /* Region size 2M, subregion = 256K */
        .subregions = (uint8_t)(CY_PROT_SUBREGION_DIS0 \
                | CY_PROT_SUBREGION_DIS4 \
                | CY_PROT_SUBREGION_DIS5 \
                | CY_PROT_SUBREGION_DIS6 \
                | CY_PROT_SUBREGION_DIS7),                /* Disable subregions 0,4,5,6,7 only */
        .userPermission = CY_PROT_PERM_RWX,               /* Access is RWX for PC=1,4 */
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1|CY_PROT_PCMASK4) /* Only allow PC=1,4 */
};

static const cy_stc_smpu_cfg_t secondary_slot_flash_prot_cfg_s = {
        .address = (uint32_t *)(CY_BOOT_SECONDARY_1_START_ADDRESS),  /* Start of secondary slot */
        .regionSize = CY_PROT_SIZE_1MB,                              /* Region size 1M, subregion = 128K */
        .subregions = (uint8)(CY_PROT_SUBREGION_DIS7),               /* Disable the last subregion only */
        .userPermission = CY_PROT_PERM_RWX,                          /* Access is RWX for PC=1,4 */
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1|CY_PROT_PCMASK4)        /* Only allow PC=1,4 */
};

static const cy_stc_smpu_cfg_t scratch_flash_prot_cfg_s = {
        .address = (uint32_t *)(CY_BOOT_SCRATCH_START_ADDRESS),  /* Start of secondary slot */
        .regionSize = CY_PROT_SIZE_128KB,                        /* Region size 1M, subregion = 128K */
        .subregions = (uint8)(0x00),                             /* Include all regions */
        .userPermission = CY_PROT_PERM_RW,                       /* Access is RW for PC=1,4 */
        .privPermission = CY_PROT_PERM_RW,
        .secure = true,
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1)                    /* Only allow PC=1 */
};
#endif

/* ------------------------ SRAM Setup ---------------------------- */
/* Slave SMPU config for CM0+ Application SRAM region
 * The IPC pipes/semaphores used by the system for communication between
 * the cores rely on peer core IPC variables that are located in the BSS
 * segment of the SRAM. To allow access to these variables, the first
 * few KBs should be unprotected. Hence, the protection has been excluded
 * for first sub-region for all variants. The protected regions are based
 * on the SMPU SRAM memory map described in the readme.
 */
static const cy_stc_smpu_cfg_t cm0p_sram_prot_cfg_s = {
        .address = (uint32_t *)(CY_SRAM_BASE),             /* Beginning of SRAM  */
#if CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_1MB
        .regionSize = CY_PROT_SIZE_256KB,                  /* 256K of SRAM, sub-region = 32K */
        .subregions = (uint8_t)(CY_PROT_SUBREGION_DIS6
                | CY_PROT_SUBREGION_DIS7),                 /* Exclude (6,7) regions */
#elif CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_288KB
        .regionSize = CY_PROT_SIZE_64KB,                   /* 64K of SRAM, sub-region = 8K */
        .subregions = (uint8_t)(0x00),                     /* Exclude first sub-region */
#elif CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_256KB
        .regionSize = CY_PROT_SIZE_64KB,                   /* 64K of SRAM, sub-region = 8K */
        .subregions = (uint8_t)(0x00),                     /* Exclude first sub-region */
#else
        .regionSize = CY_PROT_SIZE_16KB,                   /* 16K of SRAM */
        .subregions = 0x00,                                /* Include all sub-regions */
#endif
        .userPermission = CY_PROT_PERM_RWX,                /* Read/Write only for PC=1,2 */
        .privPermission = CY_PROT_PERM_RWX,
        .secure = true,                                    /* Secure access only */
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1|CY_PROT_PCMASK2)  /* Only allow PC=1,2 */
};

/* Slave SMPU config for Shared SRAM region */
static const cy_stc_smpu_cfg_t shared_sram_prot_cfg_s = {
        .address = (uint32_t *)(SHARED_SRAM_START),     /* Beginning of Shared SRAM  */
#if CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_1MB
        .regionSize = CY_PROT_SIZE_64KB,                /* 64K of SRAM */
#elif CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_288KB
        .regionSize = CY_PROT_SIZE_32KB,                 /* 32K of SRAM */
#elif CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_256KB
        .regionSize = CY_PROT_SIZE_32KB,                 /* 32K of SRAM */
#else
        .regionSize = CY_PROT_SIZE_16KB,                 /* 16K of SRAM */
#endif
        .subregions = (uint8_t)(0x00),                   /* Include all sub-regions */
        .userPermission = CY_PROT_PERM_RW,               /* Read/Write no execute  */
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK1|CY_PROT_PCMASK2|CY_PROT_PCMASK4) /* Only allow PC=1,2,4 to R/W */
};

/* Slave SMPU config for CM4 App SRAM region
 * The IPC pipes/semaphores used by the system for communication between
 * the cores rely on peer core IPC variables that are located in the BSS
 * segment of the SRAM. To allow access to these variables, the first
 * few KBs should be unprotected. Hence, the protection has been excluded
 * for first sub-region for all variants. The protected regions are based
 * on the SMPU SRAM memory map described in the readme.
 */
static const cy_stc_smpu_cfg_t cm4_sram_prot_cfg_s = {
        .address = (uint32_t *)(CY_SRAM_BASE),                    /* Beginning of SRAM  */
#if CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_1MB
        .regionSize = CY_PROT_SIZE_1MB,                           /* 1MB with 128KB subregions of SRAM */
        .subregions = (uint8_t)(CY_PROT_SUBREGION_DIS0 |
                CY_PROT_SUBREGION_DIS1),                          /* Disable (0,1) regions */
#elif CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_288KB
        .regionSize = CY_PROT_SIZE_256KB,                         /* 256K with 32KB subregions of SRAM */
        .subregions = (uint8_t)(CY_PROT_SUBREGION_DIS0 |
                CY_PROT_SUBREGION_DIS1 | CY_PROT_SUBREGION_DIS2), /* Disable (0,1,2) regions */
#elif CY_SRAM_SIZE == PROT_UNITS_SRAM_SIZE_256KB
        .regionSize = CY_PROT_SIZE_256KB,                         /* 256K with 32KB subregions of SRAM */
        .subregions = (uint8_t)(uint8_t)(CY_PROT_SUBREGION_DIS0 |
                CY_PROT_SUBREGION_DIS1 | CY_PROT_SUBREGION_DIS2), /* Disable (0,1,2) regions */
#else
        .regionSize = CY_PROT_SIZE_128KB,         /* 128K with 16KB subregions of SRAM */
        .subregions = 0x00,                       /* Include all sub-regions */
#endif
        .userPermission = CY_PROT_PERM_RWX,       /* Read/Write no execute  */
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)(CY_PROT_PCMASK4)     /* Only allow PC=4 to R/W */
};

/* Common Master SMPU config for all SMPUs */
static const cy_stc_smpu_cfg_t smpu_prot_cfg_m = {
        .userPermission = CY_PROT_PERM_R, /* Allow all to have read access (PC=0 has write access) */
        .privPermission = CY_PROT_PERM_R, /* Allow all to have read access (PC=0 has write access) */
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)PROT_UNITS_DEVICE_PC_MASK
};

#if(CY_IP_MXPERI_VERSION == 1u)
/* Slave (RG) PPU config for MS_CTL MMIO region */
static const cy_stc_ppu_rg_cfg_t ms_ctl_prot_cfg_s = {
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_DISABLED,
        .secure = true,
        .pcMatch = false,
        .pcMask = (uint16_t)0u /* Only allow PC=0 */
};

/* Common master config for all Region PPUs */
static const cy_stc_ppu_rg_cfg_t rg_prot_cfg_m = {
        .userPermission = CY_PROT_PERM_R, /* Allow all to have read access (PC=0 has write access) */
        .privPermission = CY_PROT_PERM_R, /* Allow all to have read access (PC=0 has write access) */
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)PROT_UNITS_DEVICE_PC_MASK
};

/* Common master config for all PROG PPUs */
static const cy_stc_ppu_prog_cfg_t prog_prot_cfg_m = {
        .userPermission = CY_PROT_PERM_R, /* Allow all to have read access (PC=0 has write access) */
        .privPermission = CY_PROT_PERM_R, /* Allow all to have read access (PC=0 has write access) */
        .secure = false,
        .pcMatch = false,
        .pcMask = (uint16_t)PROT_UNITS_DEVICE_PC_MASK
};
#endif

/*******************************************************************************
 * Function Name: prot_units_init
 ********************************************************************************
 * Summary:
 *   Initialize the protection units.
 *
 *******************************************************************************/
cy_en_prot_status_t prot_units_init(void)
{
    cy_en_prot_status_t status;

    /* Configure Bus masters
     * CM0+ - Allowed PC values = 0,1,2
     * CM4  - Allowed PC values = 4
     * TC   - Allowed PC values = 5
     */
    status = Cy_Prot_ConfigBusMaster(CPUSS_MS_ID_CM0, true, true, CY_PROT_PCMASK1 | CY_PROT_PCMASK2);
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigBusMaster(CPUSS_MS_ID_CM4, true, false, CY_PROT_PCMASK4) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigBusMaster(CPUSS_MS_ID_TC, true, false, CY_PROT_PCMASK5) : status;

    /* Transition the non-secure bus masters to PC != 0 */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_SetActivePC(CPUSS_MS_ID_CM4, CY_PROT_PC4) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_SetActivePC(CPUSS_MS_ID_TC, CY_PROT_PC5) : status;

    /* Configure SMPU slaves */
    /* SMPU 13 - CM0+ Bootloader Flash */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT13, &cm0p_btldr_flash_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT13) : status;

    /* SMPU 12 - CM0+ App SRAM */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT12, &cm0p_sram_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT12) : status;

    /* SMPU 11 - CM0+ App Flash */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT11, &cm0p_app_flash_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT11) : status;

    /* SMPU 10 - Shared SRAM */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT10, &shared_sram_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT10) : status;

    /* For 512K and 1M devices only */
#if (CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_1MB) || (CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_512KB)
    /* SMPU 9 - CM4 App + Secondary Slot Flash */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT9, &cm4_secondary_flash_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT9) : status;

    /* For 2M devices only */
#elif (CY_FLASH_SIZE == PROT_UNITS_FLASH_SIZE_2MB)
    /* SMPU 9 - CM4 App Flash */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT9, &cm4_app_flash_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT9) : status;

    /* SMPU 8 - Secondary Slot Flash */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT8, &secondary_slot_flash_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT8) : status;

    /* SMPU 6 - Scratch Flash */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT6, &scratch_flash_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT6) : status;
#endif

    /* SMPU 7 - Protected Storage Flash */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT7, &protected_storage_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT7) : status;

    /* SMPU 5 - CM4 App SRAM */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT5, &cm4_sram_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuSlaveStruct(PROT_SMPU_SMPU_STRUCT5) : status;

#if(CY_IP_MXPERI_VERSION == 1u)
    /* PPU RG - MS_CTL (Bus master control in SMPU) */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigPpuFixedRgSlaveStruct(PERI_GR_PPU_RG_SMPU, &ms_ctl_prot_cfg_s) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigPpuFixedRgMasterStruct(PERI_GR_PPU_RG_SMPU, &rg_prot_cfg_m) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnablePpuFixedRgSlaveStruct(PERI_GR_PPU_RG_SMPU) : status;
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnablePpuFixedRgMasterStruct(PERI_GR_PPU_RG_SMPU) : status;
#else
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigPpuFixedSlaveAtt(PERI_MS_PPU_FX_PROT_SMPU_MAIN, (uint16_t)0xFFFFu, CY_PROT_PERM_DISABLED, CY_PROT_PERM_DISABLED, true) : status;
#endif
    /************************************************************
     * Take control of the master structs of protection units
     * that can possibly be used to override the protection
     * settings or initiate a denial-of-service attack.
     ************************************************************/
    for(uint8_t idx = 0u; idx < CPUSS_PROT_SMPU_STRUCT_NR; idx++)
    {
        /* Master structs for SMPU 15-0 */
        status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigSmpuMasterStruct((PROT_SMPU_SMPU_STRUCT_Type*) &PROT->SMPU.SMPU_STRUCT[idx], &smpu_prot_cfg_m) : status;
        status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnableSmpuMasterStruct((PROT_SMPU_SMPU_STRUCT_Type*) &PROT->SMPU.SMPU_STRUCT[idx]) : status;

        /* Master structs for PROG PPU 15-0 */
#if(CY_IP_MXPERI_VERSION == 1u)
        status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigPpuProgMasterStruct((PERI_PPU_PR_Type*) &PERI->PPU_PR[idx], &prog_prot_cfg_m) : status;
        status = (status == CY_PROT_SUCCESS) ? Cy_Prot_EnablePpuProgMasterStruct((PERI_PPU_PR_Type*) &PERI->PPU_PR[idx]) : status;
#else
        status = (status == CY_PROT_SUCCESS) ? Cy_Prot_ConfigPpuProgMasterAtt(((PERI_MS_PPU_PR_Type*) &PERI_MS->PPU_PR[idx]), (uint16_t)PROT_UNITS_DEVICE_PC_MASK, CY_PROT_PERM_R, CY_PROT_PERM_R, true) : status;
#endif

        if(status != CY_PROT_SUCCESS)
        {
            break;
        }
    }

    /* Transition CM0+ to PC=1 to lock the protections in place */
    status = (status == CY_PROT_SUCCESS) ? Cy_Prot_SetActivePC(CPUSS_MS_ID_CM0, CY_PROT_PC1) : status;

    return status;
}

/* [] END OF FILE */
