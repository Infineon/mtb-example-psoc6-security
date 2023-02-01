/*******************************************************************************
* \file dfu_user.c
* \version 4.20
*
* This file provides the custom API for a firmware application with
* DFU SDK.
* - Cy_DFU_ReadData (address, length, ctl, params) - to read  the NVM block
* - Cy_Bootalod_WriteData(address, length, ctl, params) - to write the NVM block
*
********************************************************************************
* \copyright
* (c) (2016-2022), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation. All rights reserved.
********************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation or one of its
* affiliates ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
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
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

/* Include header files */
#include "cy_pdl.h"
#include "cy_flash.h"
#include "cy_dfu.h"
#include "../proj_btldr_cm0p/keys/ecc-public-key-p256.h"

#if (CY_IP_MXCRYPTO == 0u)
#error "Device does not support Crypto HW block. Use a different device \
that supports Crypto HW or modify the DFU validation code to use \
MbedTLS software implementation (out of scope of this code example)."
#endif

static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple);
static cy_rslt_t extract_pub_key(char *pub_key_der_in, uint8_t length, char *pub_key_out);
static cy_rslt_t signature_der_to_asn1(uint8_t *sign_in, uint8_t *sign_out);
static void calculate_sha256_digest(uint8_t* message_start_addr, uint32_t message_size, uint8_t* calc_sha256_digest);

/*******************************************************************************
* Function Name: IsMultipleOf
********************************************************************************
*
* This internal function check if value parameter is a multiple of parameter
* multiple
*
* \param value      value that will be checked
* \param multiple   value with which value is checked
*
* \return 1 - value is multiple of parameter multiple, else 0
*
*******************************************************************************/
static uint32_t IsMultipleOf(uint32_t value, uint32_t multiple)
{
    return ( ((value % multiple) == 0U)? 1UL : 0UL);
}

/*******************************************************************************
* Function Name: Cy_DFU_WriteData
********************************************************************************
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_WriteData (uint32_t address, uint32_t length, uint32_t ctl,
        cy_stc_dfu_params_t *params)
{
    /* application flash limits */
    /* Note that bootloader is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOT_BOOTLOADER_SIZE;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;
    /* EM_EEPROM Limits*/
    const uint32_t minEmEepromAddress = CY_EM_EEPROM_BASE;
    const uint32_t maxEmEepromAddress = CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE;

    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    /* Set primary slot application range */
    uint32_t startAddress = CY_DFU_APP0_VERIFY_START;
    uint32_t endAddress = CY_DFU_APP0_VERIFY_START + CY_DFU_APP0_VERIFY_LENGTH;

    /* Check if the address  and length are valid
     * Note Length = 0 is valid for erase command */
    if ( (IsMultipleOf(address, CY_FLASH_SIZEOF_ROW) == 0U) ||
            ( (length != CY_FLASH_SIZEOF_ROW) && ( (ctl & CY_DFU_IOCTL_ERASE) == 0U) ) )
    {
        status = CY_DFU_ERROR_LENGTH;
    }

    /* Refuse to write to a row within a range of primary slot application */
    if ( (startAddress <= address) && (address < endAddress) )
    {   /* It is forbidden to overwrite the currently running application */
        status = CY_DFU_ERROR_ADDRESS;
    }

    /* Check if the address is inside the valid range */
    if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) )
            || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
    {   /* Do nothing, this is an allowed memory range to update to */
    }
    else
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    if (status == CY_DFU_SUCCESS)
    {
        if ((ctl & CY_DFU_IOCTL_ERASE) != 0U)
        {
            (void) memset(params->dataBuffer, 0, CY_FLASH_SIZEOF_ROW);
        }

        cy_en_flashdrv_status_t fstatus =  Cy_Flash_WriteRow(address, (uint32_t*)params->dataBuffer);
        status = (fstatus == CY_FLASH_DRV_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_DATA;
    }
    return (status);
}


