/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for CM4 user application for
*              ModusToolbox.
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
#include "cybsp.h"
#include "cyhal.h"
#include "cy_pdl.h"
#include "cy_result.h"
#include "cy_device.h"
#include "watchdog.h"
#include "cy_retarget_io_pdl.h"

/* FreeRTOS header files */
#include "FreeRTOS.h"
#include "task.h"

/* User task header files */
#include "dfu_task.h"

#define CY_RMA_OPCODE                       (0x28000000UL)                      /* The SROM API opcode for RMA lifecycle stage conversion */
#define CY_RMA_CMD_ID                       (0x120028F0UL)
#define CY_OPCODE_SUCCESS                   (0xA0000000UL)                      /* The command completed with no errors */
#define CY_OPCODE_STS_Msk                   (0xF0000000UL)                      /* The status mask of the SROM API return value */
#define CY_IPC_STRUCT                       (Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_SYSCALL)) /* IPC structure to be used */
#define CY_IPC_NOTIFY_STRUCT0               (0x1UL << CY_IPC_INTR_SYSCALL1)     /* IPC notify bit for IPC_STRUCT0 (dedicated to System Call) */
#define CY_RMA_OBJECT_SIZE                  (20 + 256)                           /* 20 bytes - fixed object size + 256 bytes of signature (can go upto 256 bytes)*/

#define CY_READ_UID_OPCODE                  (0x1F000000UL)                      /* The SROM API opcode to read Unique ID */

#define SUCCESS                             (0)
#define FAILED                              (!SUCCESS)

#define PROTECTED_MEM_SIZE                  0x4000
#define PROTECTED_MEM_START_ADDR            0x1001C000
#define PROTECTED_MEM_ROW_SIZE              0x200

#define IPC_STATUS_WAIT_TIME_S               60u                                 /* maximum wait time for IPC Lock */
#define DELAY_IPC_STATUS_CHECK_MS            1000u                               /* delay for checking the IPC status */

typedef struct
{
    uint32_t unique_id_0;
    uint32_t unique_id_1;
    uint32_t unique_id_2;
}read_uid_param_t;

read_uid_param_t uid_param;

typedef struct {
    uint32_t opcode;
    uint32_t obj_size;
    uint32_t cmd_id;
    uint32_t unique_id_0;
    uint32_t unique_id_1;
    uint32_t unique_id_2;
    uint8_t signature[256];
} transit_rma_param_t;

/* RMA Certificate to be sent to the device. It is unique per device */
transit_rma_param_t rmaParam =
{
    .opcode = 0x28000000,
    .obj_size = 0x00000014,
    .cmd_id = 0x120028F0,
    .unique_id_0 = 0x118c84f4,
    .unique_id_1 = 0x01af2c10,
    .unique_id_2 = 0x007a0415,
    .signature = {0x84, 0x95, 0x4c, 0x6a, 0xf4, 0x80, 0x64, 0x73, 0xc2, 0x7f, 0x5d, 0x46, 0xaa, 0xc7, 0x77, 0x9b,
0xea, 0x20, 0xdf, 0x3e, 0x22, 0xd6, 0xd3, 0x07, 0x8b, 0x1d, 0x80, 0x50, 0xc3, 0xff, 0x82, 0x15,
0x60, 0xb3, 0x8b, 0x22, 0x8d, 0x6f, 0x79, 0x42, 0x25, 0xda, 0x05, 0x66, 0x75, 0x84, 0x3f, 0x56,
0x24, 0x37, 0x3f, 0xa1, 0x9b, 0x4b, 0x2f, 0x3e, 0x35, 0x6b, 0x07, 0x5e, 0x5d, 0xf5, 0xf6, 0x06,
0x44, 0xde, 0xa2, 0xe3, 0xb3, 0x4c, 0x8f, 0xfd, 0x79, 0x55, 0x63, 0x9a, 0xc2, 0x66, 0x0e, 0x87,
0xe1, 0x30, 0x02, 0x3a, 0xe5, 0x1f, 0x03, 0x3d, 0x1e, 0x31, 0x1e, 0xd3, 0xdc, 0x49, 0xe4, 0x8b,
0x28, 0xc6, 0xe6, 0x85, 0xac, 0xbd, 0xcf, 0x3f, 0xcd, 0xf7, 0x21, 0x76, 0xd6, 0x9c, 0x63, 0x32,
0x5d, 0xb5, 0xf7, 0x8b, 0x7a, 0x4c, 0x35, 0xcd, 0x0e, 0x64, 0x45, 0xed, 0x28, 0x40, 0xc7, 0x0d,
0xf4, 0xdc, 0x4b, 0x75, 0xe0, 0x6a, 0x1d, 0x84, 0xac, 0xef, 0x14, 0x4e, 0xdb, 0x6a, 0x6b, 0xad,
0xcf, 0xb2, 0xc9, 0x16, 0xba, 0x45, 0xea, 0xbb, 0x18, 0xb8, 0x21, 0x2a, 0x86, 0x5d, 0x46, 0x44,
0xa2, 0x7e, 0x68, 0x07, 0xb3, 0x9a, 0x41, 0xd6, 0x32, 0x6a, 0x01, 0x83, 0x31, 0xfb, 0x67, 0x0b,
0x28, 0x5a, 0x5a, 0x01, 0x0f, 0xa2, 0x93, 0x40, 0x55, 0xb6, 0x43, 0x64, 0x3b, 0xae, 0xbe, 0x7a,
0xa9, 0xfa, 0x31, 0xcc, 0x81, 0x36, 0x65, 0xd6, 0xf2, 0x58, 0xc0, 0x4c, 0x25, 0x58, 0xe7, 0x4e,
0x6d, 0xa4, 0x2c, 0x59, 0xa0, 0x6a, 0xae, 0x6c, 0xc4, 0xa8, 0x39, 0xa0, 0x6a, 0xc3, 0x58, 0xe8,
0x87, 0xb1, 0xaa, 0xfc, 0x86, 0x0f, 0x5c, 0x16, 0x11, 0x20, 0x8d, 0xa1, 0x73, 0xb4, 0x0d, 0x56,
0x2c, 0xa6, 0x62, 0x20, 0x6c, 0x28, 0x2d, 0xbf, 0x77, 0xaa, 0x16, 0x16, 0x49, 0xac, 0x4c, 0xc9}
};

