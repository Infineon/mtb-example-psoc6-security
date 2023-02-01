/******************************************************************************
* File Name:   ipc_communication.c
*
* Description: This file contains function definitions for setting up system
*              IPC communication and user IPC pipe.
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
#include "cy_pdl.h"
#include "ipc_communication.h"

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
extern void cm4_msg_callback(void);

/*******************************************************************************
 * Function Name: ipc_send_msg_to_cm0p
 ********************************************************************************
 * Summary:
 *   Sends a message word to the CM0p core on a dedicated channel.
 *
 * Parameters:
 *   message - The message to be sent to the other core
 *
 *******************************************************************************/
void ipc_send_msg_to_cm0p(uint32_t message)
{
    /* Get IPC base register address */
    IPC_STRUCT_Type *ipc_intr_cm0p_addr = Cy_IPC_Drv_GetIpcBaseAddress(CM0P_IPC_INT_STRUCT_NUM);

    int status = CY_IPC_DRV_SUCCESS;

    /* Send message on the IPC channel */
    status = Cy_IPC_Drv_SendMsgWord(ipc_intr_cm0p_addr, IPC_CH0_INTR_ACQUIRE_MASK, message);

    if(CY_IPC_DRV_SUCCESS != status)
    {
        /* Insert error handling */
        CY_ASSERT(0);
    }
}

/*******************************************************************************
 * Function Name: ipc_rcv_msg_from_cm0p
 ********************************************************************************
 * Summary:
 *   Receives a message word from the CM0p core on a dedicated channel.
 *
 * Parameters:
 *   void
 * 
 * Result:
 *   uint32_t - The message word sent from CM0p
 *
 *******************************************************************************/
uint32_t ipc_rcv_msg_from_cm0p(void)
{
    /* Get IPC base register address */
    IPC_STRUCT_Type *ipc_intr_cm4_addr = Cy_IPC_Drv_GetIpcBaseAddress(CM4_IPC_INT_STRUCT_NUM);

    uint32_t message = 0;
    int status = CY_IPC_DRV_SUCCESS;

    /* Read the message word received on the IPC channel */
    status = Cy_IPC_Drv_ReadMsgWord(ipc_intr_cm4_addr, &message);

    if (CY_IPC_DRV_SUCCESS != status)
    {
        /* Insert error handling */
        CY_ASSERT(0);
    }

    return message;
}

/*******************************************************************************
 * Function Name: setup_ipc_communication_cm4
 ********************************************************************************
 * Summary:
 *   Sets up IPC hardware on the CM4 for inter-core communication.
 *
 * Parameters:
 *   void
 *
 *******************************************************************************/
void setup_ipc_communication_cm4(void)
{
    Cy_IPC_Sema_Init(CY_IPC_CHAN_SEMA, 0UL, (uint32_t *)NULL);

    /* Get interrupt base address for the corresponding IPC struct number */
    IPC_INTR_STRUCT_Type *ipc_intr_cm4_addr = Cy_IPC_Drv_GetIntrBaseAddr(CM4_IPC_INT_STRUCT_NUM);

    uint32_t ipc_intr_cm4_mask = Cy_IPC_Drv_GetInterruptMask(ipc_intr_cm4_addr);
    uint32_t release_intr = _FLD2VAL(IPC_INTR_STRUCT_INTR_MASK_RELEASE, ipc_intr_cm4_mask) | IPC_CH1_INTR_RELEASE_MASK;
    uint32_t acquire_intr = _FLD2VAL(IPC_INTR_STRUCT_INTR_MASK_NOTIFY, ipc_intr_cm4_mask) | IPC_CH1_INTR_ACQUIRE_MASK;

    /* Setup the interrupt mask for acquire and release notification */
    Cy_IPC_Drv_SetInterruptMask(ipc_intr_cm4_addr, release_intr, acquire_intr);

    /* Configure interrupt for IPC */
    cy_stc_sysint_t ipc_intr_config =
    {
        .intrSrc = (IRQn_Type) CM4_IPC_INTR_NUM,
        .intrPriority = IPC_INTR_PRIORITY
    };

    /* Initialize interrupt */
    (void)Cy_SysInt_Init(&ipc_intr_config, &cm4_msg_callback);

    /* Enable the interrupts */
    NVIC_EnableIRQ(ipc_intr_config.intrSrc);
}


/* [] END OF FILE */
