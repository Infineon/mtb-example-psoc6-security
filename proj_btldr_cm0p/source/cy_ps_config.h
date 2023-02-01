/******************************************************************************
* File Name:   cy_ps_config.h
*
* Description: Definitions and function prototypes for project security.
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

#ifndef CY_PS_CONFIG_H
#define CY_PS_CONFIG_H

#include <string.h>

#include "source/cy_ps_keystorage.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*               Macros
***************************************/
#define CY_PS_VERSION_MAJOR             1UL  /* Major version */
#define CY_PS_VERSION_MINOR             20UL /* Minor version */
#define CY_PS_APP_VERSION               ((CY_PS_VERSION_MAJOR << 24u) | (CY_PS_VERSION_MINOR << 16u))      /* App Version */
#define CY_PS_ID                        CY_PDL_DRV_ID(0x38u)                                               /* Bootloader ID */
#define CY_PS_ID_INFO                   (uint32_t)( CY_PS_ID | CY_PDL_STATUS_INFO )                        /* Bootloader INFO ID */
#define CY_PS_ID_WARNING                (uint32_t)( CY_PS_ID | CY_PDL_STATUS_WARNING)                      /* Bootloader WARNING ID */
#define CY_PS_ID_ERROR                  (uint32_t)( CY_PS_ID | CY_PDL_STATUS_ERROR)                        /* Bootloader ERROR ID */
#define CY_PS_CHECKID(val)              ((uint32_t)(val) & (CY_PDL_MODULE_ID_Msk << CY_PDL_MODULE_ID_Pos)) /* Check ID macro */


/* MCUBoot bootloader and User application addresses */
#define CY_PS_SECURE_RAM_BEGIN          (CY_START_OF_SRAM)                     /* Bootloader begin (secure) SRAM address */
#define CY_PS_SECURE_RAM_END            (CY_START_OF_SRAM + 0x00001000UL)      /* Bootloader end (secure) SRAM address */
#define CY_PS_SECURE_VT_ADDRESS         (0x10000400UL)                         /* Bootloader VT address */
#define CY_PS_SECURE_DIGSIG_SIZE        (256u)                                 /* Size (in Bytes) of the digital signature */


/* Access Modifiers for Flash and RAM blacklists */
#define CY_PS_MEM_DENIED                (0x00UL)            /* Read and Write access denied */
#define CY_PS_MEM_R                     (0x01UL)            /* Read access allowed */
#define CY_PS_MEM_W                     (0x02UL)            /* Write access allowed */
#define CY_PS_MEM_RW                    (0x03UL)            /* Read and Write access allowed */

/* SysCall interrupt configuration macros.*/
#define CY_PS_SYSCALL_INTR_NUMBER       (NvicMux0_IRQn)     /* SysCall Interrupt NVIC position */
#define CY_PS_SYSCALL_INTR_PRIORITY     (3u)                /* SysCall Interrupt priority */
#define CY_PS_SYSCALL_INTR_STRUCT       (0u)                /* SysCall IPC_INTR_STRUCT number */
#define CY_PS_SYSCALL_INTR_STRUCT_MASK  (0x07UL)            /* SysCall IPC_INTR_STRUCT notify interrupt mask */

/* TOC2 Flag values for PSoC6ABLE2 devices */
#if defined(CY_DEVICE_PSOC6ABLE2)
/* Clock selection for Flash boot execution. */
#define CY_PS_FLASHBOOT_CLK_25MHZ       (0x00UL)            /* 25MHz clock selection for Flashboot (default) */
#define CY_PS_FLASHBOOT_CLK_8MHZ        (0x01UL)            /* 8MHz clock selection for Flashboot */
#define CY_PS_FLASHBOOT_CLK_50MHZ       (0x02UL)            /* 50MHz clock selection for Flashboot */

/* Debugger wait window selection for Flash boot execution. */
#define CY_PS_FLASHBOOT_WAIT_20MS       (0x00UL)            /* 20ms debugger wait window for Flashboot (default) */
#define CY_PS_FLASHBOOT_WAIT_10MS       (0x01UL)            /* 10ms debugger wait window for Flashboot */
#define CY_PS_FLASHBOOT_WAIT_1MS        (0x02UL)            /* 1ms debugger wait window for Flashboot */
#define CY_PS_FLASHBOOT_WAIT_0MS        (0x03UL)            /* 0ms debugger wait window for Flashboot */
#define CY_PS_FLASHBOOT_WAIT_100MS      (0x04UL)            /* 100ms debugger wait window for Flashboot */

