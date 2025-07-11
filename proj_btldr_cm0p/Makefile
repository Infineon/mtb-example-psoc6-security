################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# CM0+ bootloader project make file.
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

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=proj_btldr_cm0p

# CPU to target; CM4 is the default CPU when this variable is not present.
CORE=CM0P
CORE_NAME=CM0P_0

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
COMPONENTS=

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES+=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
#INCLUDES=
# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=

# Include the application make files 
include ./app.mk

# Add additional defines to the build process (without a leading -D).
# ECC256_KEY_FILE='"$(SIGN_KEY_FILE_ECC).pub"'
# The ECC256_KEY_FILE is the public key used to verify the application bundle (CM0+ and CM4)
DEFINES+=MBEDTLS_CONFIG_FILE='"mcuboot_crypto_config.h"' \
         ECC256_KEY_FILE='"$(SIGN_KEY_FILE_ECC).pub"' \
         MBEDTLS_CONFIG_FILE='"mcuboot_crypto_config.h"' \
         MCUBOOT_IMAGE_NUMBER=$(MCUBOOT_IMAGE_NUMBER) \
         MCUBOOT_MAX_IMG_SECTORS=$(MCUBOOT_MAX_IMG_SECTORS)

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

# Toolchain specific linker flags  
# The Bootloader Flash and SRAM size is copied to the linker file from the shared_config.mk file
ifeq ($(TOOLCHAIN), GCC_ARM)
LINKER_SCRIPT=$(wildcard ../shared/linker_script/TARGET_$(RENAMED_TARGET)/TOOLCHAIN_$(TOOLCHAIN)/COMPONENT_CM0P/*_btldr.ld)
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

# Absolute path to the compiler's "bin" directory. The variable name depends on the
# toolchain used for the build. Refer to the ModusToolbox user guide to get the correct
# variable name for the toolchain used in your build.
#
# The default path depends on the selected TOOLCHAIN and is set in the Make recipe.
CY_COMPILER_GCC_ARM_DIR?=

$(info Tools Directory: $(CY_TOOLS_DIR))

CY_BUILD_LOCATION=./build
BINARY_OUT_PATH=$(CY_BUILD_LOCATION)/$(TARGET)/$(CONFIG)/$(APPNAME)
CY_MCUELFTOOL_DIR=$(wildcard $(CY_TOOLS_DIR)/cymcuelftool-*)
MCUELFTOOL_LOC=$(CY_MCUELFTOOL_DIR)/bin/cymcuelftool
SREC_CAT_LOC=$(CY_TOOLS_DIR)/srecord/bin/srec_cat

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
POSTBUILD+=\
cp -f $(BINARY_OUT_PATH).hex $(BINARY_OUT_PATH)_unsigned.hex;\
rm -f $(BINARY_OUT_PATH).hex;\
cp -f $(BINARY_OUT_PATH).elf $(BINARY_OUT_PATH)_unsigned.elf;\
rm -f $(BINARY_OUT_PATH).elf;\
$(MCUELFTOOL_LOC) --sign $(BINARY_OUT_PATH)_unsigned.elf SHA256 --encrypt RSASSA-PKCS \
--key ./keys/$(SIGN_KEY_FILE_RSA).pem --output $(BINARY_OUT_PATH).elf \
--hex $(BINARY_OUT_PATH).hex;

# Cymcuelftool populates the .cymeta and .cychecksum sections
# which are not physical addresses on the device. They are
# used by certain tools like PSoC Programmer to determine
# the device and verify checksum. Since we don't use these
# tools, these sections are removed. Keeping them will result
# in errors during programming because these addresses don't
# exist on the device.
# 
# srec_cat tool is used to exclude addresses we don't care
# about.
METADATA_START_ADDR=0x90300000
METADATA_END_ADDR=0x90700000

POSTBUILD+=$(SREC_CAT_LOC) $(BINARY_OUT_PATH).hex -intel -exclude $(METADATA_START_ADDR) \
$(METADATA_END_ADDR) -o $(BINARY_OUT_PATH)_stripped.hex -intel --Output_Block_Size 16 ; \
cp -f $(BINARY_OUT_PATH)_stripped.hex $(BINARY_OUT_PATH).hex; \
rm -f $(BINARY_OUT_PATH)_stripped.hex;

################################################################################
# Key Generation Scripts
################################################################################

# Python path definition
CY_PYTHON_REQUIREMENT=true

# Output directory
OUT_DIR=./keys

# Generated file names for RSA
RSA_PRIV_NAME=rsa_private_generated.txt
RSA_PUB_NAME=rsa_public_generated.txt
MOD_NAME=rsa_to_c_generated.txt

# Generated file names for ECC
ECC_PRIV_NAME=$(SIGN_KEY_FILE_ECC).pem
ECC_PUB_NAME=$(SIGN_KEY_FILE_ECC).pub
ECC_PUB_NAME_H=ecc-public-key-p256.h

# Python script path
RSA_TO_C_PYTHON_SCRIPT=./scripts/rsa_to_c.py
IMGTOOL_PATH=$(MCUBOOT_PATH)/scripts/imgtool.py

# RSA key length
# Supported lengths: 1024, 2048
RSA_KEY_LEN=2048

# Generate the RSA-2048 public and private key
rsa_keygen:
	rm -f $(OUT_DIR)/$(RSA_PRIV_NAME) $(OUT_DIR)/$(RSA_PUB_NAME) $(OUT_DIR)/$(MOD_NAME) $(OUT_DIR)/$(SIGN_KEY_FILE_RSA)
	openssl genrsa -out $(OUT_DIR)/$(RSA_PRIV_NAME) $(RSA_KEY_LEN)
	openssl rsa -in $(OUT_DIR)/$(RSA_PRIV_NAME) -outform PEM -pubout -out $(OUT_DIR)/$(RSA_PUB_NAME)

	# Create C-code ready public key
	$(CY_PYTHON_PATH) $(RSA_TO_C_PYTHON_SCRIPT) $(OUT_DIR)/$(RSA_PUB_NAME) > $(OUT_DIR)/$(MOD_NAME)
	
	# Copy the contents of the generated private key into the .pem file
	cp $(OUT_DIR)/$(RSA_PRIV_NAME) $(OUT_DIR)/$(SIGN_KEY_FILE_RSA).pem
	
# Generate the ECC-P256 public and private key
ecc_keygen:
	rm -f $(OUT_DIR)/$(ECC_PRIV_NAME) $(OUT_DIR)/$(ECC_PUB_NAME) $(OUT_DIR)/$(ECC_PUB_NAME_H)
	$(CY_PYTHON_PATH) $(IMGTOOL_PATH) keygen -k $(OUT_DIR)/$(ECC_PRIV_NAME) -t ecdsa-p256
	$(CY_PYTHON_PATH) $(IMGTOOL_PATH) getpub -k $(OUT_DIR)/$(ECC_PRIV_NAME) >> $(OUT_DIR)/$(ECC_PUB_NAME)
	$(CY_PYTHON_PATH) $(IMGTOOL_PATH) getpub -k $(OUT_DIR)/$(ECC_PRIV_NAME) >> $(OUT_DIR)/$(ECC_PUB_NAME_H)

include $(CY_TOOLS_DIR)/make/start.mk