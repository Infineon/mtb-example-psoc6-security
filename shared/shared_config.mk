################################################################################
# \file shared_config.mk
# \version 1.0
#
# \brief
# Holds configuration and error checking that are common to both the MCUBoot 
# Bootloader and user apps. Ensure that this file is included in the
# application's Makefile. 
#
################################################################################
# \copyright
# Copyright 2020-2022, Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

################################################################################
# MCUboot Specific Configuration
###############################################################################

# Supported values: BOOT, UPGRADE
# The CM0+ Blinky app defines the LED period differently depending on whether the
# image is BOOT or UPGRADE. 
# BOOT: Use when image is built for the primary slot. 
#       --pad argument is not passed to the imgtool. 
# UPGRADE: Use when image is built for secondary slot. 
#          --pad argument is passed to the imgtool.
IMG_TYPE ?= BOOT

# use SWAP_UPGRADE = 0 for overwrite only mode, secondary image is simply
# copied to primary slot after successful validation.
# SWAP based upgrade is not supported in this code example.
SWAP_UPGRADE ?= 0

# Set to 1 when MCUboot needs to support external flash for the secondary slot.
# Using external flash is not supported in this code example.
# Default location is internal flash.
USE_EXT_FLASH ?= 0

# Use hardware accelerated Crypto for MbedTLS
USE_CRYPTO_HW ?= 1

# Override the default flash map used by MCUBoot
DEFINES+=CY_FLASH_MAP_EXT_DESC

# Add appropriate defines based on SWAP type.
# Only SWAP using overwrite is supported in this code example
ifeq ($(SWAP_UPGRADE), 0)
DEFINES+=MCUBOOT_OVERWRITE_ONLY
endif

# Name of the key file, used in two places. 
# 1. In the Bootloader app, used for user image authentication. 
# 2. Passed as a parameter to the Python module "imgtool" for signing the image
#    in the user app Makefile. The path of this key file is set in the CM4 user
#    app Makefile.   
SIGN_KEY_FILE_ECC=cypress-test-ec-p256

# Name of the key file, used in one place. 
# 1. Part of cy_ps_key_storage.c file and used for bootloader app validation
#    by the flashboot.
# 2. Passed as a parameter to the cymcuelftool module for signing the image
#    using RSA in the bootloader app Makefile. The path of this key file is 
#    set in the CM4 user app Makefile. 
SIGN_KEY_FILE_RSA=cypress-test-rsa-2048

################################################################################

# NOTE: Following variables are passed as options to the linker. 
# Ensure that the values have no trailing white space. Linker will throw error
# otherwise. 

# The bootloader app is run by CM0+. Therefore, the scratch size and the
# bootloader size are used with the linker script for the bootloader app. 
# The slot sizes (primary, secondary) are used with the linker script
# for the user apps run by CM0+ and CM4 CPUs. 

# Default Flash and SRAM beginning 
START_OF_FLASH=0x10000000
START_OF_SRAM=0x08000000

# Bootloader size = 112K
CM0P_BTLDR_FLASH_SIZE=0x1C000

# Protected Storage size = 16K
PROTECTED_MEM_SIZE=0x4000