/* Flash boot validation selection in chip NORMAL mode. */
#define CY_PS_FLASHBOOT_VALIDATE_YES    (0x01UL)            /* Validate app1 (MCUBoot) in NORMAL mode (default) */
#define CY_PS_FLASHBOOT_VALIDATE_NO     (0x00UL)            /* Do not validate app1 (MCUBoot) in NORMAL mode */
#endif

/* TOC2 Flag values for PSoC6A2M / PSoC6A512K devices */
#if defined(CY_DEVICE_PSOC6A2M) || defined(CY_DEVICE_PSOC6A512K)
/* Clock selection for Flash boot execution. */
#define CY_PS_FLASHBOOT_CLK_8MHZ        (0x00UL)            /* 8MHz clock selection for Flashboot */
#define CY_PS_FLASHBOOT_CLK_25MHZ       (0x01UL)            /* 25MHz clock selection for Flashboot */
#define CY_PS_FLASHBOOT_CLK_50MHZ       (0x02UL)            /* 50MHz clock selection for Flashboot (default) */

/* Debugger wait window selection for Flash boot execution. */
#define CY_PS_FLASHBOOT_WAIT_20MS       (0x00UL)            /* 20ms debugger wait window for Flashboot (default) */
#define CY_PS_FLASHBOOT_WAIT_10MS       (0x01UL)            /* 10ms debugger wait window for Flashboot */
#define CY_PS_FLASHBOOT_WAIT_1MS        (0x02UL)            /* 1ms debugger wait window for Flashboot */
#define CY_PS_FLASHBOOT_WAIT_0MS        (0x03UL)            /* 0ms debugger wait window for Flashboot */
#define CY_PS_FLASHBOOT_WAIT_100MS      (0x04UL)            /* 100ms debugger wait window for Flashboot */

/* Determines if SWJ pins are configured by flash boot */
#define CY_PS_FLASHBOOT_SWJ_PINS_DISABLE (0x01)             /* Disable Debug pins */
#define CY_PS_FLASHBOOT_SWJ_PINS_ENABLE  (0x02)             /* Enable Debug pins (default) */

/* Flash boot validation selection in chip NORMAL mode. */
#define CY_PS_FLASHBOOT_VALIDATE_NO     (0x01UL)            /* Do not validate app1 (MCUBoot) in NORMAL mode */
#define CY_PS_FLASHBOOT_VALIDATE_YES    (0x02UL)            /* Validate app1 (MCUBoot) in NORMAL mode (recommended) */
#endif

/* Error codes returned by Flash boot when it fails to boot. These can be verified by attaching the
* debugger and examining the RAM address pointed to by CY_PS_FLASHBOOT_ERROR_ADDR. */
#define CY_PS_FLASHBOOT_ERROR_ADDR      (CY_SRAM0_BASE + CY_SRAM0_SIZE \
                                        - (CY_SRAM0_SIZE/CPUSS_RAMC0_MACRO_NR)) /* Flashboot error code address */
#define CY_PS_FLASHBOOT_ERROR_NONE                (0UL)             /* Error code, Indicates no error */
#define CY_PS_FLASHBOOT_ERROR_INVALID_APP_SIGN    (0xF1000100UL)    /* Error code, App signature validation failed */
#define CY_PS_FLASHBOOT_ERROR_INVALID_TOC         (0xF1000101UL)    /* Error code, Empty or Invalid TOC    */
#define CY_PS_FLASHBOOT_ERROR_INVALID_KEY         (0xF1000102UL)    /* Error code, Invalid Public key      */
#define CY_PS_FLASHBOOT_ERROR_UNREACHABLE         (0xF1000103UL)    /* Error code, Got to unreachable code */
#define CY_PS_FLASHBOOT_ERROR_TOC_DATA_CLOCK      (0xF1000104UL)    /* Error code, TOC contains invalid CM0+ clock attribute */
#define CY_PS_FLASHBOOT_ERROR_TOC_DATA_DELAY      (0xF1000105UL)    /* Error code, TOC contains invalid listen window delay  */
#define CY_PS_FLASHBOOT_ERROR_FLL_CONFIG          (0xF1000106UL)    /* Error code, FLL configuration failed */
#define CY_PS_FLASHBOOT_ERROR_INVALID_APP_DATA    (0xF1000107UL)    /* Error code, Invalid application structure */

