################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# CM0+ bootloader application make file.
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
# Basic Configuration
################################################################################

# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager 
# ('make modlibs' from command line), which will also update Eclipse IDE launch 
# configurations. If TARGET is manually edited, ensure TARGET_<BSP>.mtb with a 
# valid URL exists in the application, run 'make getlibs' to fetch BSP contents
# and update or regenerate launch configurations for your IDE.
TARGET=CY8CKIT-062-WIFI-BT

# Name of application (used to derive name of final linked file).
APPNAME=bootloader_cm0p

# Name of toolchain to use. Options include:
#
# GCC_ARM -- GCC provided with ModusToolbox IDE
# ARM     -- ARM Compiler (must be installed separately)
# IAR     -- IAR Compiler (must be installed separately)
#
# See also: CY_COMPILER_PATH below
TOOLCHAIN=GCC_ARM

# CPU to target; CM4 is the default CPU when this variable is not present.
CORE=CM0P

# Default build configuration. Options include:
#
# Debug -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS

CONFIG=Debug

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# Advanced Configuration
################################################################################

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

# Do not define PSOC6HAL component as HAL is not supported for CM0+
COMPONENTS=CUSTOM_DESIGN_MODUS

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=BSP_DESIGN_MODUS

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
#INCLUDES=
# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=$(MCUBOOT_PATH)/boot/bootutil/include\
          $(MCUBOOT_PATH)/boot/bootutil/src\
          $(MCUBOOT_CY_PATH)/cy_flash_pal/include\
          $(MCUBOOT_CY_PATH)/cy_flash_pal/flash_qspi\
          $(MCUBOOTAPP_PATH)\
          $(MCUBOOTAPP_PATH)/config\
          $(MBEDTLS_PATH)/crypto/include\
          $(MCUBOOT_CY_PATH)/libs/watchdog



# Include the application make files 
include ./app.mk
include ../shared/shared_config.mk

# Add additional defines to the build process (without a leading -D).
# ECC256_KEY_FILE='"$(SIGN_KEY_FILE_ECC).pub"'
# The ECC256_KEY_FILE is the public key used to verify the application bundle (CM0+ and CM4)
DEFINES+=   \
         ECC256_KEY_FILE='"$(SIGN_KEY_FILE_ECC).pub"' \
         MBEDTLS_CONFIG_FILE='"mcuboot_crypto_config.h"' \
         MCUBOOT_IMAGE_NUMBER=$(MCUBOOT_IMAGE_NUMBER) \
         MCUBOOT_MAX_IMG_SECTORS=$(MCUBOOT_MAX_IMG_SECTORS)

ifeq ($(USE_CRYPTO_HW), 1)
CY_MBEDTLS_ACCLERATION_LIB_PATH=$(MCUBOOT_CY_PATH)/libs/cy-mbedtls-acceleration/mbedtls_MXCRYPTO

