/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for CM0+ user application for
 *              ModusToolbox
 *
 * Related Document: See README.md
 *
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
#include "cyhal.h"
#include "cybsp.h"
#include "ipc_communication.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
/* Defines for LED states */
#define HIGH (0u)
#define LOW  (1u)

/* LED pin define for CY8CKIT-062-BLE or CY8CKIT-062S2-4343W or CY8CKIT-062-WIFI-BT kit */
#if defined(CY8CPROTO_062_4343W)
#define USER_LED P13_7
#elif defined(CY8CKIT_062S2_43012) || defined(CY8CKIT_062_BLE) || defined(CY8CKIT_062_WIFI_BT)
#define USER_LED P1_5
#elif defined(CY8CPROTO_063_BLE)
#define USER_LED P6_3
#elif defined(CY8CPROTO_062S3_4343W)
#define USER_LED P11_1
#endif

/* Set LED toggle interval based on image type */
#if defined(BOOT_IMG)
#define LED_TOGGLE_INTERVAL_MS         (1000u)
#define IMG_TYPE                       "BOOT"
#elif defined(UPGRADE_IMG)
#define LED_TOGGLE_INTERVAL_MS         (250u)
#define IMG_TYPE                       "UPGRADE"
#else
#error "[UserApp] Please define the image type: BOOT_IMG or UPGRADE_IMG\n"
#endif

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
/* IPC message commands variable */
uint32_t msg_cmd = 0;

/*******************************************************************************
 * Function prototypes
 *******************************************************************************/
void cm0p_msg_callback(void);

/* Protected storage can be used for storing any critical data.
 * It is static in this code example for simplicity, but can be
 * modified by the user to their own implementation.
 */
CY_SECTION(".protected_storage") static const uint32_t device_id = 0xAA55AA55;

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  This function enables the CM4 core, initializes the LED and toggles it
 *  every 1s or 250ms depending on the image type.
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

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable CM4. CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR);

    /* Initialize the LED pin to strong drive mode */
    cyhal_gpio_init(USER_LED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);

    /* Init the IPC communication for CM0+ */
    setup_ipc_communication_cm0p();

    for (;;)
    {
        /* Add a delay */
        cyhal_system_delay_ms(LED_TOGGLE_INTERVAL_MS);

        /* Toggle the LED */
        cyhal_gpio_toggle(USER_LED);

        /* Process IPC commands */
        switch (msg_cmd)
        {
            case IPC_CMD_READ_DATA:

                /* Send the unique device ID to CM4 to be printed */
                ipc_send_msg_to_cm4(device_id);

                break;

            default:
                break;
        }

        /* Clear command */
        msg_cmd = 0;
    }
}

/*******************************************************************************
 * Function Name: cm0p_msg_callback
 ********************************************************************************
 * Summary:
 *   Callback function to execute when receiving a message from CM4 to CM0+.
 *
 * Parameters:
 *   None
 *
 *******************************************************************************/
void cm0p_msg_callback(void)
{
    /* Get interrupt base address for the corresponding IPC struct number */
    IPC_INTR_STRUCT_Type *ipc_intr_cm0p_addr = Cy_IPC_Drv_GetIntrBaseAddr(CM0P_IPC_INT_STRUCT_NUM);

    /* Retrieve the message from CM4 */
    msg_cmd = ipc_rcv_msg_from_cm4();

    /* Clear the interrupt */
    Cy_IPC_Drv_ClearInterrupt(ipc_intr_cm0p_addr, IPC_CH0_INTR_RELEASE_MASK, IPC_CH0_INTR_ACQUIRE_MASK);
}

/* [] END OF FILE */