/* Application format selection for secure boot. */
#define CY_PS_APP_FORMAT_BASIC          (0UL)               /* Basic application format (no header) - Deprecated */
#define CY_PS_APP_FORMAT_CYPRESS        (1UL)               /* Cypress application format (Cypress header) - Recommended */

/* Application type selection for secure boot. */
#define CY_PS_APP_ID_FLASHBOOT          (0x8001UL)          /* Flash boot ID Type */
#define CY_PS_APP_ID_SECUREIMG          (0x8002UL)          /* Secure image ID Type */
#define CY_PS_APP_ID_BOOTLOADER         (0x8003UL)          /* Bootloader ID Type */

/***************************************
*            Constants
***************************************/
/* TOC2 Flag offsets and masks for PSoC6ABLE2 devices */
#if defined(CY_DEVICE_PSOC6ABLE2)
#define CY_PS_TOC_FLAGS_CLOCKS_MASK     (0x00000003UL)      /* Mask for Flashboot clock selection */
#define CY_PS_TOC_FLAGS_CLOCKS_POS      (0UL)               /* Bit position of Flashboot clock selection */
#define CY_PS_TOC_FLAGS_DELAY_MASK      (0x0000001CUL)      /* Mask for Flashboot wait window selection */
#define CY_PS_TOC_FLAGS_DELAY_POS       (2UL)               /* Bit position of Flashboot wait window selection */
#define CY_PS_TOC_FLAGS_APP_VERIFY_MASK (0x00008000UL)      /* Mask for Flashboot NORMAL mode app1 validation */
#define CY_PS_TOC_FLAGS_APP_VERIFY_POS  (31UL)              /* Bit position of Flashboot NORMAL mode app1 validation */
#endif

/* TOC2 Flag offsets and masks for PSoC6A2M / PSoCA512K devices */
#if defined(CY_DEVICE_PSOC6A2M) || defined(CY_DEVICE_PSOC6A512K)
#define CY_PS_TOC_FLAGS_CLOCKS_MASK     (0x00000003UL)      /* Mask for Flashboot clock selection */
#define CY_PS_TOC_FLAGS_CLOCKS_POS      (0UL)               /* Bit position of Flashboot clock selection */
#define CY_PS_TOC_FLAGS_DELAY_MASK      (0x0000001CUL)      /* Mask for Flashboot wait window selection */
#define CY_PS_TOC_FLAGS_DELAY_POS       (2UL)               /* Bit position of Flashboot wait window selection */
#define CY_PS_TOC_FLAGS_SWJ_ENABLE_MASK (0x00000060UL)      /* Mask for Flashboot SWJ pins control */
#define CY_PS_TOC_FLAGS_SWJ_ENABLE_POS  (5UL)               /* Bit position of Flashboot SWJ pins control */
#define CY_PS_TOC_FLAGS_APP_VERIFY_MASK (0x00000180UL)      /* Mask for Flashboot NORMAL mode app1 validation */
#define CY_PS_TOC_FLAGS_APP_VERIFY_POS  (7UL)               /* Bit position of Flashboot NORMAL mode app1 validation */
#endif

#define CY_PS_IMGVAL_VERIFYAPP_ADDR     ((volatile uint32_t *)0x16002040UL)         /* Flash boot verify app function address */
#define CY_PS_IMGVAL_VERIFYAPP_REG      (*(uint32_t *)CY_PS_IMGVAL_VERIFYAPP_ADDR)  /* Flash boot verify app function register */
#define CY_PS_IMGVAL_VALIDKEY_ADDR      ((volatile uint32_t *)0x16002044UL)         /* Flash boot validate key function address */
#define CY_PS_IMGVAL_VALIDKEY_REG       (*(uint32_t *)CY_PS_IMGVAL_VALIDKEY_ADDR)   /* Flash boot validate key function register */
#define CY_PS_IMGVAL_CRC16CCITT_ADDR    ((volatile uint32_t *)0x16002048UL)         /* Flash boot CRC16CCITT function address */
#define CY_PS_IMGVAL_CRC16CCITT_REG     (*(uint32_t *)CY_PS_IMGVAL_CRC16CCITT_ADDR) /* Flash boot CRC16CCITT function register */
#define CY_PS_IMGVAL_VALIDTOC_ADDR      ((volatile uint32_t *)0x1600204CUL)         /* Flash boot validate TOC function address */
#define CY_PS_IMGVAL_VALIDTOC_REG       (*(uint32_t *)CY_PS_IMGVAL_VALIDTOC_ADDR)   /* Flash boot validate TOC function register */