/********************************/
/*          Functions           */
/********************************/
#if (TRANSITION_TO_RMA)
/******************************************************************************
 * Function Name: TransitionToRMA
 ******************************************************************************
 * Summary:
 *  This function invokes the RMA syscall via IPC, and checks for the return
 *  code.
 *
 * Return:
 *  CY_IPC_DRV_SUCCESS on success
 *  CY_IPC_DRV_ERROR on failure
 *
 ******************************************************************************/
static uint32_t TransitionToRMA(const transit_rma_param_t *paramRMA)
{
    uint32_t result = FAILED;
    uint32_t elapsedTime = 0;

    /* Send the IPC message */
    if (Cy_IPC_Drv_SendMsgPtr(CY_IPC_STRUCT, CY_IPC_NOTIFY_STRUCT0, (const void*)paramRMA) == CY_IPC_DRV_SUCCESS)
    {
        /* Wait for the IPC structure to be freed */
        while (elapsedTime < IPC_STATUS_WAIT_TIME_S)
        {
            if (Cy_IPC_Drv_IsLockAcquired(CY_IPC_STRUCT) == false)
                break;

            /* Delay by 1s before checking again */
            Cy_SysLib_Delay(DELAY_IPC_STATUS_CHECK_MS);
            elapsedTime++;
        }

        /* The result of the SROM API call is returned to the opcode variable */
        if ((paramRMA->opcode & CY_OPCODE_STS_Msk) == CY_OPCODE_SUCCESS)
        {
            printf("\r\nTransition to RMA successful!!!");
        }
        else
        {
            printf("\r\nTransition to RMA Failed!!! ... ERR_STATUS: 0x%lx\r\n", paramRMA->opcode);
        }

        result = CY_IPC_DRV_SUCCESS;
    }
    else
        result = CY_IPC_DRV_ERROR;

    return result;
}
#endif /* TRANSITION_TO_RMA */