# Flash and RAM size for MCUBoot Bootloader app run by CM0+
ifeq ($(TARGET), CY8CPROTO-062S3-4343W)
# CM0P App Flash Size = 64K
# CM4 App Flash Size = 128K
# CM0P Bootloader / App SRAM Size = 64K
# Shared SRAM Size = 32K
# CM4 App SRAM Size = 158K (160KB - 2KB reserved for system use) 
CM0P_APP_FLASH_SIZE=0x10000
CM4_APP_FLASH_SIZE=0x20000
CM0P_BTLDR_SRAM_SIZE=0x10000
CM0P_APP_SRAM_SIZE=0x10000
SHARED_SRAM_SIZE=0x8000
CM4_APP_SRAM_SIZE=0x27800
endif
ifeq ($(TARGET), $(filter $(TARGET), CY8CKIT-062-WIFI-BT CY8CKIT-062-BLE CY8CPROTO-063-BLE))
# CM0P App Flash Size = 128K
# CM4 App Flash Size = 320K
# CM0P Bootloader / App SRAM Size = 64K
# Shared SRAM Size = 32K
# CM4 App SRAM Size = 158K (160KB - 2KB reserved for system use) 
CM0P_APP_FLASH_SIZE=0x20000
CM4_APP_FLASH_SIZE=0x50000
CM0P_BTLDR_SRAM_SIZE=0x10000
CM0P_APP_SRAM_SIZE=0x10000
SHARED_SRAM_SIZE=0x8000
CM4_APP_SRAM_SIZE=0x27800
endif
ifeq ($(TARGET), $(filter $(TARGET), CY8CPROTO-062-4343W CY8CKIT-062S2-43012))
# CM0P App Flash Size = 128K
# CM4 App Flash Size = 768K
# CM0P Bootloader / App SRAM Size = 192K
# Shared SRAM Size = 64K
# CM4 App SRAM Size = 766K (768KB - 2KB reserved for system use)
CM0P_APP_FLASH_SIZE=0x20000
CM4_APP_FLASH_SIZE=0xC0000
CM0P_BTLDR_SRAM_SIZE=0x30000
CM0P_APP_SRAM_SIZE=0x30000
SHARED_SRAM_SIZE=0x10000
CM4_APP_SRAM_SIZE=0xBF800

# Size of the scratch area used by MCUboot while swapping the image between
# primary slot and secondary slot. Although defined, the SWAP using status
# is not supported in this code example. It is left to the user to 
# implement the same. 
MCUBOOT_SCRATCH_SIZE=0x20000
endif

# Variables for flash start addresses
CM0P_BTLDR_FLASH_START=$(START_OF_FLASH)
PROTECTED_MEM_START=$(shell expr $$(( $(CM0P_BTLDR_FLASH_START) + $(CM0P_BTLDR_FLASH_SIZE) )) )
CM0P_APP_FLASH_START=$(shell expr $$(( $(PROTECTED_MEM_START) + $(PROTECTED_MEM_SIZE) )) )
CM4_APP_FLASH_START=$(shell expr $$(( $(CM0P_APP_FLASH_START) + $(CM0P_APP_FLASH_SIZE) )) )

# Variables for SRAM start addresses
CM0P_BTLDR_SRAM_START=$(START_OF_SRAM)
CM0P_APP_SRAM_START=$(START_OF_SRAM)
SHARED_SRAM_START=$(shell expr $$(( $(START_OF_SRAM) + $(CM0P_APP_SRAM_SIZE))) )
CM4_APP_SRAM_START=$(shell expr $$(( $(SHARED_SRAM_START) + $(SHARED_SRAM_SIZE) )) )

# Slot size includes the flash size of both the CM0P App and the CM4 App
# One slot = MCUboot Header + App + TLV + Trailer (Trailer is not present for BOOT image)
# The slot size should be:  (Total_Flash - BOOTLOADER_APP_FLASH_SIZE)/2 rounded down to nearest 512 bytes.
# The RAM size of the User App is set in the linker script as given below.
#   - The CM0p SRAM is reused between bootloader the CM0p user app
#     CM0p RAM size = CM0P_APP_SRAM_SIZE = CM0P_BTLDR_SRAM_SIZE
#   - CM4 RAM size = Total RAM available - 2 KB reserved for system use - CM0P_APP_SRAM_SIZE.
MCUBOOT_SLOT_SIZE=$(shell expr $$(( $(CM0P_APP_FLASH_SIZE) + $(CM4_APP_FLASH_SIZE) )) )

# Total flash size = App1 size + App2 size
TOTAL_APP_FLASH_SIZE=$(shell expr $$(( $(MCUBOOT_SLOT_SIZE)*2)) )