/*******************************************************************************
* Function Name: Cy_DFU_ReadData
********************************************************************************
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_ReadData (uint32_t address, uint32_t length, uint32_t ctl,
        cy_stc_dfu_params_t *params)
{
    /* application flash limits */
    /* Note that bootloader is out of range */
    const uint32_t minUFlashAddress = CY_FLASH_BASE + CY_BOOT_BOOTLOADER_SIZE;
    const uint32_t maxUFlashAddress = CY_FLASH_BASE + CY_FLASH_SIZE;
    /* EM_EEPROM Limits*/
    const uint32_t minEmEepromAddress = CY_EM_EEPROM_BASE;
    const uint32_t maxEmEepromAddress = CY_EM_EEPROM_BASE + CY_EM_EEPROM_SIZE;

    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    /* Check if the length is valid */
    if (IsMultipleOf(length, CY_FLASH_SIZEOF_ROW) == 0U)
    {
        status = CY_DFU_ERROR_LENGTH;
    }

    /* Check if the address is inside the valid range */
    if ( ( (minUFlashAddress <= address) && (address < maxUFlashAddress) )
            || ( (minEmEepromAddress <= address) && (address < maxEmEepromAddress) )  )
    {   /* Do nothing, this is an allowed memory range to update to */
    }
    else
    {
        status = CY_DFU_ERROR_ADDRESS;
    }

    /* Read or Compare */
    if (status == CY_DFU_SUCCESS)
    {
        if ((ctl & CY_DFU_IOCTL_COMPARE) == 0U)
        {
            (void) memcpy(params->dataBuffer, (const void *)address, length);
            status = CY_DFU_SUCCESS;
        }
        else
        {
            status = ( memcmp(params->dataBuffer, (const void *)address, length) == 0 )
                             ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
        }
    }
    return (status);
}

/******************************************************************************
 * Function Name: Cy_DFU_GetAppMetadata
 ******************************************************************************
 * Summary:
 *  This is a weak function that is part of the DFU middleware that is
 *  overridden here to not use the metadata section. Since imgtool already
 *  adds a trailer to the image, there is no need for maintaining a
 *  separate metadata again.
 *
 * Parameters:
 *  appID - ID of the application to be updated
 *  *params - pointer to DFU parameters structure (cy_stc_dfu_params_t)
 *
 * Return:
 *  cy_en_dfu_status_t - DFU operation status
 *
 ******************************************************************************/
cy_en_dfu_status_t Cy_DFU_GetAppMetadata(uint32_t appId, uint32_t *verifyAddress, uint32_t *verifySize)
{
    /*
     * This function doesn't do anything, it just returns success as the status.
     * The purpose of this code is to disable compiler warnings for Non-optimized
     * builds which do not remove unused functions and require them for the
     * completeness of the linking step.
     */
    (void) appId;
    (void) verifyAddress;
    (void) verifySize;

    return CY_DFU_SUCCESS;
}

/******************************************************************************
 * Function Name: Cy_DFU_ValidateApp
 ******************************************************************************
 * Summary:
 *  This is a weak function that is part of the DFU middleware that is
 *  overridden here to support the validation of imgtool signed images.
 *
 * Note: Image should have the following structure:
 * +---------------------------+
 * | Header                    |
 * +---------------------------+
 * | Payload                   |
 * +---------------------------+
 * | TLV area                  |
 * | +-----------------------+ |
 * | | TLV area header       | |
 * | +-----------------------+ |
 * | | SHA256 hash           | |
 * | +-----------------------+ |
 * | | Keyhash               | |
 * | +-----------------------+ |
 * | | Signature             | |
 * | +-----------------------| |
 * +---------------------------+
 *
 * If the protected_tlv is added, this function must be modified
 * appropriately.
 *
 * Parameters:
 *  appID - ID of the application to be updated
 *  *params - pointer to DFU parameters structure (cy_stc_dfu_params_t)
 *
 * Return:
 *  cy_en_dfu_status_t - DFU operation status
 *
 ******************************************************************************/
