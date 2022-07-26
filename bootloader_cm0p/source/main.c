/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for PSoC 6 MCU: Security Application
 *              Template for ModusToolbox.
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

/* Driver header files */
#include "cy_pdl.h"
#include "cy_result.h"
#include "cycfg.h"
#include "cy_retarget_io_pdl.h"
#include "cycfg_clocks.h"
#include "cycfg_peripherals.h"
#include "cycfg_pins.h"

/* Flash PAL header files */
#ifdef CY_BOOT_USE_EXTERNAL_FLASH
#include "flash_qspi.h"
#endif

/* MCUboot header files */
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "bootutil/sign_key.h"
#include "bootutil/bootutil_log.h"

#ifdef CY_FLASH_MAP_EXT_DESC
#include <source/custom_memory_map.h>
#endif

/* Watchdog header file */
#include "watchdog.h"

/* Program security header files */
#include "cy_ps_config.h"
#include "cy_ps_prot_units.h"
#include "cy_ps_keystorage.h"
#include "cy_ps_efuse.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
/* Configures the SWJ pins to allow debugging if set
 * Note: Only possible if access restrictions permit it
 */
#define CONFIGURE_SWJ_PINS               (0u)

/* WDT time out for reset mode, in milliseconds. */
#define WDT_TIME_OUT_MS                  (4000UL)

/* Number of attempts to check if UART TX is complete. 10ms delay is applied
 * between successive attempts.
 */
#define UART_TX_COMPLETE_POLL_COUNT      (10UL)

/* EFuse Register Offsets */
#define LIFECYCLE_EFUSE_OFFSET           0x02B
#define DEAD_ACCESS0_EFUSE_OFFSET        0x027
#define DEAD_ACCESS1_EFUSE_OFFSET        0x028
#define SECURE_ACCESS0_EFUSE_OFFSET      0x029
#define SECURE_ACCESS1_EFUSE_OFFSET      0x02A

/* Flashboot Access Control Register Masks */
#define CY_FB_AP_CTL_CM0_ENABLE_MASK     ( (uint32_t)1u <<  0u)
#define CY_FB_AP_CTL_CM4_ENABLE_MASK     ( (uint32_t)1u <<  1u)
#define CY_FB_AP_CTL_SYS_ENABLE_MASK     ( (uint32_t)1u <<  2u)

/* Flashboot Access Control / Debug Port Status Register addresses */
#if defined(CY_DEVICE_PSOC6A2M) || defined(CY_DEVICE_PSOC6A512K)
#define CPUSS_AP_CTL_ADDR                (0x40201414)
#define CPUSS_AP_CTL                     (* (reg32 *) CPUSS_AP_CTL_ADDR)
#define CPUSS_DP_STATUS_ADDR             (0x40201410)
#define CPUSS_DP_STATUS                  (* (reg32 *) CPUSS_DP_STATUS_ADDR)
#elif defined(CY_DEVICE_PSOC6ABLE2)
#define CPUSS_AP_CTL_ADDR                (0x40210540)
#define CPUSS_AP_CTL                     (* (reg32 *) CPUSS_AP_CTL_ADDR)
#define CPUSS_DP_STATUS_ADDR             (0x40210208)
#define CPUSS_DP_STATUS                  (* (reg32 *) CPUSS_DP_STATUS_ADDR)
#endif

/*******************************************************************************
 * Function prototypes
 *******************************************************************************/
static void deinit_hw(void);
static void do_boot(void);

#if (CONFIGURE_SWJ_PINS == 1u)
static void configure_swj(void);
#endif


/* Flashboot parameters stored in TOC2 for PSOC6ABLE2 devices */
#if defined(CY_DEVICE_PSOC6ABLE2)
#define CY_PS_FLASHBOOT_FLAGS ((CY_PS_FLASHBOOT_VALIDATE_YES << CY_PS_TOC_FLAGS_APP_VERIFY_POS)  \
        | (CY_PS_FLASHBOOT_WAIT_20MS << CY_PS_TOC_FLAGS_DELAY_POS) \
        | (CY_PS_FLASHBOOT_CLK_25MHZ << CY_PS_TOC_FLAGS_CLOCKS_POS))
