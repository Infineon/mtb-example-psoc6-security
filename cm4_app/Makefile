################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# CM4 user application make file.
#
################################################################################
# \copyright
# Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company)
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
# Basic Configuration
################################################################################

# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager 
# ('make modlibs' from command line), which will also update Eclipse IDE launch 
# configurations. If TARGET is manually edited, ensure TARGET_<BSP>.mtb with a 
# valid URL exists in the application, run 'make getlibs' to fetch BSP contents
# and update or regenerate launch configurations for your IDE.
TARGET=CY8CKIT-062-WIFI-BT

# Name of toolchain to use. Options include:
#
# GCC_ARM -- GCC provided with ModusToolbox IDE
# ARM     -- ARM Compiler (must be installed separately)
# IAR     -- IAR Compiler (must be installed separately)
#
# See also: CY_COMPILER_PATH below
TOOLCHAIN=GCC_ARM

# Default build configuration. Options include:
#
# Debug -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS
# 
# If CONFIG is manually edited, ensure to update or regenerate launch configurations 
# for your IDE.
CONFIG=Debug

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

# Include the shared configuration make file
include ../shared/shared_config.mk

# Name of application (used to derive name of final linked file).
# 
# If APPNAME is edited, ensure to update or regenerate launch 
# configurations for your IDE. For BOOT images the app name is 
# "primary_app" and for UPGRADE images the app name is "primary_app_UPGRADE"
# with "_UPGRADE" suffix added as part of the postbuild steps. 
APPNAME=primary_app

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
KEY_FILE_PATH=$(CY_INTERNAL_APPLOC)/../bootloader_cm0p/keys

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

################################################################################
# Advanced Configuration
################################################################################

MCUBOOT_PATH=../../mtb_shared/mcuboot/v1.7.2-cypress

MCUBOOT_CY_PATH=$(MCUBOOT_PATH)/boot/cypress
MCUBOOTAPP_PATH=$(MCUBOOT_CY_PATH)/MCUBootApp

MBEDTLS_PATH=$(MCUBOOT_PATH)/ext/mbedtls
CRYPTO_LIB_PATH=$(MBEDTLS_PATH)/crypto/library

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
COMPONENTS+=FREERTOS CUSTOM_DESIGN_MODUS

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=CM0P_SLEEP BSP_DESIGN_MODUS

