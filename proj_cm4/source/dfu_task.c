/******************************************************************************
* File Name: dfu_task.c
*
* Description: Demonstrates the implementation of a FreeRTOS task for device
*              firmware update and inter-processor communication to
*              request data from the CM0+
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
#include "dfu_task.h"
#include "cybsp.h"
#include "cyhal.h"
#include "cy_pdl.h"
#include "cy_dfu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "transport_uart.h"
#include "cy_retarget_io_pdl.h"
#include "ipc_communication.h"
#include "dfu_user.h"

/****************************************************************************
 * Macros
 *****************************************************************************/
/* LED pin define for  or CY8CKIT-062S2-43012 kit */
#if defined(CY8CKIT_062S2_43012)
#define DFU_STATUS_LED (P11_1)
/* LED pin define for CY8CKIT-062-BLE or CY8CKIT-062-WIFI-BT kit */
#elif defined(CY8CKIT_062_BLE) || defined(CY8CKIT_062_WIFI_BT)
#define DFU_STATUS_LED (P13_7)
/* LED pin define for CY8CPROTO-063-BLE kit */
#elif defined(CY8CPROTO_063_BLE)
#define DFU_STATUS_LED (P7_1)
/* LED pin defines for CY8CPROTO-062S3-4343W and CY8CPROTO-062-4343Wkit */
#elif defined(CY8CPROTO_062_4343W) || defined(CY8CPROTO_062S3_4343W)
/* DFU_STATUS_LED not defined */
#endif

/* Define for image type */
#ifdef BOOT_IMAGE
#define IMG_TYPE                       "BOOT"
#elif defined(UPGRADE_IMAGE)
#define IMG_TYPE                       "UPGRADE"
#else
#error "[UserApp] Please define the image type: BOOT_IMAGE or UPGRADE_IMAGE\n"
#endif

#define DFU_UART_POLL_COUNT (10u)

/****************************************************************************
 * Functions Prototypes
 *****************************************************************************/
void cm4_msg_callback(void);

/****************************************************************************
 * Global Variables
 *****************************************************************************/
/* Message variables */
bool msg_flag = false;
uint32_t msg_value;
bool dfu_start_flag = false;

/******************************************************************************
 * Function Name: dfu_task
 ******************************************************************************
 * Summary:
 *  This task initializes the LED and DFU middleware and then waits for host
 *  to start the DFU operation.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void dfu_task(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Used to count seconds */
    uint32_t count = 0;

    /* Status codes for DFU API */
    cy_en_dfu_status_t status;

    char message[300];

    /*
     * DFU state, one of the:
     * - CY_DFU_STATE_NONE
     * - CY_DFU_STATE_UPDATING
     * - CY_DFU_STATE_FINISHED
     * - CY_DFU_STATE_FAILED
     */
    uint32_t state;

    /* Timeout for Cy_DFU_Continue(), in milliseconds */
    const uint32_t paramsTimeout = 20u;

    /* Buffer to store DFU commands */
    CY_ALIGN(4) static uint8_t buffer[CY_DFU_SIZEOF_DATA_BUFFER];

    /* Buffer for DFU data packets for transport API */
    CY_ALIGN(4) static uint8_t packet[CY_DFU_SIZEOF_CMD_BUFFER];

    /* DFU params, used to configure DFU */
    cy_stc_dfu_params_t dfu_params;

    /* Initialize dfu_params structure */
    dfu_params.timeout          = paramsTimeout;
    dfu_params.dataBuffer       = &buffer[0];
    dfu_params.packetBuffer     = &packet[0];

    /* Initialize the DFU */
    status = Cy_DFU_Init(&state, &dfu_params);

    /* Stop program execution if DFU init failed */
    CY_ASSERT(CY_DFU_SUCCESS == status);

    /* Init the IPC communication for CM4 */
    setup_ipc_communication_cm4();