#endif

/* Flashboot parameters stored in TOC2 for PSOC6A2M / PSoC6A512K devices */
#if defined(CY_DEVICE_PSOC6A2M) || defined(CY_DEVICE_PSOC6A512K)
#define CY_PS_FLASHBOOT_FLAGS ((CY_PS_FLASHBOOT_VALIDATE_YES << CY_PS_TOC_FLAGS_APP_VERIFY_POS)  \
        | (CY_PS_FLASHBOOT_WAIT_20MS << CY_PS_TOC_FLAGS_DELAY_POS) \
        | (CY_PS_FLASHBOOT_CLK_25MHZ << CY_PS_TOC_FLAGS_CLOCKS_POS) \
        | (CY_PS_FLASHBOOT_SWJ_PINS_ENABLE << CY_PS_TOC_FLAGS_SWJ_ENABLE_POS))
#endif

/* TOC2 in SFlash */
CY_SECTION(".cy_toc_part2") __USED static const cy_stc_ps_toc_t cy_toc2 =
{
        .objSize     = sizeof(cy_stc_ps_toc_t) - sizeof(uint32_t),  /* Object Size (Bytes) excluding CRC */
        .magicNum    = CY_PS_TOC2_MAGICNUMBER,                      /* TOC2 ID (magic number) */
        .userKeyAddr = (uint32_t)&CySecureKeyStorage,               /* User key storage address */
        .smifCfgAddr = 0UL,                                         /* SMIF config list pointer */
        .appAddr1    = CY_START_OF_FLASH,                           /* App1 (MCUBoot) start address */
        .appFormat1  = CY_PS_APP_FORMAT_CYPRESS,                    /* App1 Format */
        .appAddr2    = 0,                                           /* App2 (User App) start address */
        .appFormat2  = 0,                                           /* App2 Format */
        .shashObj    = 1UL,                                         /* Include public key in the SECURE HASH */
        .sigKeyAddr  = (uint32_t)&SFLASH->PUBLIC_KEY,               /* Address of signature verification key */
        .tocFlags    = CY_PS_FLASHBOOT_FLAGS,                       /* Flashboot flags stored in TOC2 */
        .crc         = 0UL                                          /* CRC populated by cymcuelftool */
};

/* RTOC2 in SFlash, this is a duplicate of TOC2 for redundancy */
CY_SECTION(".cy_rtoc_part2") __USED static const cy_stc_ps_toc_t cy_rtoc2 =
{
        .objSize     = sizeof(cy_stc_ps_toc_t) - sizeof(uint32_t),  /* Object Size (Bytes) excluding CRC */
        .magicNum    = CY_PS_TOC2_MAGICNUMBER,                      /* TOC2 ID (magic number) */
        .userKeyAddr = (uint32_t)&CySecureKeyStorage,               /* User key storage address */
        .smifCfgAddr = 0UL,                                         /* SMIF config list pointer */
        .appAddr1    = CY_START_OF_FLASH,                           /* App1 (MCUBoot) start address */
        .appFormat1  = CY_PS_APP_FORMAT_CYPRESS,                    /* App1 Format */
        .appAddr2    = 0,                                           /* App2 (User App) start address */
        .appFormat2  = 0,                                           /* App2 Format */
        .shashObj    = 1UL,                                         /* Include public key in the SECURE HASH */
        .sigKeyAddr  = (uint32_t)&SFLASH->PUBLIC_KEY,               /* Address of signature verification key */
        .tocFlags    = CY_PS_FLASHBOOT_FLAGS,                       /* Flashboot flags stored in TOC2 */
        .crc         = 0UL                                          /* CRC populated by cymcuelftool */
};

/***************************************
 *   Application header and signature
 ***************************************/