# Slot addresses and sizes needed by MCUBoot
MCUBOOT_PRIMARY_SLOT_START_ADDR=$(CM0P_APP_FLASH_START)
MCUBOOT_SECONDARY_SLOT_START_ADDR=$(shell expr $$(( $(CM0P_APP_FLASH_START) + $(MCUBOOT_SLOT_SIZE) )) )

# Scratch start address needed for SWAP mode in 2M devices 
# Note: SWAP is only supported in 2M devices
ifeq ($(TARGET), $(filter $(TARGET), CY8CPROTO-062-4343W CY8CKIT-062S2-43012))
MCUBOOT_SCRATCH_START_ADDR=$(shell expr $$(( $(MCUBOOT_SECONDARY_SLOT_START_ADDR) + $(MCUBOOT_SLOT_SIZE) )) )
endif

# MCUBoot header size
# Must be a multiple of 1024 because of the following reason. 
# CM0p image starts right after the header and the CM0p image begins with the
# interrupt vector table. The starting address of the table must be 1024-bytes
# aligned. See README.md for details.
# Number of bytes to be aligned to = Number of interrupt vectors x 4 bytes.
# (1024 = 256 x 4)
# After the CM0p image comes the CM4 image. 
# 
# Header size is used in two places. 
# 1. The location of CM0p image is offset by the header size from the ORIGIN
# value specified in the linker script. 
# 2. Passed to the imgtool while signing the image. The imgtool fills the space
# of this size with zeroes and then adds the actual header from the beginning of
# the image.
MCUBOOT_HEADER_SIZE=0x400 

# Maximum number of flash sectors (or rows) per image slot.
# The maximum number of flash sectors for which swap status is tracked in
# the image trailer. 
# In the bootloader app, this value is used in DEFINE+= to override the macro
# with the same name in "mcuboot/boot/cypress/MCUBootApp/config/mcuboot_config/mcuboot_config.h".
# In the user app, this value is passed with "-M" option to the imgtool while 
# signing the image. imgtool adds padding in the trailer area
# depending on this value. 
# This value can be simply set to MCUBOOT_SLOT_SIZE/FLASH_ROW_SIZE.
# For PSoC 6, FLASH_ROW_SIZE=512 bytes.
MCUBOOT_SECTOR_SIZE = 512

# Minimum number of sectors accecpted by MCUBoot Library is 32
MIN_IMG_SECTORS = 32

# Number of flash sectors per image slot
FLASH_IMG_SECTORS = $(shell expr $$(( $(MCUBOOT_SLOT_SIZE) / $(MCUBOOT_SECTOR_SIZE) )) )

# Maximum number of flash sectors (or rows) per image slot.
MCUBOOT_MAX_IMG_SECTORS = $(shell expr $$(( $(MIN_IMG_SECTORS) > $(FLASH_IMG_SECTORS) ? $(MIN_IMG_SECTORS) : $(FLASH_IMG_SECTORS) )) )

# Number of images supported in case of multi-image bootloading. 
# This application supports only up to 1 image.
MCUBOOT_IMAGE_NUMBER ?= 1

# The following defines describe the flash map used by MCUBoot
DEFINES+=TARGET=$(TARGET) \
         CY_BOOT_BOOTLOADER_SIZE=$(CM0P_BTLDR_FLASH_SIZE) \
         CY_BOOT_PRIMARY_1_START_ADDRESS=$(MCUBOOT_PRIMARY_SLOT_START_ADDR) \
         CY_BOOT_PRIMARY_1_SIZE=$(MCUBOOT_SLOT_SIZE) \
         CY_BOOT_SECONDARY_1_START_ADDRESS=$(MCUBOOT_SECONDARY_SLOT_START_ADDR) \
         CY_BOOT_SECONDARY_1_SIZE=$(MCUBOOT_SLOT_SIZE) \
         CM0P_APP_FLASH_START=$(CM0P_APP_FLASH_START) \
         CM4_APP_FLASH_START=$(CM4_APP_FLASH_START) \
         PROTECTED_MEM_START=$(PROTECTED_MEM_START) \
         MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE) \
         CY_START_OF_FLASH=$(START_OF_FLASH) \
         CY_START_OF_SRAM=$(SRAM_OF_FLASH) \
         SHARED_SRAM_START=$(SHARED_SRAM_START) \
         CM4_APP_SRAM_START=$(CM4_APP_SRAM_START) \
         CM0P_BTLDR_SRAM_SIZE=$(CM0P_BTLDR_SRAM_SIZE)
         
