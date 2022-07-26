/******************************************************************************
* File Name: cy_ps_efuse.h
*
* Description: Header file for eFuse configuration.
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

#ifndef CY_PS_EFUSE_H
#define CY_PS_EFUSE_H

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
* WARNING: Enables EFUSE write
***************************************/
/* Control macro for including/excluding EFUSE data */
#define CY_EFUSE_AVAILABLE   (0)

/***************************************
*               Macros
***************************************/

/* Helper macros to populate the efuse data structure */
#define CY_EXPAND_ARG2(arg)      arg, arg
#define CY_EXPAND_ARG3(arg)      arg, arg, arg
#define CY_EXPAND_ARG4(arg)      CY_EXPAND_ARG2(arg),   CY_EXPAND_ARG2(arg)     /* Expand to 2 */
#define CY_EXPAND_ARG8(arg)      CY_EXPAND_ARG4(arg),   CY_EXPAND_ARG4(arg)     /* Expand to 4 */
#define CY_EXPAND_ARG16(arg)     CY_EXPAND_ARG8(arg),   CY_EXPAND_ARG8(arg)     /* Expand to 8 */
#define CY_EXPAND_ARG24(arg)     CY_EXPAND_ARG16(arg),  CY_EXPAND_ARG8(arg)     /* Expand to 16 */
#define CY_EXPAND_ARG32(arg)     CY_EXPAND_ARG16(arg),  CY_EXPAND_ARG16(arg)    /* Expand to 32 */
#define CY_EXPAND_ARG64(arg)     CY_EXPAND_ARG32(arg),  CY_EXPAND_ARG32(arg)    /* Expand to 64 */
#define CY_EXPAND_ARG128(arg)    CY_EXPAND_ARG64(arg),  CY_EXPAND_ARG64(arg)    /* Expand to 128 */
#define CY_EXPAND_ARG256(arg)    CY_EXPAND_ARG128(arg), CY_EXPAND_ARG128(arg)   /* Expand to 256 */
#define CY_EXPAND_ARG512(arg)    CY_EXPAND_ARG256(arg), CY_EXPAND_ARG256(arg)   /* Expand to 512 */

/* EFUSE bit action macros */
#define CY_EFUSE_STATE_SET       (0x01U) /* Tell programmer to set the EFUSE bit */
#define CY_EFUSE_STATE_UNSET     (0x00U) /* Tell programmer to check that the EFUSE bit is not set */
#define CY_EFUSE_STATE_IGNORE    (0xffU) /* Tell programmer to ignore the EFUSE bit */

/* EFuse ignore write bits macros */
#define CY_EFUSE_STATE_IGNORE2   CY_EXPAND_ARG2(CY_EFUSE_STATE_IGNORE)      /* Ignore 2-bits */
#define CY_EFUSE_STATE_IGNORE3   CY_EXPAND_ARG3(CY_EFUSE_STATE_IGNORE)      /* Ignore 3-bits */
#define CY_EFUSE_STATE_IGNORE4   CY_EXPAND_ARG4(CY_EFUSE_STATE_IGNORE)      /* Ignore 4-bits */
#define CY_EFUSE_STATE_IGNORE8   CY_EXPAND_ARG8(CY_EFUSE_STATE_IGNORE)      /* Ignore 8-bits */
#define CY_EFUSE_STATE_IGNORE16  CY_EXPAND_ARG16(CY_EFUSE_STATE_IGNORE)     /* Ignore 16-bits */
#define CY_EFUSE_STATE_IGNORE32  CY_EXPAND_ARG32(CY_EFUSE_STATE_IGNORE)     /* Ignore 32-bits */
#define CY_EFUSE_STATE_IGNORE64  CY_EXPAND_ARG64(CY_EFUSE_STATE_IGNORE)     /* Ignore 64-bits */
#define CY_EFUSE_STATE_IGNORE128 CY_EXPAND_ARG128(CY_EFUSE_STATE_IGNORE)    /* Ignore 128-bits */
#define CY_EFUSE_STATE_IGNORE256 CY_EXPAND_ARG256(CY_EFUSE_STATE_IGNORE)    /* Ignore 256-bits */
#define CY_EFUSE_STATE_IGNORE512 CY_EXPAND_ARG512(CY_EFUSE_STATE_IGNORE)    /* Ignore 512-bits */