#define CY_PS_VT_OFFSET     ((uint32_t)(&__Vectors[0]) - CY_START_OF_FLASH \
        - offsetof(cy_stc_ps_appheader_t, core0Vt)) /* CM0+ VT Offset */
#define CY_PS_CPUID         (0xC6000000UL)          /* CM0+ ARM CPUID[15:4] Reg shifted to [31:20] */
#define CY_PS_CORE_IDX      (0UL)                   /* Index ID of the CM0+ core */

/** Secure Application header */
CY_SECTION(".cy_app_header") __USED
        static const cy_stc_ps_appheader_t cy_ps_appHeader = {
                .objSize        = CY_BOOT_BOOTLOADER_SIZE - CY_PS_SECURE_DIGSIG_SIZE,
                .appId          = (CY_PS_APP_VERSION | CY_PS_APP_ID_SECUREIMG),
                .appAttributes  = 0UL,                          /* Reserved */
                .numCores       = 1UL,                          /* Only CM0+ */
                .core0Vt        = CY_PS_VT_OFFSET,              /* CM0+ VT offset */
                .core0Id        = CY_PS_CPUID | CY_PS_CORE_IDX, /* CM0+ core ID */
};

/* Secure Digital signature (Populated by cymcuelftool) */
CY_SECTION(".cy_app_signature") __USED CY_ALIGN(4)
static const uint8_t cy_ps_appSignature[CY_PS_SECURE_DIGSIG_SIZE] = {0u};

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  System entrance point. This function initializes peripherals, initializes
 *  retarget IO, and performs a boot by calling the MCUboot functions. 
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 ******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    uint8_t lifecycle, dead_access0, dead_access1, secure_access0, secure_access1;

    /* Structure holding the address to boot from */
    struct boot_rsp rsp;

    /* Initialize system resources and peripherals. */
    init_cycfg_all();

    /* Certain PSoC 6 devices enable CM4 by default at startup. It must be
     * either disabled or enabled & running a valid application for flash write
     * to work from CM0+. Flash writes may be required for updating the image
     * in boot_go(). We need to keep CM4 disabled for this to work. The bootloader
     * app can then enable it in do_boot(). Note that debugging of CM4 is not
     * supported when it is disabled.
     */
#if defined(CY_DEVICE_PSOC6ABLE2)
    if (CY_SYS_CM4_STATUS_ENABLED == Cy_SysGetCM4Status())
    {
        Cy_SysDisableCM4();
    }
#endif /* #if defined(CY_DEVICE_PSOC6ABLE2) */

    /* Enable interrupts */
    __enable_irq();

    /* Initialize retarget-io to redirect the printf output */
    cy_retarget_io_pdl_init(CY_RETARGET_IO_BAUDRATE);

    /* Get lifecycle states and access restrictions */
    Cy_EFUSE_GetEfuseByte(LIFECYCLE_EFUSE_OFFSET,      &lifecycle);
    Cy_EFUSE_GetEfuseByte(DEAD_ACCESS0_EFUSE_OFFSET,   &dead_access0);
    Cy_EFUSE_GetEfuseByte(DEAD_ACCESS1_EFUSE_OFFSET,   &dead_access1);
    Cy_EFUSE_GetEfuseByte(SECURE_ACCESS0_EFUSE_OFFSET, &secure_access0);
    Cy_EFUSE_GetEfuseByte(SECURE_ACCESS1_EFUSE_OFFSET, &secure_access1);

    BOOT_LOG_INF("\r\n=======================================================================");
    BOOT_LOG_INF("MCUboot Bootloader Started (CPU: CM0+)  %s %s",  __DATE__, __TIME__);
    BOOT_LOG_INF("Device lifecycle=0x%02x, dead0=0x%02x, dead1=0x%02x, secure0=0x%02x, " \
            "\r\nsecure1=0x%02x \r\n", lifecycle, dead_access0, dead_access1, secure_access0, \
            secure_access1);

    /* Get current PC value */
    uint32_t active_pc = Cy_Prot_GetActivePC(CPUSS_MS_ID_CM0);
    BOOT_LOG_INF("Active PC value: 0x%02X", (int) active_pc);