cy_en_dfu_status_t Cy_DFU_ValidateApp(uint32_t appId, cy_stc_dfu_params_t *params)
{
    uint32_t secondary_slot_start_addr = CY_DFU_APP1_VERIFY_START;
    uint32_t secondary_image_size = 0;
    uint16_t *p_trailer;
    uint32_t trailer_start_addr;
    uint32_t image_magic = 0, header_size = 0;
    uint8_t signature_length;
    uint32_t crypto_status;
    uint8_t validation_status;

#define ECC_CURVE_WIDTH          (256u)
#define VALIDATION_FAILED        (0u)
#define ECC_PUB_KEY_SIZE         (0x40)
#define WORD_LEN                 (4)
#define COMPARE_EQUAL            (0)

    CY_ALIGN(4) uint8_t trailer_hash[CY_CRYPTO_SHA256_DIGEST_SIZE];
    CY_ALIGN(4) uint8_t ecdsa_signature[CY_CRYPTO_BYTE_SIZE_OF_BITS(ECC_CURVE_WIDTH) * 2];

    extern const unsigned char ecdsa_pub_key[];
    extern const unsigned int ecdsa_pub_key_len;
    CY_ALIGN(4) unsigned char ecdsa_pub_x_y[ECC_PUB_KEY_SIZE];

    /* Get header magic */
    memcpy(&image_magic, (void *) secondary_slot_start_addr + HEADER_MAGIC_OFFSET, WORD_LEN);

    /* Check if image header magic is valid */
    if(image_magic != IMAGE_MAGIC)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    /* Get the header size */
    memcpy(&header_size, (void *) secondary_slot_start_addr + HEADER_SIZE_OFFSET, WORD_LEN);

    /* Make sure the header size matches with what's defined in the makefile */
    if(header_size != MCUBOOT_HEADER_SIZE)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    /* Get the size of the image payload */
    memcpy(&secondary_image_size, (void *) secondary_slot_start_addr + IMAGE_SIZE_OFFSET, WORD_LEN);

    /* Get trailer start address (image size + header size) */
    trailer_start_addr = (uint32_t) (secondary_slot_start_addr + secondary_image_size + header_size);
    p_trailer = (uint16_t *) trailer_start_addr;

    /* Verify trailer magic */
    if(*(p_trailer + TLV_MAGIC_OFFSET) != IMAGE_TLV_INFO_MAGIC)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    /* Check if the trailer contains SHA256 hash */
    if(*(p_trailer + TLV_HASH_TYPE_OFFSET) != IMAGE_TLV_SHA256)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    /* Check if the trailer SHA256 hash length is valid */
    if(*(p_trailer + TLV_HASH_LEN_OFFSET) != CY_CRYPTO_SHA256_DIGEST_SIZE)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    /* Copy the SHA256 hash into a buffer */
    memcpy(trailer_hash, p_trailer + TLV_HASH_OFFSET, CY_CRYPTO_SHA256_DIGEST_SIZE);

    /* Check if the trailer contains ECDSA signature */
    if(*(p_trailer + TLV_SIG_TYPE_OFFSET) != IMAGE_TLV_ECDSA256)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    /* Get the ECDSA DER signature length */
    signature_length = (uint8_t) *(p_trailer + TLV_SIG_LEN_OFFSET);

    /* Add 1 to signature length if the length is odd */
    if(signature_length % 2 != 0){
        ++signature_length;
    }

    /* Create a buffer for the ECDSA signature */
    uint8_t ecdsa_der_signature[signature_length];

    /* Copy the SHA256 hash into a buffer */
    memcpy(ecdsa_der_signature, p_trailer + TLV_SIG_OFFSET, signature_length);

    /* Convert the signature from DER to ASN.1 format */
    signature_der_to_asn1((uint8_t *) &ecdsa_der_signature, (uint8_t *) &ecdsa_signature);

    /* Extract the public key from the private key */
    extract_pub_key((char *) &ecdsa_pub_key, ecdsa_pub_key_len, (char *) &ecdsa_pub_x_y);

    /* Assigns the x and y points from the public key */
    CY_ALIGN(4) cy_stc_crypto_ecc_point pub_key = {
            .x = &ecdsa_pub_x_y,
            .y = &ecdsa_pub_x_y[CY_CRYPTO_BYTE_SIZE_OF_BITS(ECC_CURVE_WIDTH)]
    };

    /* Creates the key structure with ECC parameters */
    CY_ALIGN(4) const cy_stc_crypto_ecc_key ecc_key = {
            .type = PK_PUBLIC,
            .curveID = CY_CRYPTO_ECC_ECP_SECP256R1,
            .pubkey = pub_key
    };

    /* Enable the Crypto HW for image validation */
    crypto_status = Cy_Crypto_Core_Enable(CRYPTO);
    
    if(crypto_status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(crypto_status == CY_CRYPTO_SUCCESS);
    }

    /* Variables to calculate the hash of image + header */
    uint32_t message_size = secondary_image_size + header_size;
    uint8_t calc_sha256_digest[32] = {0};
    volatile uint32_t compare_result;

    /* Calculate the SHA256 hash of image + header */
    calculate_sha256_digest((uint8_t *) secondary_slot_start_addr, message_size, calc_sha256_digest);

    /* Check if the calculated SHA matches the one in the trailer */
    compare_result = Cy_Crypto_Core_MemCmp(CRYPTO, calc_sha256_digest, trailer_hash,
    CY_CRYPTO_SHA256_DIGEST_SIZE);

    /* Check if the hash in the trailer is equal to the calculated hash */
    if(compare_result != COMPARE_EQUAL)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    /* Convert the keys into little endian format */
    Cy_Crypto_Core_InvertEndianness((uint16_t *) ecc_key.pubkey.x, CY_CRYPTO_BYTE_SIZE_OF_BITS(ECC_CURVE_WIDTH));
    Cy_Crypto_Core_InvertEndianness((uint16_t *) ecc_key.pubkey.y, CY_CRYPTO_BYTE_SIZE_OF_BITS(ECC_CURVE_WIDTH));

    /* Convert the signature into little endian format */
    Cy_Crypto_Core_InvertEndianness(&ecdsa_signature, CY_CRYPTO_BYTE_SIZE_OF_BITS(ECC_CURVE_WIDTH));
    Cy_Crypto_Core_InvertEndianness(&ecdsa_signature[CY_CRYPTO_BYTE_SIZE_OF_BITS(ECC_CURVE_WIDTH)], CY_CRYPTO_BYTE_SIZE_OF_BITS(ECC_CURVE_WIDTH));

    /* Verify ECC hash */
    crypto_status = Cy_Crypto_Core_ECC_VerifyHash(CRYPTO, ecdsa_signature, calc_sha256_digest, CY_CRYPTO_SHA256_DIGEST_SIZE, &validation_status, &ecc_key);

    /* Check crypto operation status */
    if(crypto_status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Check validation status */
    if(validation_status == VALIDATION_FAILED)
    {
        return CY_DFU_ERROR_VERIFY;
    }

    return CY_DFU_SUCCESS;
}

/******************************************************************************
 * Function Name: calculate_sha256_digest
 ******************************************************************************
 * Summary:
 *  This function calculates the SHA256 digest.
 *
 * Parameters:
 *  *message_start_addr - Starting address of the message to be hashed
 *  message_size - The size of the message
 *  *calc_sha256_digest - Stores the result of the SHA256 hash operation
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void calculate_sha256_digest(uint8_t* message_start_addr, uint32_t message_size, uint8_t* calc_sha256_digest)
{
    cy_stc_crypto_sha_state_t hash_state = { 0 };
    uint8_t* message_ptr;
    message_ptr = (uint8_t *) message_start_addr;

#if defined(CY_CRYPTO_CFG_HW_V2_ENABLE)
    cy_stc_crypto_v2_sha256_buffers_t sha_256_buffers = {0};
#else
    cy_stc_crypto_v1_sha256_buffers_t sha_256_buffers = {0};
#endif

    uint32_t crypto_status;

    /* Initialize the Crypto SHA operation */
    crypto_status = Cy_Crypto_Core_Sha_Init(CRYPTO, &hash_state, CY_CRYPTO_MODE_SHA256, &sha_256_buffers);

    /* Start the SHA256 hash operation */
    if (CY_CRYPTO_SUCCESS == crypto_status)
    {
        crypto_status = Cy_Crypto_Core_Sha_Start(CRYPTO, &hash_state);
    }

    /* Process all chunks of the message */
    while ((message_size != 0) && (CY_CRYPTO_SUCCESS == crypto_status))
    {
        uint32_t chunk_size = (message_size >= CY_CRYPTO_SHA256_BLOCK_SIZE) ?
        CY_CRYPTO_SHA256_BLOCK_SIZE : message_size;

        crypto_status = Cy_Crypto_Core_Sha_Update(CRYPTO, &hash_state, message_ptr, chunk_size);

        message_ptr += chunk_size;
        message_size -= chunk_size;
    }

    /* Complete the SHA256 operation */
    if (CY_CRYPTO_SUCCESS == crypto_status)
    {
        crypto_status = Cy_Crypto_Core_Sha_Finish(CRYPTO, &hash_state, calc_sha256_digest);
    }

    /* Clear the used buffers */
    if (CY_CRYPTO_SUCCESS == crypto_status)
    {
        crypto_status = Cy_Crypto_Core_Sha_Free(CRYPTO, &hash_state);
    }
}