# Add dependency to the custom CM0+ App
DEPENDENT_APP_PATHS+=../cm0p_app

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
# Add path to necessary MCUBoot source files
SOURCES+=$(wildcard $(MCUBOOT_PATH)/boot/bootutil/src/*.c)\
         $(wildcard $(MCUBOOT_CY_PATH)/cy_flash_pal/cy_flash_map.c)\
         $(MCUBOOT_CY_PATH)/libs/watchdog/watchdog.c

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
# Add path to include necessary MCUBoot header files
INCLUDES+=$(MCUBOOT_PATH)/boot/bootutil/include\
          $(MCUBOOT_PATH)/boot/bootutil/src\
          $(MCUBOOT_CY_PATH)/cy_flash_pal/include\
          $(MCUBOOT_CY_PATH)/cy_flash_pal/flash_qspi\
          $(MCUBOOTAPP_PATH)\
          $(MCUBOOTAPP_PATH)/config\
          $(MBEDTLS_PATH)/crypto/include\
          $(MCUBOOT_CY_PATH)/libs/watchdog \
          $(CY_INTERNAL_APPLOC)/../bootloader_cm0p/keys

# Add additional defines to the build process (without a leading -D).
# Sets the CM4 application start address based on image type
ifeq ($(IMG_TYPE), BOOT)
DEFINES+=CY_CORTEX_M4_APPL_ADDR=$(CM4_APP_FLASH_START)
else
DEFINES+=\
CY_CORTEX_M4_APPL_ADDR=$(shell expr $$(( $(MCUBOOT_SECONDARY_SLOT_START_ADDR) + $(CM0P_APP_FLASH_SIZE) )) )
endif

# Define for converting retarget IO line feed to carriage return line feed
DEFINES+=CY_RETARGET_IO_CONVERT_LF_TO_CRLF

# Add define for kit used
KIT_NAME=$(subst -,_,$(TARGET))
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
LINKER_SCRIPT=$(wildcard ./../shared/linker_script/TARGET_$(TARGET)/TOOLCHAIN_$(TOOLCHAIN)/COMPONENT_CM4/*.ld)
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
BINARY_OUT_PATH=$(CY_CONFIG_DIR)/$(APPNAME)
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
	DEFINES+=BOOT_IMG
	IMG_EXT=_$(IMG_TYPE)
else 
ifeq ($(IMG_TYPE), UPGRADE)
	DEFINES+=UPGRADE_IMG
	SIGN_ARGS += --pad --confirm
	DFU_PRODUCT_ID=0x01020304
	IMG_EXT=_$(IMG_TYPE)
else
	$(error Set IMG_TYPE to be either BOOT or UPGRADE)
endif
endif

# Python path definition
ifeq ($(OS),Windows_NT)
PYTHON_PATH?=python
else
PYTHON_PATH?=python3
endif

# Custom pre-build commands to run.
PREBUILD=

# Path to CyMCUElftool
MCUELFTOOL_LOC=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool

# Path to bootloader application
BOOTLOADER_APP_PATH=$(CY_INTERNAL_APPLOC)/../bootloader_cm0p

# Path to Hex to CYACD2 conversion script
DFU_HEX2CYACD_SCRIPT=$(BOOTLOADER_APP_PATH)/scripts/hextocyacd2.py

# Python path definition
ifeq ($(OS),Windows_NT)
PYTHON_PATH?=python
else
PYTHON_PATH?=python3
endif

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
POSTBUILD=\
cp -f $(BINARY_OUT_PATH).hex $(BINARY_OUT_PATH)$(IMG_EXT)_raw.hex;\
rm -f $(BINARY_OUT_PATH).hex;\
$(CY_ELF_TO_HEX_TOOL) --change-addresses=$(HEADER_OFFSET) $(CY_ELF_TO_HEX_OPTIONS) $(BINARY_OUT_PATH).elf $(BINARY_OUT_PATH)$(IMG_EXT)_unsigned.hex;\
$(PYTHON_PATH) $(IMGTOOL_PATH) $(SIGN_ARGS) $(BINARY_OUT_PATH)$(IMG_EXT)_unsigned.hex $(BINARY_OUT_PATH).hex; \
cp -f $(BINARY_OUT_PATH).hex $(BINARY_OUT_PATH)$(IMG_EXT).hex;

# For Upgrade images, the following post-build command is required for
# creating the CYACD2 upgrade file. The hextocyacd2.py script is used
# for this purpose.
ifeq ($(IMG_TYPE), UPGRADE)
POSTBUILD+=$(PYTHON_PATH) $(DFU_HEX2CYACD_SCRIPT) $(BINARY_OUT_PATH)$(IMG_EXT).hex $(BINARY_OUT_PATH)$(IMG_EXT).cyacd2 \
           -row=512 -chk=sum -id=$(DFU_PRODUCT_ID) -size=$(MCUBOOT_SLOT_SIZE);
POSTBUILD+=rm -f $(BINARY_OUT_PATH).hex;
endif

# Path to python script for merging hex files
MERGE_HEX_PYTHON_SCRIPT=$(BOOTLOADER_APP_PATH)/scripts/postbuild_merge_hex.py

# Concatenates the hex files of all the application into a single binary
# required during production
ifeq ($(IMG_TYPE), BOOT)
ifeq ($(PRODUCTION), 1)
POSTBUILD+=$(PYTHON_PATH) $(MERGE_HEX_PYTHON_SCRIPT) -t $(TARGET) -c $(CONFIG) --toolchain-path $(CY_TOOLS_PATHS)\
-o $(CY_CONFIG_DIR) -n $(PRODUCTION_HEX_NAME) --bootloader-app-path $(BOOTLOADER_APP_PATH) \
--bootloader-app-name bootloader_cm0p --user-dual-app-path $(CY_INTERNAL_APPLOC)\
--user-dual-app-name $(APPNAME)
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

# Absolute path to the compiler's "bin" directory.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# IDE provided compiler by default).
CY_COMPILER_PATH=


# Locate ModusToolbox IDE helper tools folders in default installation
# locations for Windows, Linux, and macOS.
CY_WIN_HOME=$(subst \,/,$(USERPROFILE))
CY_TOOLS_PATHS ?= $(wildcard \
    $(CY_WIN_HOME)/ModusToolbox/tools_* \
    $(HOME)/ModusToolbox/tools_* \
    /Applications/ModusToolbox/tools_*)

# If you install ModusToolbox IDE in a custom location, add the path to its
# "tools_X.Y" folder (where X and Y are the version number of the tools
# folder).
CY_TOOLS_PATHS+=

# Default to the newest installed tools folder, or the users override (if it's
# found).
CY_TOOLS_DIR=$(lastword $(sort $(wildcard $(CY_TOOLS_PATHS))))

ifeq ($(CY_TOOLS_DIR),)
$(error Unable to find any of the available CY_TOOLS_PATHS -- $(CY_TOOLS_PATHS))
endif

$(info Tools Directory: $(CY_TOOLS_DIR))

include $(CY_TOOLS_DIR)/make/start.mk
	
# Get libraries for CM0p App for building it
getlibs : getlibs_cm0p
getlibs_cm0p:
	$(MAKE) -C ../cm0p_app/ $(MAKECMDGOALS)
