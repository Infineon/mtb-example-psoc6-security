/******************************************************************************
* File Name: cy_ps_keystorage.h
*
* Description: Secure key storage header for application usage.
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

#ifndef CY_PS_KEY_STORAGE_H
#define CY_PS_KEY_STORAGE_H

#include <stdint.h>
#include <stddef.h>
#include "cy_syslib.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*               Macros
***************************************/

/* Macros used to define the user-defined key array */  
#define CY_PS_SECURE_KEY_LENGTH         (256u)  /* Key length (Bytes) */
#define CY_PS_SECURE_KEY_ARRAY_SIZE     (4u)    /* Number of Keys */

/* Macros used to define the Public key. */    
#define CY_PS_PUBLIC_KEY_RSA_2048       (0UL)   /* RSASSA-PKCS1-v1_5-2048 signature scheme */
#define CY_PS_PUBLIC_KEY_RSA_1024       (1UL)   /* RSASSA-PKCS1-v1_5-1024 signature scheme */
#define CY_PS_PUBLIC_KEY_STRUCT_OFFSET  (8UL)   /* Offset to public key struct in number of bytes */
#define CY_PS_PUBLIC_KEY_MODULOLENGTH   (256UL) /* Modulus length of the RSA key */
#define CY_PS_PUBLIC_KEY_EXPLENGTH      (32UL)  /* Exponent length of the RSA key */
#define CY_PS_PUBLIC_KEY_SIZEOF_BYTE    (8UL)   /* Size of Byte in number of bits */

/***************************************
*               Structs
***************************************/
/* Public key definition structure as expected by the Crypto driver */
typedef struct
{
    uint32_t moduloAddr;            /* Address of the public key modulus */
    uint32_t moduloSize;            /* Size (bits) of the modulus part of the public key */
    uint32_t expAddr;               /* Address of the public key exponent */
    uint32_t expSize;               /* Size (bits) of the exponent part of the public key */
    uint32_t barrettAddr;           /* Address of the Barret coefficient */
    uint32_t inverseModuloAddr;     /* Address of the binary inverse modulo */
    uint32_t rBarAddr;              /* Address of the (2^moduloLength mod modulo) */
} cy_ps_stc_crypto_public_key_t;

/* Public key structure */
typedef struct
{
    uint32_t objSize;                                           /* Public key Object size */
    uint32_t signatureScheme;                                   /* Signature scheme */
    cy_ps_stc_crypto_public_key_t publicKeyStruct;              /* Public key definition struct */
    uint8_t  moduloData[CY_PS_PUBLIC_KEY_MODULOLENGTH];         /* Modulo data */
    uint8_t  expData[CY_PS_PUBLIC_KEY_EXPLENGTH];               /* Exponent data */
    uint8_t  barrettData[CY_PS_PUBLIC_KEY_MODULOLENGTH + 4UL];  /* Barret coefficient data */
    uint8_t  inverseModuloData[CY_PS_PUBLIC_KEY_MODULOLENGTH];  /* Binary inverse modulo data */
    uint8_t  rBarData[CY_PS_PUBLIC_KEY_MODULOLENGTH];           /* 2^moduloLength mod modulo data */
} cy_ps_stc_public_key_t;

/***************************************
*               Globals
***************************************/
/* Secure Key Storage (Note: Ensure that the alignment matches the Protection unit configuration) */
extern const uint8_t CySecureKeyStorage[CY_PS_SECURE_KEY_ARRAY_SIZE][CY_PS_SECURE_KEY_LENGTH];

/* Public key in SFlash */
extern const cy_ps_stc_public_key_t cy_publicKey;

#if defined(__cplusplus)
}
#endif

#endif /* CY_PS_KEY_STORAGE_H */

/* [] END OF FILE */