/* EFuse macros for reserved bits */
#define CY_EFUSE_RESERVED0 {CY_EFUSE_STATE_IGNORE256, CY_EFUSE_STATE_IGNORE32, \
                               CY_EFUSE_STATE_IGNORE16, CY_EFUSE_STATE_IGNORE8 }

#define CY_EFUSE_RESERVED1 {CY_EFUSE_STATE_IGNORE128, CY_EFUSE_STATE_IGNORE32}

#define CY_EFUSE_LIFECYCLE_RESERVED0 {CY_EFUSE_STATE_IGNORE4}  /* Ignore EFUSE_LIFECYCLE_STAGE_Type.RESERVED0 EFUSE bits */

/* SFlash area SYS AP MPU protection Enable macros */
#define CY_EFUSE_SFLASH_ALLOWED_ENTIRE  {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET} /* Allow all SFlash */
#define CY_EFUSE_SFLASH_ALLOWED_1_2     {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET}   /* Allow first 1/2 of SFlash */
#define CY_EFUSE_SFLASH_ALLOWED_1_4     {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET}   /* Allow first 1/4 of SFlash */
#define CY_EFUSE_SFLASH_ALLOWED_NONE    {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET}     /* Do not allow SFlash */
#define CY_EFUSE_SFLASH_ALLOWED_IGNORE  {CY_EFUSE_STATE_IGNORE2}                     /* Ignore bits */

/* MMIO area SYS AP MPU protection Enable macros */
#define CY_EFUSE_MMIO_ALLOWED_ENTIRE    {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET} /* Allow all MMIO */
#define CY_EFUSE_MMIO_ALLOWED_IPC       {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET}   /* Allow only IPC */
#define CY_EFUSE_MMIO_ALLOWED_NONE      {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET}   /* Do not allow MMIO */
#define CY_EFUSE_MMIO_ALLOWED_IGNORE    {CY_EFUSE_STATE_IGNORE2}                     /* Ignore bits */

/* Flash area SYS AP MPU protection Enable macros */
#define CY_EFUSE_FLASH_ALLOWED_ENTIRE   {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET}   /* Allow all Flash */
#define CY_EFUSE_FLASH_ALLOWED_7_8      {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET}     /* Allow first 7/8 of Flash */
#define CY_EFUSE_FLASH_ALLOWED_3_4      {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET}     /* Allow first 3/4 of Flash */
#define CY_EFUSE_FLASH_ALLOWED_1_2      {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET}       /* Allow first 1/2 of Flash */
#define CY_EFUSE_FLASH_ALLOWED_1_4      {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET}     /* Allow first 1/4 of Flash */
#define CY_EFUSE_FLASH_ALLOWED_1_8      {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET}       /* Allow first 1/8 of Flash */
#define CY_EFUSE_FLASH_ALLOWED_1_16     {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET}       /* Allow first 1/16 of Flash */
#define CY_EFUSE_FLASH_ALLOWED_NONE     {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET}         /* Do not allow Flash */
#define CY_EFUSE_FLASH_ALLOWED_IGNORE   {CY_EFUSE_STATE_IGNORE, CY_EFUSE_STATE_IGNORE,CY_EFUSE_STATE_IGNORE} /* Ignore bits */

