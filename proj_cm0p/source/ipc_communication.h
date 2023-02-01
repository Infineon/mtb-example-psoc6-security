/******************************************************************************
* File Name:   ipc_communication.h
*
* Description: This file contains definitions of constants and structures for
*              setting up user pipe and function prototypes for configuring
*              system and user IPC pipe.
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

#ifndef IPC_COMMUNICATION_H
#define IPC_COMMUNICATION_H

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cy_pdl.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* IPC data channel for User PIPE EP0 */
#define CM0P_IPC_INTR_NUM               (cpuss_interrupts_ipc_8_IRQn)
#define CM4_IPC_INTR_NUM                (cpuss_interrupts_ipc_9_IRQn)

#define CM0P_IPC_INT_STRUCT_NUM         (8)
#define CM4_IPC_INT_STRUCT_NUM          (9)

#define CM0P_IPC_INTR_MUX               (NvicMux7_IRQn)

#define IPC_CH0_INTR_RELEASE_MASK       (1UL << CM0P_IPC_INT_STRUCT_NUM)
#define IPC_CH0_INTR_ACQUIRE_MASK       (1UL << CM0P_IPC_INT_STRUCT_NUM)
#define IPC_CH1_INTR_RELEASE_MASK       (1UL << CM4_IPC_INT_STRUCT_NUM)
#define IPC_CH1_INTR_ACQUIRE_MASK       (1UL << CM4_IPC_INT_STRUCT_NUM)

#define IPC_CMD_READ_DATA               0x01

#define IPC_INTR_PRIORITY               (3)

/*******************************************************************************
* Function prototypes
*******************************************************************************/
void setup_ipc_communication_cm0p(void);
void ipc_send_msg_to_cm4(uint32_t message);
uint32_t ipc_rcv_msg_from_cm4(void);


#endif /* IPC_COMMUNICATION_H */

/* [] END OF FILE */
