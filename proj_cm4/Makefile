################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# CM4 user project make file.
#
################################################################################
# \copyright
# Copyright 2018-2025, Cypress Semiconductor Corporation (an Infineon company)
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

include ../common.mk


################################################################################
# Basic Configuration
################################################################################

# If set to "1", it will merge all application hex files and generate a
# single binary.
PRODUCTION=0

# Name of combined production hex file
PRODUCTION_HEX_NAME=production

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# MCUboot Specific Configuration
###############################################################################

# Name of application (used to derive name of final linked file).
# 
# If APPNAME is edited, ensure to update or regenerate launch 
# configurations for your IDE. For BOOT images the app name is 
# "primary_app" and for UPGRADE images the app name is "primary_app_UPGRADE"
# with "_UPGRADE" suffix added as part of the postbuild steps. 
APPNAME=proj_cm4

# Starting address of the user app or the offset at which the header of an image
# will begin. Image = Header + App + TLV + Trailer. See MCUboot documenation for
# details. 
# New relocated address = ORIGIN + HEADER_OFFSET
# ORIGIN is defined in the linker script and is usually the address at the
# start of CM0+ image.
ifeq ($(IMG_TYPE), BOOT)
HEADER_OFFSET ?= 0
else
HEADER_OFFSET ?= $(shell expr $$(( $(MCUBOOT_SECONDARY_SLOT_START_ADDR) - $(MCUBOOT_PRIMARY_SLOT_START_ADDR) )) )
endif

# Path of the key file passed as a parameter to the Python module "imgtool" for
# signing the image. Name of the key file is set in shared/shared_config.mk
# as a common parameter to both the Bootloader app and User apps.
KEY_FILE_PATH=../proj_btldr_cm0p/keys

# Set the version of the app using the following three variables.
# This version information is passed to the Python module "imgtool" while
# signing the image in the post build step. Default values are set as follows.
ifeq ($(IMG_TYPE), BOOT)
APP_VERSION_MAJOR ?= 1
APP_VERSION_MINOR ?= 0
APP_VERSION_BUILD ?= 0
else
ifeq ($(IMG_TYPE), UPGRADE)
APP_VERSION_MAJOR ?= 2
APP_VERSION_MINOR ?= 0
APP_VERSION_BUILD ?= 0
endif
endif

#Ignore the build directory(if any) of other build mode to avoid linker error.
ifeq ($(IMG_TYPE), BOOT)
CY_IGNORE+=build/UPGRADE
else
ifeq ($(IMG_TYPE), UPGRADE)
CY_IGNORE+=build/BOOT
endif
endif

################################################################################
# Advanced Configuration
################################################################################