/* SRAM area SYS AP MPU protectionEnable macros */
#define CY_EFUSE_SRAM_ALLOWED_ENTIRE    {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET}   /* Allow all SRAM */
#define CY_EFUSE_SRAM_ALLOWED_7_8       {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET}     /* Allow first 7/8 of SRAM */
#define CY_EFUSE_SRAM_ALLOWED_3_4       {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET}     /* Allow first 3/4 of SRAM */
#define CY_EFUSE_SRAM_ALLOWED_1_2       {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET}       /* Allow first 1/2 of SRAM */
#define CY_EFUSE_SRAM_ALLOWED_1_4       {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET}     /* Allow first 1/4 of SRAM */
#define CY_EFUSE_SRAM_ALLOWED_1_8       {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET}       /* Allow first 1/8 of SRAM */
#define CY_EFUSE_SRAM_ALLOWED_1_16      {CY_EFUSE_STATE_UNSET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET}       /* Allow first 1/16 of SRAM */
#define CY_EFUSE_SRAM_ALLOWED_NONE      {CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET, CY_EFUSE_STATE_SET}         /* Do not allow SRAM */
#define CY_EFUSE_SRAM_ALLOWED_IGNORE    {CY_EFUSE_STATE_IGNORE, CY_EFUSE_STATE_IGNORE,CY_EFUSE_STATE_IGNORE} /* Ignore bits */

/* Customer data ignore macros */
#define CY_EFUSE_CUSTOMER_IGNORE8      {{CY_EFUSE_STATE_IGNORE8}}                  /* Customer data ignore 8-bits */
#define CY_EFUSE_CUSTOMER_IGNORE16     CY_EXPAND_ARG2(CY_EFUSE_CUSTOMER_IGNORE8)   /* Customer data ignore 16-bits */
#define CY_EFUSE_CUSTOMER_IGNORE32     CY_EXPAND_ARG2(CY_EFUSE_CUSTOMER_IGNORE16)  /* Customer data ignore 32-bits */
#define CY_EFUSE_CUSTOMER_IGNORE64     CY_EXPAND_ARG2(CY_EFUSE_CUSTOMER_IGNORE32)  /* Customer data ignore 64-bits */
#define CY_EFUSE_CUSTOMER_IGNORE128    CY_EXPAND_ARG2(CY_EFUSE_CUSTOMER_IGNORE64)  /* Customer data ignore 128-bits */
#define CY_EFUSE_CUSTOMER_IGNORE256    CY_EXPAND_ARG2(CY_EFUSE_CUSTOMER_IGNORE128) /* Customer data ignore 256-bits */
#define CY_EFUSE_CUSTOMER_IGNORE512    CY_EXPAND_ARG2(CY_EFUSE_CUSTOMER_IGNORE256) /* Customer data ignore 512-bits */
#define CY_EFUSE_CUSTOMER_IGNORE_PSOC6ABLE2  {CY_EFUSE_CUSTOMER_IGNORE512}
#define CY_EFUSE_CUSTOMER_IGNORE_PSOC6A2M    {CY_EFUSE_CUSTOMER_IGNORE256, CY_EFUSE_CUSTOMER_IGNORE64, \
                                                 CY_EFUSE_CUSTOMER_IGNORE32, CY_EFUSE_CUSTOMER_IGNORE16, \
                                                 CY_EFUSE_CUSTOMER_IGNORE8}
#define CY_EFUSE_CUSTOMER_IGNORE_PSOC6A512K  {CY_EFUSE_CUSTOMER_IGNORE256, CY_EFUSE_CUSTOMER_IGNORE64, \
                                                 CY_EFUSE_CUSTOMER_IGNORE32, CY_EFUSE_CUSTOMER_IGNORE16, \
                                                 CY_EFUSE_CUSTOMER_IGNORE8}

/* Asset hash ignore macros */
#define CY_EFUSE_ASSET_HASH_IGNORE     {{CY_EFUSE_STATE_IGNORE8}}

#if defined(__cplusplus)
}
#endif

#endif /* CY_PS_EFUSE_H */

/* [] END OF FILE */