#define CY_PS_KEYADDR_PUBLIC_KEY        (0UL)   /* Public Key index in SFlash */
#define CY_PS_KEYADDR_USER_KEY          (1UL)   /* User key start index */

#define CY_PS_TOC2_MAGICNUMBER          (0x01211220UL)  /* TOC2 identifier */

#define CY_PS_TOC_EMPTY                 (0UL)           /* Flag to tell that TOC2 and RTOC2 are empty */
#define CY_PS_TOC_INVALID               (1UL)           /* Flag to tell that TOC2 and RTOC2 are invalid */
#define CY_PS_VALID_TOC2                (2UL)           /* Flag to tell that TOC2 is valid */
#define CY_PS_VALID_RTOC2               (3UL)           /* Flag to tell that RTOC2 is valid */

#define CY_PS_NORMAL_LIFECYCLE          (2UL)           /* Device normal lifecycle */
#define CY_PS_SECURE_LIFECYCLE          (3UL)           /* Device secure lifecycle */

/***************************************
*               Structs
***************************************/

/* Table of Content structure */
typedef struct{
    volatile uint32_t objSize;       /* Object size (Bytes) */
    volatile uint32_t magicNum;      /* TOC ID (magic number) */
    volatile uint32_t userKeyAddr;   /* Secure key address in user Flash */
    volatile uint32_t smifCfgAddr;   /* SMIF configuration structure */
    volatile uint32_t appAddr1;      /* First user application object address */
    volatile uint32_t appFormat1;    /* First user application format */
    volatile uint32_t appAddr2;      /* Second user application object address */
    volatile uint32_t appFormat2;    /* Second user application format */
    volatile uint32_t shashObj;      /* Number of additional objects to be verified (S-HASH) */
    volatile uint32_t sigKeyAddr;    /* Signature verification key address */
    volatile uint32_t addObj[116];   /* Additional objects to include in S-HASH */
    volatile uint32_t tocFlags;      /* Flags in TOC to control Flash boot options */
    volatile uint32_t crc;           /* CRC16-CCITT */
}cy_stc_ps_toc_t;

/* Application header in Cypress format */
typedef struct{
    volatile uint32_t objSize;       /* Object size (Bytes) */
    volatile uint32_t appId;         /* Application ID/version */
    volatile uint32_t appAttributes; /* Attributes (reserved for future use) */
    volatile uint32_t numCores;      /* Number of cores */
    volatile uint32_t core0Vt;       /* (CM0+)VT offset - offset to the vector table from that entry */
    volatile uint32_t core0Id;       /* CM0+ core ID */
}cy_stc_ps_appheader_t;

/* Application error codes */
typedef enum
{
    CY_PS_SUCCESS               = 0x00u,                    /* Operation success */
    CY_PS_BAD_PARAM             = CY_PS_ID_ERROR   | 0x01u, /* Bad parameter was passed */
    CY_PS_FAILURE               = CY_PS_ID_ERROR   | 0x03u, /* Generic operation failure */
    CY_PS_SYSCALL_DISALLOWED    = CY_PS_ID_INFO    | 0x04u, /* Disallowed SysCall command */
    CY_PS_CRYPTO_DISALLOWED     = CY_PS_ID_ERROR   | 0x05u, /* Disallowed Crypto command */
    CY_PS_EMPTY_TOC             = CY_PS_ID_INFO    | 0x06u, /* Empty TOC2 and RTOC2 */
    CY_PS_INVALID_FORMAT        = CY_PS_ID_ERROR   | 0x07u, /* Invalid application format */
    CY_PS_INVALID_TOC           = CY_PS_ID_ERROR   | 0x08u, /* Invalid TOC */
    CY_PS_INVALID_KEY           = CY_PS_ID_ERROR   | 0x09u, /* Invalid key format */
    CY_PS_INVALID_IMAGE         = CY_PS_ID_ERROR   | 0x0au, /* Invalid application */
    CY_PS_INVALID_USERKEY       = CY_PS_ID_ERROR   | 0x0bu, /* Invalid User key */
    CY_PS_INVALID_FLASH_ACCESS  = CY_PS_ID_WARNING | 0x0cu, /* Invalid access to secure Flash address */
    CY_PS_INVALID_RAM_ACCESS    = CY_PS_ID_WARNING | 0x0du, /* Invalid access to secure RAM address */
} cy_en_ps_status_t;

#if defined(__cplusplus)
}

#endif

#endif /* CY_PS_CONFIG_H */

/* [] END OF FILE */