#ifdef DFU_STATUS_LED
    /* Initialize the DFU status LED */
    result = cyhal_gpio_init(DFU_STATUS_LED, CYHAL_GPIO_DIR_OUTPUT,
            CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_ON);

    if(result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
#endif

    /* Initialize the UART for debug logs */
    result = cy_retarget_io_pdl_init(CY_RETARGET_IO_BAUDRATE);


    if(result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize DFU communication */
    Cy_DFU_TransportStart();

    /* Get unique device ID from protected storage via IPC */
    ipc_send_msg_to_cm0p(IPC_CMD_READ_DATA);

    sprintf(message, "\n=========================================================\r\n"\
            "[DFU task] Image Type: %s, Version: %d.%d.%d, CPU: CM4,\r\n%s, %s\n\r"\
            "\n=========================================================\r\n", IMG_TYPE,\
            APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD,__DATE__, __TIME__);

    /* Print message on serial terminal */
    printf("%s", message);

    while(1)
    {
        /* Check if a message was received from CM0+ */
        if (msg_flag)
        {
            msg_flag = false;

            /* Print the device ID received from CM0+ */
            printf("Unique Device ID received: 0x%08X\n\r", (unsigned int) msg_value);

            printf("Starting DFU operation\r\n");

            dfu_start_flag = true;
        }

        if(dfu_start_flag)
        {
            status = Cy_DFU_Continue(&state, &dfu_params);

            ++count;

            if (state == CY_DFU_STATE_FINISHED)
            {
                printf("Transfer complete. Validating image!\r\n");

                /* Finished loading the application image */
                /* Validate DFU application, if it is valid then switch to it */
                status = Cy_DFU_ValidateApp(1u, &dfu_params);

                if (status == CY_DFU_SUCCESS)
                {
                    printf("Validation successful\r\n");

#ifdef DFU_STATUS_LED
                    /* Set the LED to ON */
                    cyhal_gpio_write(DFU_STATUS_LED, CYBSP_LED_STATE_ON);
#endif

                    /* Wait for TX transfer to complete */
                    cy_retarget_io_wait_tx_complete(CYBSP_UART_HW, DFU_UART_POLL_COUNT);

                    /* Stop transfer and reset */
                    Cy_DFU_TransportStop();
                    Cy_DFU_ExecuteApp(1u);
                }
                else if (status == CY_DFU_ERROR_VERIFY)
                {
                    printf("Validation Failed\r\n");
                    
                    /* Wait for TX transfer to complete */
                    cy_retarget_io_wait_tx_complete(CYBSP_UART_HW, DFU_UART_POLL_COUNT);

                    /*
                     * Restarts loading, an alternatives are to Halt MCU here
                     * or switch to the other app if it is valid.
                     * Error code may be handled here, i.e. print to debug UART.
                     */
                    Cy_DFU_Init(&state, &dfu_params);
                    Cy_DFU_TransportReset();
                    Cy_DFU_ExecuteApp(1u);
                }
            }
            else if (state == CY_DFU_STATE_FAILED)
            {
                /* An error has happened during the loading process */
                /* Handle it here */
                /* In this Code Example just restart loading process */
                Cy_DFU_Init(&state, &dfu_params);
                Cy_DFU_TransportReset();
            }
            else if (state == CY_DFU_STATE_UPDATING)
            {
                uint32_t passed5seconds = (count >= (5000ul/paramsTimeout)) ? 1u : 0u;

#ifdef DFU_STATUS_LED
                /* Toggle LED to indicate image transfer is in progress */
                cyhal_gpio_toggle(DFU_STATUS_LED);
#endif

                /*
                 * if no command has been received during 5 seconds when the loading
                 * has started then restart loading.
                 */
                if (status == CY_DFU_SUCCESS)
                {
                    count = 0u;
                }
                else if (status == CY_DFU_ERROR_TIMEOUT)
                {
                    if (passed5seconds != 0u)
                    {
                        count = 0u;
                        Cy_DFU_Init(&state, &dfu_params);
                        Cy_DFU_TransportReset();
                    }
                }
                else
                {
                    count = 0u;

                    /* Delay because Transport still may be sending error response to a host */
                    cyhal_system_delay_ms(paramsTimeout);
                    Cy_DFU_Init(&state, &dfu_params);
                    Cy_DFU_TransportReset();
                }
            }
        }
    }
}

/*******************************************************************************
 * Function Name: cm4_msg_callback
 ********************************************************************************
 * Summary:
 *   Callback function to execute when receiving a message from CM0+ to CM4.
 *
 * Parameters:
 *   None
 *
 *******************************************************************************/
void cm4_msg_callback(void)
{
    msg_value = ipc_rcv_msg_from_cm0p();

    if (msg_value != 0)
    {
        msg_flag = true;
    }

    /* Get interrupt base address for the corresponding ipc struct number (Range:0-15) */
    IPC_INTR_STRUCT_Type *ipc_intr_cm4_addr = Cy_IPC_Drv_GetIntrBaseAddr(CM4_IPC_INT_STRUCT_NUM);

    /* Clear the interrupt */
    Cy_IPC_Drv_ClearInterrupt(ipc_intr_cm4_addr, IPC_CH1_INTR_RELEASE_MASK,
            IPC_CH1_INTR_ACQUIRE_MASK);
}


/* [] END OF FILE */