#if (CONFIGURE_SWJ_PINS == 1u)

    /* Check current status of debug access ports */
    BOOT_LOG_INF("CPUSS_AP_CTL: 0x%08X, CPUSS_DP_STATUS: 0x%08X  \r\n", \
            (int) CPUSS_AP_CTL, (int) CPUSS_DP_STATUS);

    /* Configure the SWJ pins */
    configure_swj();

    /* Check status of debug access ports after modification */
    BOOT_LOG_INF("CPUSS_AP_CTL after modification: 0x%08X, CPUSS_DP_STATUS: " \
            "0x%08X \r\n", (int) CPUSS_AP_CTL, (int) CPUSS_DP_STATUS);
#endif

    /* Configure the protection units */
    BOOT_LOG_INF("Configuring protection units...");

    result = prot_units_init();

    if(result != CY_PROT_SUCCESS)
    {
        BOOT_LOG_INF("Protection units setup failed!\r\n");
        CY_ASSERT(0);
    }

    BOOT_LOG_INF("Protection units configured successfully!");

    /* Get current PC value */
    active_pc = Cy_Prot_GetActivePC(CPUSS_MS_ID_CM0);
    BOOT_LOG_INF("Active PC value after protection unit " \
            "configuration: 0x%02X\r\n", (int) active_pc);

    /* Validate user application */
    if (boot_go(&rsp) == 0)
    {
        BOOT_LOG_INF("User Application validated successfully");
        BOOT_LOG_INF("--- rsp Addr             0x%08X", (int) &rsp);
        BOOT_LOG_INF("--- HDR Addr             0x%08X", (int) &rsp.br_hdr);
        BOOT_LOG_INF("--- br_flash_div_id      0x%08X", (int) rsp.br_flash_dev_id);
        BOOT_LOG_INF("--- br_image_off         0x%08X", (int) rsp.br_image_off);
        BOOT_LOG_INF("--- ih_load_addr         0x%08X", (int) rsp.br_hdr->ih_load_addr);
        BOOT_LOG_INF("--- ih_img_size          0x%08X", (int) rsp.br_hdr->ih_img_size);
        BOOT_LOG_INF("--- ih_hdr_size          0x%08X", (int) rsp.br_hdr->ih_hdr_size);
        BOOT_LOG_INF("--- ih_protect_tlv_size  0x%08X\r\n", (int) rsp.br_hdr->ih_protect_tlv_size);

        /* Initialize the watchdog timer. It should be updated from the user application
         * to mark successful start up of the application. If the watchdog is not updated,
         * reset will be initiated by watchdog timer and swap revert operation will be
         * started to roll back to the operable image.
         */
        cy_wdg_init(WDT_TIME_OUT_MS);

        /* Jump to user application */
        do_boot();
    }
    else
    {
        BOOT_LOG_INF("MCUboot Bootloader found no bootable image.");
        BOOT_LOG_INF("--- rsp Addr     0x%08X", (int) &rsp);
        BOOT_LOG_INF("--- HDR Addr     0x%08X", (int) rsp.br_hdr);

        /* Wait for UART transfer to complete */
        cy_retarget_io_wait_tx_complete(CYBSP_UART_HW, UART_TX_COMPLETE_POLL_COUNT);
    }

    /* Delay by 500ms for any house-keeping */
    Cy_SysLib_Delay(500);

    while (true)
    {
        /* Enter deep sleep */
        Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }

    return 0;
}

/******************************************************************************
 * Function Name: deinit_hw
 ******************************************************************************
 * Summary:
 *  This function performs the necessary hardware de-initialization.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  None
 *
 ******************************************************************************/
static void deinit_hw(void)
{
    /* Deinitialize the UART hardware */
    cy_retarget_io_pdl_deinit();

    /* Deinitialize the UART pins */
    Cy_GPIO_Port_Deinit(CYBSP_UART_RX_PORT);
    Cy_GPIO_Port_Deinit(CYBSP_UART_TX_PORT);
}