/******************************************************************************
 * Function Name: extract_pub_key
 ******************************************************************************
 * Summary:
 *  This function extracts the ECC public key x and y points from an ECC
 *  public key encoded in DER format.
 *
 * Parameters:
 *  *pub_key_der_in - The ECC public key in DER format
 *  length - The length of the ECC public key in DER format
 *  *pub_key_out - The ECC public key x and y values
 *
 * Return:
 *  cy_rslt_t - Operation status
 *
 ******************************************************************************/
static cy_rslt_t extract_pub_key(char *pub_key_der_in, uint8_t length, char *pub_key_out)
{
    uint8_t bytes_read = length;

#define BIT_STREAM_MARKER      (0x42)
#define PUB_KEY_IDENTIFIER_HI  (0x00)
#define PUB_KEY_IDENTIFIER_LO  (0x04)
#define ECC_PUB_KEY_SIZE       (0x40)
#define CY_RSLT_FAILURE        (0x01)

    while(bytes_read != 0)
    {
        if(*pub_key_der_in == BIT_STREAM_MARKER)
        {
            pub_key_der_in++;
            if(*pub_key_der_in == PUB_KEY_IDENTIFIER_HI )
            {
                pub_key_der_in++;
                if(*pub_key_der_in == PUB_KEY_IDENTIFIER_LO)
                {
                    pub_key_der_in++;
                    memcpy(pub_key_out, pub_key_der_in, ECC_PUB_KEY_SIZE);
                    return CY_RSLT_SUCCESS;
                }
                else
                {
                    pub_key_der_in--;
                }
            }
            else
            {
                pub_key_der_in--;
            }
        }

        pub_key_der_in++;
        bytes_read--;
    }

    return CY_RSLT_FAILURE;
}