ifeq ($(TARGET), $(filter $(TARGET), CY8CPROTO-062-4343W CY8CKIT-062S2-43012))
DEFINES+=CY_BOOT_SCRATCH_START_ADDRESS=$(MCUBOOT_SCRATCH_START_ADDR) \
         CY_BOOT_SCRATCH_SIZE=$(MCUBOOT_SCRATCH_SIZE)
endif

# Toolchain specific linker flags
ifeq ($(TOOLCHAIN), GCC_ARM)
LDFLAGS+=-Wl,--defsym=ORIGIN_OF_FLASH=$(START_OF_FLASH),--defsym=ORIGIN_OF_SRAM=$(START_OF_SRAM)
LDFLAGS+=-Wl,--defsym=CM0P_BTLDR_FLASH_SIZE=$(CM0P_BTLDR_FLASH_SIZE),--defsym=CM0P_APP_FLASH_SIZE=$(CM0P_APP_FLASH_SIZE),--defsym=CM4_APP_FLASH_SIZE=$(CM4_APP_FLASH_SIZE)
LDFLAGS+=-Wl,--defsym=SECONDARY_SLOT_FLASH_START=$(MCUBOOT_SECONDARY_SLOT_START_ADDR)
LDFLAGS+=-Wl,--defsym=PROT_STRG_SIZE=$(PROTECTED_MEM_SIZE),--defsym=PROTECTED_MEM_START=$(PROTECTED_MEM_START),--defsym=CM0P_APP_FLASH_START=$(CM0P_APP_FLASH_START)
LDFLAGS+=-Wl,--defsym=CM4_APP_FLASH_START=$(CM4_APP_FLASH_START),--defsym=CM0P_BTLDR_SRAM_SIZE=$(CM0P_BTLDR_SRAM_SIZE),--defsym=CM0P_APP_SRAM_SIZE=$(CM0P_APP_SRAM_SIZE),--defsym=SHARED_SRAM_SIZE=$(SHARED_SRAM_SIZE),--defsym=CM4_APP_SRAM_SIZE=$(CM4_APP_SRAM_SIZE)
LDFLAGS+=-Wl,--defsym=SHARED_SRAM_START=$(SHARED_SRAM_START),--defsym=CM4_APP_SRAM_START=$(CM4_APP_SRAM_START)
LDFLAGS+=-Wl,--defsym=MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE)
LDFLAGS+=-Wl,--defsym=MCUBOOT_SLOT_SIZE=$(MCUBOOT_SLOT_SIZE)
LDFLAGS+=-Wl,--defsym=TOTAL_APP_FLASH_SIZE=$(TOTAL_APP_FLASH_SIZE)

ifeq ($(TARGET), $(filter $(TARGET), CY8CPROTO-062-4343W CY8CKIT-062S2-43012))
LDFLAGS+=-Wl,--defsym=MCUBOOT_SCRATCH_START_ADDR=$(MCUBOOT_SCRATCH_START_ADDR),--defsym=MCUBOOT_SCRATCH_SIZE=$(MCUBOOT_SCRATCH_SIZE)
endif

else
$(error Only GCC_ARM is supported at this moment)
endif

# Check if SWAP upgrade is being requested on unsupported parts
ifeq ($(SWAP_UPGRADE), 1)
$(error SWAP Upgrade feature is not supported in this code example. It is left to the users to implement it.\
Refer to the README.md file for more information.)
endif