DEFINES+=CY_CRYPTO_HAL_DISABLE MBEDTLS_USER_CONFIG_FILE='"mcuboot_crypto_acc_config.h"'
SOURCES+=$(wildcard $(CY_MBEDTLS_ACCLERATION_LIB_PATH)/*.c)
INCLUDES+=$(CY_MBEDTLS_ACCLERATION_LIB_PATH)
endif

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=hardfp

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
LDLIBS=

# Custom pre-build commands to run.
# MCUboot uses submodules and "make getlibs" currently does not support
# fetching submodules.
# The PREBUILD command fetches the submodules only if the content of the 
# "mcuboot/ext/mbedtls" directory is empty. 
#
# QSPI memory configuration files (cycfg_qspi_memslot.c/.h) are not
# auto-generated since QSPI is not enabled by default in design.modus. 
# The second pre-build step invokes the QSPI Configurator tool to generate the
# files from design.cyqspi file, under GeneratedSource directory. This
# configuration is used by the programmer to program the external flash. 
# See cy_serial_flash_prog.c file for details. 

PREBUILD=\
if [ -n "$$(ls -A $(MBEDTLS_PATH) 2>/dev/null)" ]; then\
    echo "Git submodules for MCUboot exist. Skipping this step...";\
    else\
    echo "Initializing Git Submodules for MCUboot";\
    cd libs/mcuboot;git submodule update --init --recursive;\
fi;

BINARY_OUT_PATH=$(CY_BUILD_LOC)/$(TARGET)/$(CONFIG)/$(APPNAME)
BUILD_OUT_PATH=$(CY_INTERNAL_APPLOC)/build/$(TARGET)/$(CONFIG)/$(APPNAME)

# Custom post-build commands to run.
# 1. Take a backup of the original hex file (_unsigned.hex)
# 2. Delete the original hex
# 3. Take a backup of the original elf file (_unsigned.elf)
# 4. Delete the original elf
# 5. Signs the .elf file using RSA private key using cymcuelftool
#    to generate the $(APPNAME).hex/elf files.
#
# Note that the final file should be $(APPNAME).hex so that Eclipse and
# Make CLI can locate the file for programming.
POSTBUILD=\
cp -f $(BINARY_OUT_PATH).hex $(BINARY_OUT_PATH)_unsigned.hex;\
rm -f $(BINARY_OUT_PATH).hex;\
cp -f $(BINARY_OUT_PATH).elf $(BINARY_OUT_PATH)_unsigned.elf;\
rm -f $(BINARY_OUT_PATH).elf;\
$(MCUELFTOOL_LOC) --sign $(BUILD_OUT_PATH)_unsigned.elf SHA256 --encrypt RSASSA-PKCS \
--key $(CY_INTERNAL_APPLOC)/keys/$(SIGN_KEY_FILE_RSA).pem --output $(BUILD_OUT_PATH).elf \
--hex $(BUILD_OUT_PATH).hex

# Toolchain specific linker flags  
# The Bootloader Flash and SRAM size is copied to the linker file from the shared_config.mk file
ifeq ($(TOOLCHAIN), GCC_ARM)
LINKER_SCRIPT=$(wildcard ./../shared/linker_script/TARGET_$(TARGET)/TOOLCHAIN_$(TOOLCHAIN)/COMPONENT_CM0P/*_btldr.ld)
else
$(error Only GCC_ARM is supported at this moment)
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
# folder). Make sure you use forward slashes.
CY_TOOLS_PATHS+=

# Default to the newest installed tools folder, or the users override (if it's
# found).
CY_TOOLS_DIR=$(lastword $(sort $(wildcard $(CY_TOOLS_PATHS))))
MCUELFTOOL_LOC= $(CY_MCUELFTOOL_DIR)/bin/cymcuelftool

ifeq ($(CY_TOOLS_DIR),)
$(error Unable to find any of the available CY_TOOLS_PATHS -- $(CY_TOOLS_PATHS). On Windows, use forward slashes.)
endif

$(info Tools Directory: $(CY_TOOLS_DIR))

include $(CY_TOOLS_DIR)/make/start.mk

################################################################################
# Key Generation Scripts
################################################################################

# Python path definition
ifeq ($(OS),Windows_NT)
PYTHON_PATH?=python
else
PYTHON_PATH?=python3
endif

# Output directory
OUT_DIR=$(CY_INTERNAL_APPLOC)/keys

# Generated file names for RSA
PRIV_NAME=rsa_private_generated.txt
PUB_NAME=rsa_public_generated.txt
MOD_NAME=rsa_to_c_generated.txt

# Generated file names for ECC
ECC_PRIV_NAME=$(SIGN_KEY_FILE_ECC).pem
ECC_PUB_NAME=$(SIGN_KEY_FILE_ECC).pub
ECC_PUB_NAME_H=ecc-public-key-p256.h

# Python script path
RSA_TO_C_PYTHON_SCRIPT=$(CY_INTERNAL_APPLOC)/scripts/rsa_to_c.py
IMGTOOL_PATH=$(MCUBOOT_PATH)/scripts/imgtool.py

# RSA key length
# Supported lengths: 1024, 2048
RSA_KEY_LEN=2048

# Generate the RSA-2048 public and private key
rsa_keygen:
	rm -f $(OUT_DIR)/$(PRIV_NAME) $(OUT_DIR)/$(PUB_NAME) $(OUT_DIR)/$(MOD_NAME)
	openssl genrsa -out $(OUT_DIR)/$(PRIV_NAME) $(RSA_KEY_LEN)
	openssl rsa -in $(OUT_DIR)/$(PRIV_NAME) -outform PEM -pubout -out $(OUT_DIR)/$(PUB_NAME)

	# Create C-code ready public key
	$(PYTHON_PATH) $(RSA_TO_C_PYTHON_SCRIPT) $(OUT_DIR)/$(PUB_NAME) > $(OUT_DIR)/$(MOD_NAME)
	
# Generate the ECC-P256 public and private key
ecc_keygen:
	rm -f $(OUT_DIR)/$(ECC_PRIV_NAME) $(OUT_DIR)/$(ECC_PUB_NAME) $(OUT_DIR)/$(ECC_PUB_NAME_H)
	$(PYTHON_PATH) $(IMGTOOL_PATH) keygen -k $(OUT_DIR)/$(ECC_PRIV_NAME) -t ecdsa-p256
	$(PYTHON_PATH) $(IMGTOOL_PATH) getpub -k $(OUT_DIR)/$(ECC_PRIV_NAME) >> $(OUT_DIR)/$(ECC_PUB_NAME)
	$(PYTHON_PATH) $(IMGTOOL_PATH) getpub -k $(OUT_DIR)/$(ECC_PRIV_NAME) >> $(OUT_DIR)/$(ECC_PUB_NAME_H)
	