/******************************************************************************
 * Function Name: signature_der_to_asn1
 ******************************************************************************
 * Summary:
 *  This function parses the ECDSA signature in DER format to give the ASN.1
 *  ECDSA signature values.
 *
 * Parameters:
 *  *sign_in - The ECDSA signature in DER format
 *  *sign_out - The ECDSA signature in ASN.1 format
 *
 * Return:
 *  cy_rslt_t - Operation status
 *
 ******************************************************************************/
static cy_rslt_t signature_der_to_asn1(uint8_t *sign_in, uint8_t *sign_out)
{

#define CRYPTO_SIGN_R_OFFSET     (4)
#define CRYPTO_SIGN_RS_OFFSET    (2)
#define CRYPTO_SIGN_RS_SIZE      (32)

#define CY_SIG_DER_PREFIX        (0x30U)
#define CY_SIG_DER_MARKER        (0x02U)
#define CY_RSLT_FAILURE          (0x01)

    /* DER signature representation
     *
     * 0x30 - DER prefix
     * 0x45 - Length of rest of Signature
     * 0x02 - Marker for r value
     * 0x21 - Length of r value (0x1F - 0x21)
     * 00ed...8f - r value, Big Endian
     * 0x02 - Marker for s value
     * 0x21 - Length of s value (0x1F - 0x21)
     * 7a98...ed - s value, Big Endian
     * */
    int32_t r_len=0, s_len=0;
    int32_t r_offset=0, s_offset=0;
    int32_t sign_idx = 0, sign_len = 0;

    /* check prefix && check r-marker */
    if(CY_SIG_DER_PREFIX == sign_in[sign_idx])
    {
        sign_idx++;

        sign_len = (int32_t)sign_in[sign_idx];
        sign_idx++;

        (void)memset(sign_out, 0x00, CRYPTO_SIGN_RS_SIZE * 2);

        if(CY_SIG_DER_MARKER == sign_in[sign_idx])
        {
            sign_idx++;

            r_len = (int32_t)sign_in[sign_idx];
            sign_idx++;

            r_offset = sign_idx;
            sign_idx += r_len;

            if(CY_SIG_DER_MARKER == sign_in[sign_idx])
            {

                sign_idx++;

                s_len = (int32_t)sign_in[sign_idx];
                sign_idx++;

                s_offset = sign_idx;
                sign_idx += s_len;

                /* check TLV length :
                 * r_len - 1 byte
                 * s_len - 1 byte
                 * r-tag - 1 byte
                 * s-tag - 1 byte */
                /* Check R and S size with optional trailing zero */
                if((r_len  + s_len + 4) == sign_len )
                {
                    /* ASN.1 signature representation
                     *
                     * ECDSASignature ::= SEQUENCE
                     * {
                     *      r   INTEGER,
                     *      s   INTEGER
                     * }
                     * */
                    if (r_len > CRYPTO_SIGN_RS_SIZE)
                    {
                        r_offset += r_len - CRYPTO_SIGN_RS_SIZE;
                        r_len = CRYPTO_SIGN_RS_SIZE;
                    }

                    if (s_len > CRYPTO_SIGN_RS_SIZE)
                    {
                        s_offset += s_len - CRYPTO_SIGN_RS_SIZE;
                        s_len = CRYPTO_SIGN_RS_SIZE;
                    }

                    memcpy(sign_out + CRYPTO_SIGN_RS_SIZE - r_len, sign_in + r_offset, r_len);
                    memcpy(sign_out + CRYPTO_SIGN_RS_SIZE * 2 - s_len, sign_in + s_offset, s_len);
                    return CY_RSLT_SUCCESS;
                }
            }
        }
    }

    return CY_RSLT_FAILURE;
}


/* [] END OF FILE */