void ReadUniqueID(read_uid_param_t  *uid_param)
{

    uid_param->unique_id_2 = ((uint32_t) SFLASH_DIE_YEAR << CY_UNIQUE_ID_DIE_LOT_2_Pos) |
                 (((uint32_t)(((SFLASH_V1_Type *) SFLASH)->DIE_MONTH)) <<  CY_UNIQUE_ID_DIE_LOT_1_Pos) |
                 ((uint32_t)(((SFLASH_V1_Type *) SFLASH)->DIE_DAY));

    uid_param->unique_id_1 = (((uint32_t)SFLASH_DIE_MINOR & 1U) << (CY_UNIQUE_ID_DIE_MINOR_Pos - CY_UNIQUE_ID_DIE_X_Pos)) |
                 ((uint32_t) SFLASH_DIE_SORT << (CY_UNIQUE_ID_DIE_SORT_Pos  - CY_UNIQUE_ID_DIE_X_Pos)) |
                 ((uint32_t) SFLASH_DIE_Y << (CY_UNIQUE_ID_DIE_Y_Pos     - CY_UNIQUE_ID_DIE_X_Pos)) |
                 ((uint32_t) SFLASH_DIE_X);

    uid_param->unique_id_0 = ((uint32_t) SFLASH_DIE_WAFER << CY_UNIQUE_ID_DIE_WAFER_Pos) |
                 ((uint32_t) SFLASH_DIE_LOT(2U) << CY_UNIQUE_ID_DIE_LOT_2_Pos) |
                 ((uint32_t) SFLASH_DIE_LOT(1U) << CY_UNIQUE_ID_DIE_LOT_1_Pos) |
                 ((uint32_t) SFLASH_DIE_LOT(0U));

    printf("ReadUniqueID : Device unique ID is: 0x%08lx 0x%08lx 0x%08lx \r\n", uid_param->unique_id_0, uid_param->unique_id_1, uid_param->unique_id_2);
}

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  This function does the device initialization, frees the watchdog timer,
 *  creates the DFU task and calls the scheduler.
 *  It also transitions the device to RMA when the TRANSITION_TO_RMA is set.
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
    bool bitval_secure, bitval_normal, bitval_secdbg, bitval_rma;
    uint32_t bit_secure_stage = offsetof(cy_stc_efuse_data_t, LIFECYCLE_STAGE.SECURE);
    uint32_t bit_normal_stage = offsetof(cy_stc_efuse_data_t, LIFECYCLE_STAGE.NORMAL);
    uint32_t bit_secure_debug_stage = offsetof(cy_stc_efuse_data_t, LIFECYCLE_STAGE.SECURE_WITH_DEBUG);
    uint32_t bit_secure_rma = offsetof(cy_stc_efuse_data_t, LIFECYCLE_STAGE.RMA);

    /* Update watchdog timer to mark successful start up of application */
    cy_wdg_kick();
    cy_wdg_free();

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* Enable global interrupts */
    __enable_irq();

    (void) result; /* To avoid compiler warning in release build */

    cy_retarget_io_pdl_init(CY_RETARGET_IO_BAUDRATE);

    /* Get lifecycle states and access restrictions */
    Cy_EFUSE_GetEfuseBit(bit_normal_stage, &bitval_normal);
    Cy_EFUSE_GetEfuseBit(bit_secure_debug_stage, &bitval_secdbg);
    Cy_EFUSE_GetEfuseBit(bit_secure_stage, &bitval_secure);
    Cy_EFUSE_GetEfuseBit(bit_secure_rma, &bitval_rma);

    if(bitval_rma)
    {
        printf("Life Cycle Stage : RMA\r\n");
    }
    else if(bitval_secdbg)
    {
        printf("Life Cycle Stage : SECURE_DEBUG\r\n");
    }
    else if(bitval_secure)
    {
        printf("Life Cycle mStage : SECURE\r\n");
    }
    else
    {
        printf("Life Cycle Stage : NORMAL\r\n");
    }

    /* Read and print the 12byte Unique ID of silicon. It is required to generate RMA certificate */
    ReadUniqueID(&uid_param);

    /* Delay by 100ms to clear the print buffer before dfu_task */
    Cy_SysLib_Delay(100);

#if (TRANSITION_TO_RMA)
    uint32_t rma_api_result;

    rma_api_result = TransitionToRMA(&rmaParam);
    printf("\r\nTransitionToRMA IPC status: 0x%lx\r\n", rma_api_result);
#else
    /* Create one task:
     * dfu_task - task that configures the DFU to upgrade the firmware
     */
    xTaskCreate((void *)dfu_task, "DFU Task", DFU_TASK_STACK_SIZE, NULL, DFU_TASK_PRIORITY, NULL);

    /* Start the scheduler */
    vTaskStartScheduler();

    /********************** Should never get here ***************************/
    /* RTOS scheduler exited */
    /* Halt the CPU if scheduler exits */
    CY_ASSERT(0);
#endif

    for (;;);
}


/* [] END OF FILE */