/******************************************************************************
 * Function Name: do_boot
 ******************************************************************************
 * Summary:
 *  This function jumps to the user application address defined in the linker
 *  script
 *
 * Parameters:
 *  rsp - Pointer to a structure holding the address to boot from.
 *
 * Return:
 *  None
 *
 ******************************************************************************/
static void do_boot(void)
{
    static uint32_t appStartAddr = 0u;
    static uint32_t appStackPtr  = 0u;

#define RESET_VECTOR_POS (0x04)

    /* CM0P_APP_FLASH_START and MCUBOOT_HEADER_SIZE is defined in Makefiles */
    uint32_t *CM0_App_Stack_Ptr = (uint32_t *)(CM0P_APP_FLASH_START + MCUBOOT_HEADER_SIZE);
    uint32_t *CM0_App_PC_Ptr    = (uint32_t *)(CM0P_APP_FLASH_START + MCUBOOT_HEADER_SIZE + RESET_VECTOR_POS);

    BOOT_LOG_INF("CM0 app stack: 0x%08X", (int) *CM0_App_Stack_Ptr);
    BOOT_LOG_INF("CM0 app PC:    0x%08X", (int) *CM0_App_PC_Ptr);

    /* Get the user application start address and stack pointer address */
    appStartAddr = *CM0_App_PC_Ptr;
    appStackPtr  = *CM0_App_Stack_Ptr;

    /* Set the main stack pointer to the user app stack pointer */
    __set_MSP(appStackPtr);

    BOOT_LOG_INF("Starting User Application on CM0+. Please wait...");
    cy_retarget_io_wait_tx_complete(CYBSP_UART_HW, UART_TX_COMPLETE_POLL_COUNT);

    /* De-initialize all hardware resources before jumping */
    deinit_hw();

    /* Transition CM0+ to PC=2 which is assigned for CM0+ user operation */
    Cy_Prot_SetActivePC(CPUSS_MS_ID_CM0, CY_PROT_PC2);

    /* Jump to user application start address */
    ((void (*)(void))appStartAddr)();
}

#if (CONFIGURE_SWJ_PINS == 1u)
/******************************************************************************
 * Function Name: configure_swj
 ******************************************************************************
 * Summary:
 *  This function configures the access ports to allow access from CM0+, CM4
 *  and the System.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  None
 *
 ******************************************************************************/
static void configure_swj(void)
{
    /* Enable CM0+, CM4 and System Access Ports */
    CPUSS_AP_CTL = (CY_FB_AP_CTL_CM0_ENABLE_MASK | CY_FB_AP_CTL_CM4_ENABLE_MASK | CY_FB_AP_CTL_SYS_ENABLE_MASK);

    /* Note, PSoC6A-BLE-2 and PSoC6A-2M devices use the same pins and pin configuration
     * for SWJ functionality
     * P6_4 - SWO/TDO
     * P6_5 - SWDOE/TDI
     * P6_6 - SWDIO/TMS
     * P6_7 - SWCLK/TCLK
     */
    Cy_GPIO_Pin_FastInit(P6_4_PORT, P6_4_NUM, CY_GPIO_DM_STRONG_IN_OFF, 0, P6_4_CPUSS_SWJ_SWO_TDO);
    Cy_GPIO_Pin_FastInit(P6_5_PORT, P6_5_NUM, CY_GPIO_DM_PULLUP_IN_OFF, 0, P6_5_CPUSS_SWJ_SWDOE_TDI);
    Cy_GPIO_Pin_FastInit(P6_6_PORT, P6_6_NUM, CY_GPIO_DM_PULLUP_IN_OFF, 0, P6_6_CPUSS_SWJ_SWDIO_TMS);
    Cy_GPIO_Pin_FastInit(P6_7_PORT, P6_7_NUM, CY_GPIO_DM_PULLDOWN_IN_OFF, 0, P6_7_CPUSS_SWJ_SWCLK_TCLK);
}
#endif

/* [] END OF FILE */