MCUBOOT_PATH=$(SEARCH_mcuboot)
MCUBOOT_CY_PATH=$(MCUBOOT_PATH)/boot/cypress
MCUBOOTAPP_PATH=$(MCUBOOT_CY_PATH)/MCUBootApp
MBEDTLS_PATH=$(MCUBOOT_PATH)/ext/mbedtls

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS+=FREERTOS

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=CM0P_SLEEP

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
# Add path to necessary MCUBoot source files
SOURCES+=$(wildcard $(MCUBOOT_PATH)/boot/bootutil/src/*.c)\
         $(wildcard $(MCUBOOT_CY_PATH)/cy_flash_pal/cy_flash_map.c)\
         $(MCUBOOT_CY_PATH)/libs/watchdog/watchdog.c\
         $(MCUBOOT_CY_PATH)/libs/retarget_io_pdl/cy_retarget_io_pdl.c

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
# Add path to include necessary MCUBoot header files
INCLUDES+=$(SEARCH_mcuboot)/boot/bootutil/include\
          $(SEARCH_mcuboot)/boot/bootutil/src\
          $(MCUBOOT_CY_PATH)/cy_flash_pal\
          $(MCUBOOT_CY_PATH)/cy_flash_pal/flash_psoc6/include\
          $(MCUBOOT_CY_PATH)/cy_flash_pal/flash_psoc6/flash_qspi\
          $(MCUBOOTAPP_PATH)\
          $(MCUBOOTAPP_PATH)/config\
          $(MBEDTLS_PATH)/include\
          $(MCUBOOT_CY_PATH)/libs/watchdog\
          $(MCUBOOT_CY_PATH)/libs/retarget_io_pdl

# Add additional defines to the build process (without a leading -D).
# Sets the CM4 application start address based on image type
ifeq ($(IMG_TYPE), BOOT)
DEFINES+=CY_CORTEX_M4_APPL_ADDR=$(CM4_APP_FLASH_START)
else
DEFINES+=\
CY_CORTEX_M4_APPL_ADDR=$(shell expr $$(( $(MCUBOOT_SECONDARY_SLOT_START_ADDR) + $(CM0P_APP_FLASH_SIZE) )) )
endif

# Add define for kit used
KIT_NAME=$(subst -,_,$(RENAMED_TARGET))
DEFINES+=$(KIT_NAME)

# Defines for app version
DEFINES+=APP_VERSION_MAJOR=$(APP_VERSION_MAJOR) \
         APP_VERSION_MINOR=$(APP_VERSION_MINOR) \
         APP_VERSION_BUILD=$(APP_VERSION_BUILD)
         
# Define the number of images supported in case of multi-image bootloading.
DEFINES+=MCUBOOT_IMAGE_NUMBER=$(MCUBOOT_IMAGE_NUMBER) 

# Define start addresses and size for the user application
ifeq ($(IMG_TYPE), BOOT)
DEFINES+=USER_APP_START=$(CM0P_APP_FLASH_START)\
         USER_APP_SIZE=$(MCUBOOT_SLOT_SIZE)
else
DEFINES+=USER_APP_START=$(CY_BOOT_SECONDARY_1_START_ADDRESS)\
         USER_APP_SIZE=$(MCUBOOT_SLOT_SIZE)
endif
# Define the flag to enable RMA
DEFINES+=TRANSITION_TO_RMA=0

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS=

# Additional / custom linker flags.
LDFLAGS+=

# Additional / custom libraries to link in to the application.
LDLIBS+=

# Path to the linker script to use (if empty, use the default linker script).
ifeq ($(TOOLCHAIN), GCC_ARM)
LINKER_SCRIPT=$(wildcard ./../shared/linker_script/TARGET_$(RENAMED_TARGET)/TOOLCHAIN_$(TOOLCHAIN)/COMPONENT_CM4/*.ld)
CY_ELF_TO_HEX_TOOL=$(CY_CROSSPATH)/bin/arm-none-eabi-objcopy
CY_ELF_TO_HEX_OPTIONS=-O ihex
else
$(error Only GCC_ARM is supported at this moment)
endif

# Erase value is 0 for internal flash
ERASED_VALUE=0

# add flag to imgtool if not using swap for upgrade
ifeq ($(SWAP_UPGRADE), 0)
UPGRADE_TYPE:=--overwrite-only
endif

# Set build directory for BOOT and UPGRADE images
CY_BUILD_LOCATION=./build/$(IMG_TYPE)
BINARY_OUT_PATH=$(CY_BUILD_LOCATION)/$(TARGET)/$(CONFIG)
IMGTOOL_PATH=$(MCUBOOT_PATH)/scripts/imgtool.py

# Set the build version for the user application
CY_BUILD_VERSION=$(APP_VERSION_MAJOR).$(APP_VERSION_MINOR).$(APP_VERSION_BUILD)

# Signing arguments used by the imgtool for signing images
SIGN_ARGS=sign --header-size $(MCUBOOT_HEADER_SIZE) --pad-header --align 8 -v $(CY_BUILD_VERSION) \
               -S $(MCUBOOT_SLOT_SIZE) -M $(MCUBOOT_MAX_IMG_SECTORS) $(UPGRADE_TYPE) -R $(ERASED_VALUE) \
               -k $(KEY_FILE_PATH)/$(SIGN_KEY_FILE_ECC).pem

# Add defines for BOOT and UPGRADE types. For Upgrade, image is padded
# with a header and is marked as confirmed, Product ID is defined to
# be used for generating CYACD2 upgrade file. 
ifeq ($(IMG_TYPE), BOOT)
    DEFINES+=BOOT_IMAGE
    IMG_EXT=_$(IMG_TYPE)
else 
ifeq ($(IMG_TYPE), UPGRADE)
    DEFINES+=UPGRADE_IMAGE
    SIGN_ARGS += --pad --confirm
    DFU_PRODUCT_ID=0x01020304
    IMG_EXT=_$(IMG_TYPE)
else
    $(error Set IMG_TYPE to be either BOOT or UPGRADE)
endif
endif

# Custom pre-build commands to run.
PREBUILD+=

# Path to CyMCUElftool
CY_MCUELFTOOL_DIR=$(wildcard $(CY_TOOLS_DIR)/cymcuelftool-*)
MCUELFTOOL_LOC=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool

# Path to bootloader project binary
BOOTLOADER_PROJ_NAME=proj_btldr_cm0p
BOOTLOADER_BINARY_PATH=../$(BOOTLOADER_PROJ_NAME)/build/$(TARGET)/$(CONFIG)/$(BOOTLOADER_PROJ_NAME)

# Path to CM0P project binary
CM0P_PROJ_NAME=proj_cm0p
CM0P_BINARY_PATH=../$(CM0P_PROJ_NAME)/build/$(TARGET)/$(CONFIG)/$(CM0P_PROJ_NAME)

# Path to CM4 project binary
CM4_BINARY_PATH=./build/$(IMG_TYPE)/$(TARGET)/$(CONFIG)/$(APPNAME)

# Combined dual-app name and path
DUAL_APP_HEX_NAME=primary_app
DUAL_APP_HEX_PATH=$(BINARY_OUT_PATH)/$(DUAL_APP_HEX_NAME)

# Production file name and path
PRODUCTION_HEX_PATH=$(BINARY_OUT_PATH)/$(PRODUCTION_HEX_NAME)

# Path to Hex to CYACD2 conversion script
DFU_HEX2CYACD_SCRIPT=../$(BOOTLOADER_PROJ_NAME)/scripts/hextocyacd2.py

CY_MCUELFTOOL_DIR=$(wildcard $(CY_TOOLS_DIR)/cymcuelftool-*)
MCUELFTOOL_LOC=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool

SREC_CAT_LOC=$(CY_TOOLS_DIR)/srecord/bin/srec_cat

# Python path definition
CY_PYTHON_REQUIREMENT=true

# Custom post-build commands to run.
# 1. Take a backup of the original hex file (_raw.hex)
# 2. Delete the original hex
# 3. Relocate the starting address based on HEADER_OFFSET and also convert
#    .elf to _unsigned.hex
# 4. Sign the image using imgtool (.hex)
# 5. Create a copy of the hex file. For IMG_TYPE=BOOT, the file is named
# primary_app_BOOT.hex and for IMG_TYPE=UPGRADE, the file is named 
# primary_app_UPGRADE.hex.
#
# Step 3 is done so that programmer tools can place the image directly into
# secondary slot. This step is not required if an application (e.g. OTA) is
# going to write the image into secondary slot. 
#
# Note that the final file should be $(APPNAME).hex so that Eclipse and
# Make CLI can locate the file for programming.
POSTBUILD+=\
$(MCUELFTOOL_LOC) --merge $(CM0P_BINARY_PATH).elf $(CM4_BINARY_PATH).elf --output \
    $(DUAL_APP_HEX_PATH).elf --hex $(DUAL_APP_HEX_PATH).hex; \
cp -f $(DUAL_APP_HEX_PATH).hex $(DUAL_APP_HEX_PATH)$(IMG_EXT)_raw.hex;\
rm -f $(DUAL_APP_HEX_PATH).hex;\
$(CY_ELF_TO_HEX_TOOL) --change-addresses=$(HEADER_OFFSET) $(CY_ELF_TO_HEX_OPTIONS) $(DUAL_APP_HEX_PATH).elf $(DUAL_APP_HEX_PATH)$(IMG_EXT)_unsigned.hex;\
$(CY_PYTHON_PATH) $(IMGTOOL_PATH) $(SIGN_ARGS) $(DUAL_APP_HEX_PATH)$(IMG_EXT)_unsigned.hex $(DUAL_APP_HEX_PATH).hex; \
cp -f $(DUAL_APP_HEX_PATH).hex $(DUAL_APP_HEX_PATH)$(IMG_EXT).hex;

# For Upgrade images, the following post-build command is required for
# creating the CYACD2 upgrade file. The hextocyacd2.py script is used
# for this purpose.
ifeq ($(IMG_TYPE), UPGRADE)
POSTBUILD+=$(CY_PYTHON_PATH) $(DFU_HEX2CYACD_SCRIPT) $(DUAL_APP_HEX_PATH)$(IMG_EXT).hex $(DUAL_APP_HEX_PATH)$(IMG_EXT).cyacd2 \
           -row=512 -chk=sum -id=$(DFU_PRODUCT_ID) -size=$(MCUBOOT_SLOT_SIZE);
POSTBUILD+=rm -f $(DUAL_APP_HEX_PATH).hex;
endif

# Concatenates the hex files of all the application into a single binary
# required during production
ifeq ($(IMG_TYPE), BOOT)
ifeq ($(PRODUCTION), 1)
POSTBUILD+=$(SREC_CAT_LOC) $(BOOTLOADER_BINARY_PATH).hex -intel $(DUAL_APP_HEX_PATH).hex -intel -o \
           $(PRODUCTION_HEX_PATH).hex -intel --Output_Block_Size 16 ;
endif
endif

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../../

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# Absolute path to the compiler's "bin" directory. The variable name depends on the
# toolchain used for the build. Refer to the ModusToolbox user guide to get the correct
# variable name for the toolchain used in your build.
#
# The default path depends on the selected TOOLCHAIN and is set in the Make recipe.
CY_COMPILER_GCC_ARM_DIR?=

include $(CY_TOOLS_DIR)/make/start.mk
