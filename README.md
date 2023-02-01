# PSoC&trade; 6 MCU: Security application template

This is a minimal starter dual-CPU security application template for PSoC&trade; 62/63 MCU devices. This code example is meant to be a companion to the [AN221111 – PSoC&trade; 6 MCU: Designing a secured system](https://www.infineon.com/an221111) application note. It demonstrates the following features to create a secured system as explained in the application note.

- Bootloader based on industry-standard MCUboot (CM0+)
- Cryptographically signed bootloader
- Dual-CPU operation; user applications for both CM0+ and CM4
- Supports Device Firmware Update (DFU) with standard UART interface
- FreeRTOS running on CM4
- Signed CM0+ and CM4 application bundle
- Full Chain of Trust (CoT)
- Isolated CPUs using Shared Memory Protection Unit (SMPU)

The application template bundles three projects:

- **proj_btldr_cm0p** – Implements an MCUboot-based basic bootloader project run by CM0+. The bootloader handles image authentication and upgrades. When the image is valid, the bootloader lets the user project bundle (CM0+/CM4 user image) and run the image by passing the starting address of the image to it. It also programs the protection units and other system security related features.

- **proj_cm0p** – Implements a blinky LED project run by CM0+ that toggles the user LED at different rates depending on whether it was built in BOOT mode or UPGRADE mode.

- **proj_cm4** – Implements a DFU task to receive firmware updates over UART using FreeRTOS. It also demonstrates the usage of IPCs to request data from the CM0+ project. This application is merged with the *proj_cm0p* image to generate the dual-CPU image to be programmed.

   You can build the CM0+ and CM4 user project in one of the following ways:

   - **BOOT mode:** The user image is built to be programmed into the primary slot. The bootloader will simply boot the user image on the next reset.

   - **UPGRADE mode:** The user image is built to be programmed into the secondary slot. Based on the user input, the bootloader will copy the image into the primary slot and boot it on the next reset.

The application template also has files and directories that are of importance such as the following:

- **shared** directory that contains the linker scripts for different BSPs
- *common.mk* is a configuration Makefile that defines the target, toolchain, optimization etc., and defines macros that can be used in the linker scripts.

[View this README on GitHub.](https://github.com/Infineon/mtb-example-psoc6-security)

[Provide feedback on this Code Example.](https://cypress.co1.qualtrics.com/jfe/form/SV_1NTns53sK2yiljn?Q_EED=eyJVbmlxdWUgRG9jIElkIjoiQ0UyMzQ5OTIiLCJTcGVjIE51bWJlciI6IjAwMi0zNDk5MiIsIkRvYyBUaXRsZSI6IlBTb0MmdHJhZGU7IDYgTUNVOiBTZWN1cml0eSBhcHBsaWNhdGlvbiB0ZW1wbGF0ZSIsInJpZCI6ImRka2EiLCJEb2MgdmVyc2lvbiI6IjIuMC4wIiwiRG9jIExhbmd1YWdlIjoiRW5nbGlzaCIsIkRvYyBEaXZpc2lvbiI6Ik1DRCIsIkRvYyBCVSI6IklDVyIsIkRvYyBGYW1pbHkiOiJQU09DIn0=)

<br>

<details><summary><span style="font-size:1.6em;">Table of contents</span></summary>

- [PSoC™ 6 MCU: Security application template](#psoc-6-mcu-security-application-template)
  - [Requirements](#requirements)
  - [Supported toolchains (make variable 'TOOLCHAIN')](#supported-toolchains-make-variable-toolchain)
  - [Supported kits (make variable 'TARGET')](#supported-kits-make-variable-target)
  - [Hardware setup](#hardware-setup)
  - [Software setup](#software-setup)
  - [Using the code example](#using-the-code-example)
  - [Operation](#operation)
    - [Step-by-step instructions](#step-by-step-instructions)
  - [Debugging](#debugging)
  - [Design and implementation](#design-and-implementation)
  - [Project structure](#project-structure)
  - [Project configuration](#project-configuration)
    - [Flash memory layout](#flash-memory-layout)
    - [SRAM memory layout](#sram-memory-layout)
    - [Configuring the common make variables](#configuring-the-common-make-variables)
      - [Build control variables](#build-control-variables)
      - [Memory layout variables](#memory-layout-variables)
  - [Bootloader implementation](#bootloader-implementation)
    - [MCUboot basics](#mcuboot-basics)
    - [Swap-based upgrade for PSoC™ 6 MCU](#swap-based-upgrade-for-psoc-6-mcu)
    - [Flash map/partition](#flash-mappartition)
      - [Customizing the flash map](#customizing-the-flash-map)
    - [Configuring bootloader make variables](#configuring-bootloader-make-variables)
  - [CM0+ blinky user project implementation](#cm0-blinky-user-project-implementation)
    - [Protected storage](#protected-storage)
    - [Configuring CM0+ project make variables](#configuring-cm0-project-make-variables)
  - [CM4 FreeRTOS user project implementation](#cm4-freertos-user-project-implementation)
    - [Protected storage](#protected-storage-1)
    - [Device Firmware Update (DFU)](#device-firmware-update-dfu)
    - [Configuring CM4 project make variables](#configuring-cm4-project-make-variables)
  - [Security overview](#security-overview)
    - [Secured boot](#secured-boot)
    - [Protection units](#protection-units)
    - [eFuse programming for debug access restrictions and lifecycle](#efuse-programming-for-debug-access-restrictions-and-lifecycle)
    - [MCUboot security](#mcuboot-security)
    - [Generating a key-pair](#generating-a-key-pair)
      - [Generating an RSA key-pair](#generating-an-rsa-key-pair)
      - [Generating the ECC key-pair](#generating-the-ecc-key-pair)
  - [Pre- and post-build steps](#pre--and-post-build-steps)
    - [Bootloader project: Pre-build steps](#bootloader-project-pre-build-steps)
    - [Bootloader project: Post-build steps](#bootloader-project-post-build-steps)
    - [CM0+/CM4 dual-CPU user projects: Post-build steps (for development)](#cm0cm4-dual-cpu-user-projects-post-build-steps-for-development)
    - [CM0+/CM4 dual-CPU user projects: Post-build steps (for production)](#cm0cm4-dual-cpu-user-projects-post-build-steps-for-production)
  - [Bootloader project: Custom device configuration](#bootloader-project-custom-device-configuration)
  - [CM4 user project: Custom device configuration](#cm4-user-project-custom-device-configuration)
  - [Related resources](#related-resources)
  - [Other resources](#other-resources)
  - [Document history](#document-history)

</details>

<br>

## Requirements

- [ModusToolbox&trade; software](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software/) v3.0 or later (tested with v3.0)
- PSoC&trade; 6 Board support package (BSP) minimum required version: 4.0.0
- Programming language: C
- [CYPRESS&trade; programmer](https://www.infineon.com/cms/en/design-support/tools/programming-testing/psoc-programming-solutions/)
- Associated parts: All [PSoC&trade; 6 Dual-CPU MCU](https://www.infineon.com/cms/en/product/microcontroller/32-bit-psoc-arm-cortex-microcontroller/psoc-6-32-bit-arm-cortex-m4-mcu/) parts (except PSoC&trade; 64)

## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm&reg; Embedded Compiler v10.3.1 (`GCC_ARM`) - Default value of `TOOLCHAIN`

## Supported kits (make variable 'TARGET')

- [PSoC&trade; 6 Wi-Fi Bluetooth&reg; pioneer kit](https://www.infineon.com/CY8CKIT-062-WIFI-BT) (`CY8CKIT-062-WIFI-BT`) - Default value of `TARGET`
- [PSoC&trade; 6 Wi-Fi Bluetooth&reg; prototyping kit](https://www.infineon.com/CY8CPROTO-062-4343W) (`CY8CPROTO-062-4343W`)
- [PSoC&trade; 6 Bluetooth&reg; LE pioneer kit](https://www.infineon.com/CY8CKIT-062-BLE) (`CY8CKIT-062-BLE`)
- [PSoC&trade; 6 Bluetooth&reg; LE prototyping kit](https://www.infineon.com/CY8CPROTO-063-BLE) (`CY8CPROTO-063-BLE`)
- [PSoC&trade; 62S2 Wi-Fi Bluetooth&reg; pioneer kit](https://www.infineon.com/CY8CKIT-062S2-43012) (`CY8CKIT-062S2-43012`)
- [PSoC&trade; 62S3 Wi-Fi Bluetooth&reg; prototyping kit](https://www.infineon.com/CY8CPROTO-062S3-4343W) (`CY8CPROTO-062S3-4343W`)

## Hardware setup

This example uses the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

**Note:** ModusToolbox&trade; software requires KitProg3. Before using this code example, make sure that the board is upgraded to KitProg3. The tool and instructions are available in the [Firmware Loader](https://github.com/Infineon/Firmware-loader) GitHub repository.

If you do not upgrade, you will see an error like "unable to find CMSIS-DAP device" or "KitProg firmware is out of date".

## Software setup

Install a terminal emulator if you don't have one. Instructions in this document use [Tera Term](https://ttssh2.osdn.jp/index.html.en).

This example also requires some Python packages to be installed. Run the following command in your terminal from the application root folder to install all the pre-requisite packages:
```
pip install -r ./proj_btldr_cm0p/scripts/requirements.txt
```

**Note:** For MacOS users, latest version of openssl must be installed from homebrew/ports, and "/usr/local/bin" must be in `PATH` before "/usr/bin".
```
brew install openssl
```


## Using the code example

Create the project and open it using one of the following:

<details><summary><b>In Eclipse IDE for ModusToolbox&trade; software</b></summary>

1. Click the **New Application** link in the **Quick Panel** (or, use **File** > **New** > **ModusToolbox&trade; Application**). This launches the [Project Creator](https://www.infineon.com/ModusToolboxProjectCreator) tool. Project creation will take a few minutes to complete.

2. Pick a kit supported by the code example from the list shown in the **Project Creator - Choose Board Support Package (BSP)** dialog.

   When you select a supported kit, the example is reconfigured automatically to work with the kit. To work with a different supported kit later, use the [Library Manager](https://www.infineon.com/ModusToolboxLibraryManager) to choose the BSP for the supported kit. You can use the Library Manager to select or update the BSP and firmware libraries used in this application. To access the Library Manager, click the link from the **Quick Panel**.

   You can also just start the application creation process again and select a different kit.

   If you want to use the application for a kit not listed here, you may need to update the source files. If the kit does not have the required resources, the application may not work.

3. In the **Project Creator - Select Application** dialog, choose the example by enabling the checkbox.

4. (Optional) Change the suggested **New Application Name**.

5. The **Application(s) Root Path** defaults to the Eclipse workspace which is usually the desired location for the application. If you want to store the application in a different location, you can change the *Application(s) Root Path* value. Applications that share libraries should be in the same root path.

6. Click **Create** to complete the application creation process.

For more details, see the [Eclipse IDE for ModusToolbox&trade; software user guide](https://www.infineon.com/MTBEclipseIDEUserGuide) (locally available at *{ModusToolbox&trade; software install directory}/ide_{version}/docs/mt_ide_user_guide.pdf*).

</details>

<details><summary><b>In command-line interface (CLI)</b></summary>

ModusToolbox&trade; software provides the Project Creator as both a GUI tool and the command line tool, "project-creator-cli". The CLI tool can be used to create applications from a CLI terminal or from within batch files or shell scripts. This tool is available in the *{ModusToolbox&trade; software install directory}/tools_{version}/project-creator/* directory.

Use a CLI terminal to invoke the "project-creator-cli" tool. On Windows, use the command line "modus-shell" program provided in the ModusToolbox&trade; software installation instead of a standard Windows command-line application. This shell provides access to all ModusToolbox&trade; software tools. You can access it by typing `modus-shell` in the search box in the Windows menu. In Linux and macOS, you can use any terminal application.

The "project-creator-cli" tool has the following arguments:

Argument | Description | Required/optional
---------|-------------|-----------
`--board-id` | Defined in the `<id>` field of the [BSP](https://github.com/Infineon?q=bsp-manifest&type=&language=&sort=) manifest | Required
`--app-id`   | Defined in the `<id>` field of the [CE](https://github.com/Infineon?q=ce-manifest&type=&language=&sort=) manifest | Required
`--target-dir`| Specify the directory in which the application is to be created if you prefer not to use the default current working directory | Optional
`--user-app-name`| Specify the name of the application if you prefer to have a name other than the example's default name | Optional

<br />

The following example clones the "[Security application template](https://github.com/Infineon/mtb-example-psoc6-security)" application with the desired name "MySecurityApp" configured for the *CY8CKIT-062-WIFI-BT* BSP into the specified working directory, *C:/mtb_projects*:

   ```
   project-creator-cli --board-id CY8CKIT-062-WIFI-BT --app-id mtb-example-psoc6-security --user-app-name MySecurityApp --target-dir "C:/mtb_projects"
   ```

**Note:** The project-creator-cli tool uses the `git clone` and `make getlibs` commands to fetch the repository and import the required libraries. For details, see the "Project creator tools" section of the [ModusToolbox&trade; software user guide](https://www.infineon.com/ModusToolboxUserGuide) (locally available at *{ModusToolbox&trade; software install directory}/docs_{version}/mtb_user_guide.pdf*).

To work with a different supported kit later, use the [Library Manager](https://www.infineon.com/ModusToolboxLibraryManager) to choose the BSP for the supported kit. You can invoke the Library Manager GUI tool from the terminal using `make modlibs` command or use the Library Manager CLI tool "library-manager-cli" to change the BSP.

The "library-manager-cli" tool has the following arguments:

Argument | Description | Required/optional
---------|-------------|-----------
`--add-bsp-name` | Name of the BSP that should be added to the application | Required
`--set-active-bsp` | Name of the BSP that should be as active BSP for the application | Required
`--add-bsp-version`| Version of the BSP that should be added to the application if you do not wish to use the latest from the manifest | Optional
`--add-bsp-location`| Location of the BSP (local/shared) if you prefer to add the BSP in a shared path | Optional

<br />

The following example adds the CY8CPROTO-062-4343W BSP to the already created application and makes it the active BSP for the app:

   ```
   library-manager-cli --project "C:/mtb_projects/MySecurityApp" --add-bsp-name CY8CPROTO-062-4343W --add-bsp-version "latest-v4.X" --add-bsp-location "local"

   library-manager-cli --project "C:/mtb_projects/MySecurityApp" --set-active-bsp APP_CY8CPROTO-062-4343W
   ```

</details>

<details><summary><b>In third-party IDEs</b></summary>

Use one of the following options:

- **Use the standalone [Project Creator](https://www.infineon.com/ModusToolboxProjectCreator) tool:**

   1. Launch Project Creator from the Windows Start menu or from *{ModusToolbox&trade; software install directory}/tools_{version}/project-creator/project-creator.exe*.

   2. In the initial **Choose Board Support Package** screen, select the BSP, and click **Next**.

   3. In the **Select Application** screen, select the appropriate IDE from the **Target IDE** drop-down menu.

   4. Click **Create** and follow the instructions printed in the bottom pane to import or open the exported project in the respective IDE.

<br />

- **Use command-line interface (CLI):**

   1. Follow the instructions from the **In command-line interface (CLI)** section to create the application, and then import the libraries using the `make getlibs` command.

   2. Export the application to a supported IDE using the `make <ide>` command.

   3. Follow the instructions displayed in the terminal to create or import the application as an IDE project.

For a list of supported IDEs and more details, see the "Exporting to IDEs" section of the [ModusToolbox&trade; software user guide](https://www.infineon.com/ModusToolboxUserGuide) (locally available at *{ModusToolbox&trade; software install directory}/docs_{version}/mtb_user_guide.pdf*).

</details>


## Operation

This application bundles three projects: the bootloader project run by the CM0+ CPU (proj_btldr_cm0p), the blinky project run by the CM0+ CPU (proj_cm0p), and the FreeRTOS project run by the CM4 CPU (proj_cm4).

The example can be built in two different ways:

- *For development* (default) – Used for developing the application and testing it

- *For production* – Used when firmware testing is complete and the device is ready to be deployed in the field

The steps remain similar for both methods except that during a production build, an additional post-build step is executed to merge the HEX files of all the applications to create a single binary. See [CM0+/CM4 user app post-build steps for production](#cm0cm4-dual-cpu-user-app-post-build-steps-for-production).

You need to build and program the projects in the following order. Do not start building the application yet: follow the [Step-by-step instructions](#step-by-step-instructions):

1. **Build and program the bootloader project** – On next reset, CM0+ runs the bootloader and prints a message that no valid image has been found.

2. **Build the blinky project in BOOT (default) mode** – A CM0+ binary will be generated on successful build.

3. **Build and program the FreeRTOS project in BOOT (default) mode** – The CM4 binary gets generated that integrates the CM0+ binary on a successful build. Once programmed, the bootloader will find that the user application exists and then transfer control to it if the validation is successful. The blinky and FreeRTOS projects will be running on the CM0+ and CM4 CPUs respectively with debug messages printed on the serial terminal.

4. **Build the blinky project in UPGRADE mode** – A CM0+ binary will be generated on a successful build.

5. **Build and program the FreeRTOS project in UPGRADE mode** – The CM4 binary is generated that integrates the CM0+ binary on a successful build. The DFU UART connections must be done as explained in [Table 7: DFU status LED and UART connections](#cm4-freertos-user-app-implementation). The [DFU Host tool](https://www.infineon.com/dgdl/Infineon-ModusToolbox_Device_Firmware_Update_Host_Tool_(Version_1.1)-Software-v01_00-EN.pdf?fileId=8ac78c8c7e7124d1017ed92bdbb22be3) is used to transfer the image over UART. Once the transfer is complete, the bootloader will find that the upgrade image exists and then either overwrite the primary slot or swap the primary slot with the new image based on the value of `SWAP_UPGRADE`. The blinky and FreeRTOS projects will be running on the CM0+ and CM4 CPUs respectively with debug messages printed on the serial terminal.


### Step-by-step instructions

<details><summary><b>For development</b></summary>

The *proj_btldr_cm0p* project design is based on MCUboot, which uses the [imgtool](https://pypi.org/project/imgtool/) Python module for image signing and key management.

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector. Open a terminal program and select the KitProg3 COM port. Set the serial port parameters to 8N1 and 115200 baud.

2. Build and program the bootloader project. If the build fails, check if you have installed the pre-requisite python libraries described in [Software Setup](#software-setup). When programming consecutively, make sure to erase the device first before re-programming.

   <details><summary><b>Using Eclipse IDE for ModusToolbox&trade; software</b></summary>

      1. Select the *proj_btldr_cm0p* project in the Project Explorer.
   
      2. Follow the instructions in this [KBA236748](https://community.infineon.com/t5/Knowledge-Base-Articles/Working-with-multi-project-applications-in-ModusToolbox-3-0-KBA236748/ta-p/393788) to create the **\<Project Name> Program (KitProg3_MiniProg4)** OpenOCD configuration correctly.

      3. In the **Quick Panel**, scroll down, and click **\<Project Name> Program (KitProg3_MiniProg4)** thus generated.

   </details>

   <details><summary><b>Using CLI</b></summary>

     From the terminal, go to the *proj_btldr_cm0p* directory and execute the `make program` command to build and program the project using the default toolchain to the default target.

     You can specify a target and toolchain manually using the following command:
      ```
      make program TARGET=<BSP> TOOLCHAIN=<toolchain>
      ```

      Example:
      ```
      make program TARGET=CY8CPROTO-062-4343W TOOLCHAIN=GCC_ARM
      ```

   </details>

   <details><summary><b>Using CYPRESS&trade; Programmer (for programming only)</b></summary>

     Use the following settings when programming the kit using CYPRESS&trade; Programmer.
     1. Select **Reset chip**.
     2. Deselect **Program Security Data**. (**Note:** When programming eFuse, this should be selected).
     3. Set **Voltage** to 3.3 V. (**Note:** When programming eFuse, set this to 2.5 V.)
     4. Set **Reset Type** as Soft.
     5. Set **SFlash Restrictions** to "Erase/Program USER/TOC/KEY allowed".
     6. Specify the HEX file to be programmed and then click **Connect** and **Program**.

     **Figure 1. CYPRESS&trade; Programmer settings**

      ![CYPRESS Programmer settings](images/cypress-programmer.png)

   </details>

   </br>

   After programming, the *bootloader* starts automatically. Confirm that the UART terminal displays the current lifecycle state, access restrictions, status of protection units configuration, and status of the bootloader operation as highlighted in the figure.

   Note that the primary slot does not contain any valid image at this stage.

   **Figure 2. Bootloader starting with no bootable image**

   ![](images/booting-with-no-bootable-image.png)

   **Note:** If you do not see any message printed on the UART terminal (assuming that you have the right settings as mentioned in Step 1), it is most likely because of any of the following reasons:

   - **SFlash wasn't updated** – Perform an **Erase** using CYPRESS&trade; Programmer to erase the SFlash contents (USER/TOC/KEY). Now **Program** the HEX file to see if it works as expected.
   - **The validation of the bootloader has failed** – The bootloader image is signed with the RSA private key as part of the post-build steps. Ensure that the correct public key information that pairs with the private key is being used in the *cy_ps_keystorage.c* file. See [Generating a key-pair](#generating-a-key-pair) for more information.

   </br>

3. Build the *proj_cm4* project (use one of the options as shown in Step 2 above). All the projects are built during a build. This builds *proj_cm0p* along with *proj_cm4* every time to create the dual-CPU user image (*primary_app.hex*).
   
4. (Skip to step 5 if not using IDE) Create the OpenOCD configuration for programming the hex file by following the steps in this [KBA236748](https://community.infineon.com/t5/Knowledge-Base-Articles/Working-with-multi-project-applications-in-ModusToolbox-3-0-KBA236748/ta-p/393788). Change the executable from *app_combined.hex* to *primary_app.hex* in the configuration options.

5. Program the *proj_cm4* project (use one of the options as shown in Step 2 above). 

      After programming, the *proj_btldr_cm0p* project starts automatically. At this stage, valid user image exists in the primary slot and control is transferred to it if the validation is successful. The UART terminal will display the following message based on the validation status:

   - If validation is successful:

      **Figure 3. Bootloader starting with a valid user image**

      ![Bootloader starting with a valid user image](images/booting-with-valid-user-image.png)

   - If validation fails:

      **Figure 4. Bootloader starting with an invalid user image**

      ![Bootloader starting with an invalid user image](images/booting-with-invalid-user-image.png)

   **Note that the secondary slot does not contain any valid image at this stage.**

6. If the user image is running successfully, the LED toggles on the kit to indicate a successful CM0+ startup. The UART terminal displays the successful startup of the CM4 project.

   **Note:** On kits with two LEDs, you should see the second LED glow indicating a successful CM4 startup. See [Table 7: DFU status LED and UART connections](#cm4-freertos-user-app-implementation) for LED pin assignment.

7. Edit the *common.mk* file in the application root directory to create the UPGRADE image. Change the `IMG_TYPE` variable to UPGRADE.
   ```
   IMG_TYPE ?= UPGRADE
   ```

8. Build the *proj_cm4* project. The projects have been developed to incorporate minor changes based on whether `IMG_TYPE` is set to `BOOT` or `UPGRADE`. This combines *proj_cm0p* along with *proj_cm4* to create the dual-CPU user image. As part of the post-build steps, it merges the *proj_cm0p* HEX file with the *proj_cm4* HEX and signs the image using *imgtool* to create the user application bundle (*primary_app_UPGRADE.hex*) and then converts it to a *.cyacd2* file required by the DFU Host tool.

9.  Connect an external MiniProg3/4 or KitProg3 programmer/debugger and make the connections required for the DFU UART as explained in [Table 7: DFU status LED and UART connections](#cm4-freertos-user-app-implementation).

10. Open the DFU Host tool (*dfuh-tool*) from *ModusToolbox install directory}/tools_\<version>/dfuh-tool*. Verify that you see the debugger probe. Choose the one that has UART on it as follows:

   ![DFU Host tool UART](./images/dfuh-operation.png)

11. Select the *.cyacd2* file from the build output and click **Program**. The operation should take a few minutes to complete.

12. Once complete, the image validation status is displayed. If validation is successful, a software reset is initiated to boot it into the new image as follows. Else, the upgrade is aborted.

    ![DFU upgrade](./images/dfu-firmware-update.png)

    Observe that the blinky LED is now blinking faster (every 250 ms) and the debug logs indicate the upgrade image being run.

13. Once the working of the bootloader and user projects has been tested, you can blow the eFuses to transition from NORMAL to SECURE lifecycle mode for further testing. See [eFuse programming for debug access restrictions and lifecycle](#efuse-programming-for-debug-access-restrictions-and-lifecycle).

14. Once the device is working as expected in SECURE lifecycle mode, proceed to [Step-by-step instructions for production build](#step-by-step-instructions) to program multiple devices at production level.

</details>

<details><summary><b>For production</b></summary>

The *proj_btldr_cm0p* project design is based on MCUboot, which uses the [imgtool](https://pypi.org/project/imgtool/) Python module for image signing and key management.

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector. Open a terminal program and select the KitProg3 COM port. Set the serial port parameters to 8N1 and 115200 baud.

2. This step assumes that you have tested the working of your bootloader and your user projects in SECURE lifecycle stage during development. Edit the *proj_btldr_cm0p/source/cy_ps_efuse.h* file and set the `CY_EFUSE_AVAILABLE` macro to '1'.
   ```
   #define CY_EFUSE_AVAILABLE 1
   ```

3. Verify the eFuse configuration defined in the `cy_efuse_data` variable.

4. Build the bootloader project. This generates the *proj_btldr_cm0p.hex* file in the output build directory.

   <details><summary><b>Using Eclipse IDE for ModusToolbox&trade; software</b></summary>

      1. Select the *proj_btldr_cm0p* project in the Project Explorer.

      2. In the **Quick Panel**, scroll down, and click **Build \<Application Name>**.

   </details>

   <details><summary><b>Using CLI</b></summary>

     From the terminal, go to the *proj_btldr_cm0p* directory and execute the `make build` command to build the project using the default toolchain to the default target.

     You can specify a target and toolchain manually using the following command:
      ```
      make build TARGET=<BSP> TOOLCHAIN=<toolchain>
      ```

      Example:
      ```
      make build TARGET=CY8CPROTO-062-4343W TOOLCHAIN=GCC_ARM
      ```

   </details>

</br>

1. Navigate to the *proj_cm4* directory and edit the Makefile to enable production build. Change the `PRODUCTION` Makefile variable to '1'.
   ```
   PRODUCTION=1
   ```

2. Build the *proj_cm4* project. All the projects are built during a build. This builds *proj_cm0p* along with *proj_cm4* every time to create the dual-CPU user image (*primary_app.hex*). Because production build is enabled, a merged HEX file (*production.hex*) of all the three applications is also generated as part of the post-build steps. See [CM0+/CM4 dual-CPU user app post-build steps for production](#cm0cm4-dual-cpu-user-app-post-build-steps-for-production).

3. Program the merged HEX file using CYPRESS&trade; Programmer. When programming consecutively, make sure to erase the device first before re-programming:

   1. Select the correct kit in the **Probe/Kit** option. Specify the *production.hex* file to be programmed and use the following settings:

      1. Select **Reset chip**.
      2. Select **Program Security Data**. (**Note:** When not programming eFuse, leave this deselected.)
      3. Set **Voltage** to 2.5 V. (**Note:** When not programming eFuse, set this to 3.3 V.)
      4. Set **Reset Type** to Soft.
      5. Set **Sflash Restrictions** to "Erase/Program USER/TOC/KEY allowed".

      <br>

      **Note:** CYPRESS&trade; Programmer will indicate the current operating voltage at the bottom right as highlighted in the figure. If you do not see 2.5 V even after setting the **Voltage** option to 2.5 V, check if the kit has a VDD select switch / jumper and change it to 2.5 V or 1.8 V - 3.3 V variable voltage.

      <br>

      **Figure 5. CYPRESS&trade; Programmer settings**

      ![Programmer settings](images/cypress-programmer-production.png)

   2. Click **Connect** and then click **Program**.

4. After programming, the *proj_btldr_cm0p* project starts automatically. At this stage, valid user image exists in the primary slot and control is transferred to it if the validation is successful. The UART terminal displays the following message based on the validation status:

   - If validation is successful:

      **Figure 6. Bootloader starting with valid user image**

      ![Bootloader starting with valid user image](images/booting-with-valid-user-image.png)

   - If validation fails:

      **Figure 7. Bootloader starting with invalid user image**

      ![Bootloader starting with invalid user image](images/booting-with-invalid-user-image.png)

   Note that the secondary slot does not contain any valid image at this stage. This represents the state of the device in the field.

5. If the user projects are running successfully, the LED should be toggling on the kit to indicate successful CM0+ startup. The UART terminal will display the successful startup of the CM4 project.

   **Note:** On kits with two LEDs, you should see the second LED glow indicating a successful CM4 startup. See [Table 7: DFU status LED and UART connections](#cm4-freertos-user-app-implementation) for LED pin assignment.

6.  Build the *proj_cm4* project for `UPGRADE` by changing `IMG_TYPE` to `UPGRADE` in *common.mk*. This represents the new firmware to be sent to the device in the field. The projects have been developed to incorporate minor changes based on whether `IMG_TYPE` is set to `BOOT` or `UPGRADE`. As part of the post-build steps, it merges the CM0p and CM4 user projects and signs the image using *imgtool* to create the user application bundle (*primary_app_UPGRADE.hex*) and then converts it to a *.cyacd2* file required by the DFU Host tool.

7.  Connect an external MiniProg3/4 or KitProg3 and make the connections needed for the DFU UART as explained in [Table 7: DFU status LED and UART connections](#cm4-freertos-user-app-implementation).

8.  Open the DFU Host tool (*dfuh-tool*) and verify that you are seeing the debugger probe. Choose the one that has UART on it as follows:

   **Figure 8. Message from DFU host tool**

   ![DFU Host tool UART](./images/dfuh-operation.png)

9.  Select the *.cyacd2* file from the build output and click **Program**. The operation should take a few minutes to complete.

10. Once complete, the image validation status will be displayed. If validation is successful, a software reset is initiated to boot it into the new image as highlighted below. Else, the upgrade is aborted.

      **Figure 9. DFU upgrade**

      ![DFU upgrade](./images/dfu-firmware-update.png)

      Observe that the blinky LED is now blinking faster (every 250 ms) and the debug logs indicate the upgrade image being run.

</details>

## Debugging

You can debug all the examples and step through the code. For multi-project applications, the OpenOCD configurations need to be setup correctly by following the instructions in this [KBA236748](https://community.infineon.com/t5/Knowledge-Base-Articles/Working-with-multi-project-applications-in-ModusToolbox-3-0-KBA236748/ta-p/393788). Edit the existing **\<Application Name> Multi-Core Debug (KitProg3_MiniProg4)** launch group to use the **Debug** configurations of **proj_cm0p** and **proj_cm4** projects. 

Once created, in the IDE, use the **\<Application Name> Multi-Core Debug (KitProg3_MiniProg4)** configuration in the **Quick Panel**. For more details, see the "Program and debug" section in the Eclipse IDE for ModusToolbox User Guide: *{ModusToolbox install directory}/ide_{version}/docs/mt_ide_user_guide.pdf*.

## Design and implementation

This code example is meant to be a companion to the [AN221111 – PSoC&trade; 6 MCU: Designing a secured system](https://www.infineon.com/an221111) application note. The code example demonstrates several concepts such as the following:

- **Secure Boot** and **Chain of Trust**
- **Shared Memory Protection Units (SMPU)** for memory protection
- **Peripheral Protection Units (PPU)** for peripheral / memory protection
- **eFuses** for lifecycle transitions and lifecyle access restrictions
- **Protection Contexts (PC)** for bus master roles and enforcing access restrictions
- **Inter-Processor Communication (IPC)** for communication between the CPUs
- **MCUboot** bootloader functionality for validating and running images
- **Device Firmware Updates (DFU)** for updating the user image
- **Crypto** block usage for image validation

These concepts are covered in detail in the application note.

## Project structure

This application template has three projects - *proj_btldr_cm0p*, *proj_cm0p* and *proj_cm4*. It also contains a *shared* directory that contains the linker scripts.

**Figure 10. Project structure**

![Project structure](./images/project-structure.png)


## Project configuration

The three projects rely on the configurations defined in the *common.mk* file. This file defines the start addresses and sizes for different memory regions and specifies certain common build configurations. The linker script symbols are also supplied by this file.

The partitions in the memory are created using the three linker scripts present in the *shared/linker_script/\<BSP>/TOOLCHAIN_GCC_ARM* directory.

**Table 1. Linker scripts**

File           | Description
---------------|------------
*COMPONENT_CM0P/\<device>_cm0p_btldr.ld*| Specifies the memory layout for the *proj_btldr_cm0p* project
*COMPONENT_CM0P/\<device>_cm0p.ld* | Specifies the memory layout for the *proj_cm0p* project
*COMPONENT_CM4/\<device>_cm4.ld* | Specifies the memory layout for the *proj_cm4* project

The linker scripts contain symbols that specify the starting address and length of each of the memory regions. The values for these symbols are supplied using the *common.mk* file as explained below.


### Flash memory layout

The flash has been divided into the following sections:

- **Bootloader** – For running the MCUboot Bootloader project
- **Protected memory** – For storing confidential data or keys. See [Protected storage](#protected-storage) for more information.
- **Primary slot** – For running the CM0+ and CM4 user projects
- **Secondary slot** – For storing the new dual-CPU firmware image
- **Scratch** (Supported only on PSoC&trade;6 2M) – For supporting MCUboot swap-based upgrade operation. See [Swap-based upgrade for PSoC&trade; 6 MCU](#swap-based-upgrade-for-psoctrade-6-mcu) for more information.

The flash memory layout is illustrated as follows for different memory variants of PSoC&trade; 62/63:

**Figure 11. Flash memory layout**

![Flash memory layout](./images/flash-memory-layout.png)


### SRAM memory layout

The SRAM has been divided into the following sections:

- **CM0+ bootloader/app** – Scratch-pad memory for CM0+ bootloader or CM0+ user project. Note that this is shared because the SRAM region can be reused between the bootloader and the CM0+ user projects.
- **Shared SRAM** – Used by both the CM0+ and CM4 for sharing any data.
- **CM4 app** – Scratchpad memory for CM4 project.

The SRAM memory map is illustrated below for different memory variants of PSoC&trade; 62/63:

**Figure 12. SRAM memory layout**

![SRAM memory layout](./images/sram-memory-layout.png)


### Configuring the common make variables

This section explains the important make variables common to all three applications that affect their functionality. You can either update these variables directly in the Makefile or pass them along with the make build command.


#### Build control variables

**Table 2. Build control variables**

Variable | Default value | Description
 -------- | ------------- |------------
`IMG_TYPE` | BOOT  | Valid values: BOOT, UPGRADE<br>**BOOT:** Use when the image is built for the primary slot. The `--pad` argument is not passed to the *imgtool*. <br/>**UPGRADE:** Use when the image is built for the secondary slot.  The `--pad` argument is passed to the *imgtool*.<br>Also, the CM0+ blinky project defines the LED toggle delay differently depending on whether the image is BOOT type or UPGRADE type.
`SWAP_UPGRADE` | 0 | Set this to '0' when the upgrade image needs to be overwritten into the primary slot. Swap-based upgrade is not supported in this code example. It is left to the user to implement it using [PSoC&trade;6 MCU: MCUboot-based basic bootloader](https://github.com/Infineon/mtb-example-psoc6-mcuboot-basic) code example as reference.
`USE_EXT_FLASH` | 0 | Set this to '0' when the image needs to be built for the external flash. This affects the value used for padding by the *imgtool*. The padding value is '0' for the internal flash and 0xff for the external flash. This code example does not support using an external flash. Setting this to '1' will require changes that are left to the user.
`USE_CRYPTO_HW`        | 1             | When set to '1', Mbed TLS uses the crypto block in PSoC&trade; 6 MCU for providing hardware acceleration of crypto functions using the [cy-mbedtls-acceleration](https://github.com/Infineon/cy-mbedtls-acceleration) library
`KEY_FILE_PATH` | *../proj_btldr_cm0p/keys* |Path to the private key file. Used with the *imgtool* for signing the image
`APP_VERSION_MAJOR`<br>`APP_VERSION_MINOR`<br>`APP_VERSION_BUILD` | 1.0.0 if `IMG_TYPE=BOOT`<br>2.0.0 if  `IMG_TYPE=UPGRADE` | Passed to the *imgtool* with the`-v` option in *MAJOR.MINOR.BUILD* format, while signing the image. Also available as macros to the application with the same names.
`SIGN_KEY_FILE_ECC` | cypress-test-ec-p256 | Name of the ECC private and public key files (the same name is used for both keys) |
`SIGN_KEY_FILE_RSA` | cypress-test-rsa-2048 | Name of the RSA private and public key files (the same name is used for both keys) |

#### Memory layout variables

Based on the values set for the variables listed below, the *common.mk* file calculates the start addresses and offsets, and defines the symbols to be used by the linker scripts.

**Table 3. Memory layout variables**

Variable | Default value <br>(512K) | Default value <br>(1M) | Default value <br>(2M) | Description
-------- | -------------------- | ------------------ | ------------------ | -----------
`CM0P_BTLDR_FLASH_SIZE` | 0x1C000 | 0x1C000 | 0x1C000 | Flash size of the bootloader project run by CM0+. <br>In the linker script for the bootloader project (CM0+), the `LENGTH` of the `flash` region is set to this value.
`PROTECTED_MEM_SIZE` | 0x4000 | 0x4000 | 0x4000  | Flash size of the protected memory section
`CM0P_APP_FLASH_SIZE` | 0x10000 | 0x20000 | 0x20000 | Flash size of the blinky user project run by CM0+. <br>In the linker script for the user project (CM0+), the `LENGTH` of the `flash` region is set to this value.
`CM4_APP_FLASH_SIZE` | 0x20000 | 0x50000 | 0xC0000 | Flash size of the user project run by CM4. <br>In the linker script for the user project (CM4), the `LENGTH` of the `flash` region is set to this value.
`CM0P_BTLDR_SRAM_SIZE` | 0x10000 | 0x10000 | 0x30000 | RAM size of the bootloader project run by CM0+. <br/>In the linker script for the bootloader project (CM0+), `LENGTH` of the `ram` region is set to this value.
`CM0P_APP_SRAM_SIZE` | 0x10000 | 0x10000 | 0x30000 | RAM size of the blinky user project run by CM0+. <br/>In the linker script for the blinky user project (CM0+), `LENGTH` of the `ram` region is set to this value.
`SHARED_SRAM_SIZE` | 0x8000 | 0x8000 | 0x10000 | RAM size for shared scratchpad region for user projects run by CM0+/CM4.
`CM4_APP_SRAM_SIZE` | 0x27800 | 0x27800 | 0xBF800 | RAM size of the user project run by CM4. <br/>In the linker script for the user project (CM4), `LENGTH` of the `ram` region is set to this value.<br/>In the linker script for the user project (CM4), the `ORIGIN` of the `ram` region is offset to this value, and the `LENGTH` of the `ram` region is calculated based on this value.
`MCUBOOT_SCRATCH_SIZE` | NA | NA | 0x20000 | Size of the scratch area used by MCUboot while swapping the image between the primary slot and secondary slot. Scratch area is required for swap-based upgrade. Note that this is not used in the code example but exists to support swap-based upgrade when implemented.
`MCUBOOT_HEADER_SIZE` | 0x400 | 0x400 | 0x400 | Size of the MCUboot header. Must be a multiple of 1024 (see the note below).<br>Used in the following:<br>1. In the linker script for the user project (CM0+), the starting address of the`.text` section is offset by the MCUboot header size from the `ORIGIN` of the `flash` region. This is to leave space for the header that will be later inserted by the *imgtool* during post-build steps.  <br/>2. Passed to the *imgtool* while signing the image. *imgtool* fills the space of this size with zeroes (or 0xFF depending on internal or external flash) and then adds the actual header from the beginning of the image.
`MCUBOOT_SECTOR_SIZE` | 512 | 512 | 512 | Flash row size; used only when swap upgrade is enabled.
`MCUBOOT_MAX_IMG_SECTORS` | 128 | 128 | 128 | Maximum number of flash sectors (or rows) per image slot for which swap status is tracked in the image trailer. This value can be simply set to `MCUBOOT_SLOT_SIZE/MCUBOOT_SECTOR_SIZE`. For PSoC&trade; 6 MCUs, `MCUBOOT_SECTOR_SIZE`=512 bytes. When `MCUBOOT_SLOT_SIZE`=0x10000, `MCUBOOT_MAX_IMG_SECTORS` would be 128 (0x10000/512). The minimum number of sectors accepted by the MCUboot library is 32, so `MCUBOOT_MAX_IMG_SECTORS` is set to 32 if `MCUBOOT_SLOT_SIZE/MCUBOOT_SECTOR_SIZE` is less than 32. <br><br>Used in the following:<br>1. In the *common.mk* file, this value is used in `DEFINE+=` to override the macro with the same name in *mcuboot/boot/cypress/MCUBootApp<br>/config/mcuboot_config/mcuboot_config.h*.<br>2. In the CM4 user project Makefile, this value is passed with the `-M` option to the *imgtool* while signing the image. *imgtool* adds padding in the trailer area depending on this value. <br/>
`MCUBOOT_IMAGE_NUMBER` | 1 | 1 | 1 | The number of images supported in the case of multi-image bootloading. This code example supports only one image.

<br>

## Bootloader implementation

The bootloader is designed based on the [PSoC™ 6 MCU: MCUboot-based basic bootloader](https://github.com/Infineon/mtb-example-psoc6-mcuboot-basic) repo in GitHub. It is customized in this example to configure system security.

MCUboot effectively divides the memory into primary and secondary slots for the user application. This code example uses a custom memory map. To achieve this, `CY_FLASH_MAP_EXT_DESC` is defined in the *common.mk* file to override the default memory map. The custom memory partitions are then defined in the *proj_btldr_cm0p/source/cy_flash_map.h* file.

The user image running in the primary slot is a combination of CM0+ and CM4 projects and is directly programmed into the internal flash to quickly evaluate the MCUboot operation.

**Note:** This example does not support programming the user image into the external flash. The example should be modified to support this if required. See [mtb-example-psoc6-mcuboot-basic](https://github.com/Infineon/mtb-example-psoc6-mcuboot-basic) to understand how this can be done.

The upgrade application that can download the upgrade image over a wired or wireless communication interface will write the image into the secondary slot. This code example uses UART wired transfer for downloading the upgrade image. For wireless transfers, see [mtb-example-anycloud-ota-mqtt](https://github.com/Infineon/mtb-example-anycloud-ota-mqtt) which is implemented using the [anycloud-ota](https://github.com/Infineon/anycloud-ota) middleware.

This code example uses ModusToolbox&trade; software resources such as BSPs and PSoC&trade; 6 MCU to provide a rich development experience that aligns well with other code examples based on ModusToolbox&trade; software. Figure 13 shows the execution flow of the code example.

**Figure 13. Boot flow**

![Boot flow](images/boot-flow.png)

The MCUboot bootloader project is considered the "secure" project and is responsible for setting up the protection units, the TOC2 and eFuse values for lifecycle mode transitions and access restrictions.


### MCUboot basics

[MCUboot](https://github.com/mcu-tools/mcuboot) is a library that helps to implement secured bootloader applications for 32-bit MCUs.

MCUboot works by dividing the flash into two slots per image – primary and secondary. The first version of the application is programmed into the primary slot during production. A firmware update application running in the device receives the upgrade image over a wired or wireless (over-the-air or OTA) communication interface and places it in the secondary slot. This slot-based partition helps in read-/write-protecting the primary slot from a less privileged application.

Typically, a bootloader application executes in secured mode and is privileged to access the primary slot while a less-privileged application such as an OTA application cannot access the primary slot, but it can access the secondary slot.

MCUboot always boots from the primary slot and copies the image from the secondary slot into the primary slot when an upgrade is requested. The upgrade can be either overwrite-based or swap-based. In an overwrite-based upgrade, the image in the primary slot is lost and there is no way to roll back if the new image has an issue. In a swap-based upgrade, the images are swapped between the two slots and a rollback is possible. In this case, MCUboot makes use of an additional area in the flash called *scratch area* for reliable swapping. MCUboot for PSoC&trade; 6 MCU supports both swap-based and overwrite-based upgrades.

Each image slot contains the metadata which is used by MCUboot to determine the current state and what actions should be taken during the current boot operation. In the case of an upgrade image, the `img_ok` field is updated by the application to make the current image permanent in the primary slot. See [image trailer](https://github.com/mcu-tools/mcuboot/blob/v1.8.1-cypress/docs/design.md#image-trailer) for more details.

MCUboot implements reset recovery and can resume the copy operation if a reset or power failure occurs in the middle. MCUboot also supports multi-image bootloading where several pairs of primary and secondary slots can exist. In this case, MCUboot can update each image independently; that is, update the image in any primary slot using the image from the corresponding secondary slot. However, MCUboot always boots from the primary slot of image 0. The image 0 application, once booted, can boot other images as required.


### Swap-based upgrade for PSoC&trade; 6 MCU

There are three types of swap modes supported in MCUboot – scratch, move, and using a status partition. Only swap mode using status partition can be used with PSoC&trade; 6 MCU devices because of the hardware restriction of the large minimum flash write/erase size.

The MCUboot library is designed with the minimum flash to write/erase size to be 8 bytes or less. This is to ensure that data is not lost when writing to the flash sector status so that it is a single-cycle operation ensuring the robustness of the application.

Because PSoC&trade; 6 MCU devices have large minimum flash write/erase size, swap using status partition has been implemented. Using this algorithm, a separate area in the internal flash is used to store swap status values and the image trailer data such as the swap size and info, boot image magic value, and the 'image ok' field.

In overwrite mode, the image trailer is added to the end of the application image. The `Cy_DFU_ValidateApp` function in the *proj_cm4/source/dfu_user.c* file, part of the CM4 user project, is responsible for validating the image received. The function iterates through the image header and image trailer to find the hash and signature to be verified.

```
A +---------------------+
  | Header              | <- struct image_header
  +---------------------+
  | Payload             |
  +---------------------+
  | TLV area            |
  | +-----------------+ |    struct image_tlv_info with
  | | TLV area header | | <- IMAGE_TLV_PROT_INFO_MAGIC (optional)
  | +-----------------+ |
  | | Protected TLVs  | | <- Protected TLVs (not used)
B | +-----------------+ |
  | | TLV area header | | <- struct image_tlv_info with IMAGE_TLV_INFO_MAGIC
C | +-----------------+ |
  | | SHA256 hash     | | <- Hash from A - B
D | +-----------------+ |
  | | Keyhash         | | <- Indicates which pub. key for sig
  | +-----------------+ |
  | | Signature       | | <- Signature from C - D, only hash
  | +-----------------+ |
  +---------------------+
```

However, in swap-based upgrade, the image header and trailer are not added to the end of flash but instead assigned a separate area in the internal flash. It is left to the user to modify the `Cy_DFU_ValidateApp` function to support swap-based upgrade. **The code example does not support swap-based upgrade by default.**

See the "Swap status partition description" section of the [MCUboot app documentation](https://github.com/mcu-tools/mcuboot/blob/v1.8.1-cypress/boot/cypress/MCUBootApp/MCUBootApp.md).

See [MCUboot design](https://github.com/mcu-tools/mcuboot/blob/v1.8.1-cypress/docs/design.md) documentation for details.


### Flash map/partition

Figure 14 shows the default flash map or partition used with MCUboot. The partitions need not be contiguous in the memory because it is possible to configure the offset and size of each partition. However, the offset and the size must be aligned to the boundary of a flash row or sector. For PSoC&trade; 6 MCUs, the size of a flash row is 512 bytes. Also, the partition can be in either the internal flash or external flash.

The memory partition is described or defined through a flash map (a data structure). It is important that the bootloader project and the dual-CPU user projects agree on the flash map. This example uses a shared file (*common.mk*) between the three apps so that they can use the same set of flash map parameters. See [Configuring the flash map](#configuring-the-default-flash-map) for details.

**Figure 14. Typical flash map**

![Typical flash map](images/typical-flash-map.jpg)


#### Customizing the flash map

A default flash map is defined through the `boot_area_descs` variable in the *libs/mcuboot/boot/cypress/cy_flash_pal/cy_flash_map.c* file; this variable is constructed using the macros defined in the *libs/mcuboot/boot/cypress/MCUBootApp/sysflash/sysflash.h* file.

You can either configure the parameters such as the bootloader size and slot size that constitute the default flash map or completely override the default flash map.

Because this code example required a custom flash map, the following modifications were done:

- Added `DEFINES+=CY_FLASH_MAP_EXT_DESC` in the *common.mk* file.
- Defined and initialized the `struct flash_area *boot_area_descs[]` variable using the *proj_btldr_cm0p/source/cy_flash_map.h* file to create the custom flash map.

See [Table 3. Memory layout variables](#memory-layout-variables) for details on configuring these parameters.


### Configuring bootloader make variables

This section explains the important make variables in the *Makefile* that affect the MCUboot functionality. You can either update these variables directly in the Makefile or pass them along with the `make build` command.

The make variables depend on the values set in the *common.mk* file. The variables mostly define the MCUboot files to include for the build. Additionally, it contains make targets for key generation as explained in the [Generating a key-pair](#generating-a-key-pair) section.

The post-build steps generate the SHA-256 digest and then sign it with the RSA-2048 private key to generate the signed HEX file. The original HEX file is overwritten with the signed HEX file so that programming of the HEX can be done directly without any modifications to OpenOCD configurations.


## CM0+ blinky user project implementation

The CM0+ blinky user project toggles an LED at an interval decided by the value of `IMG_TYPE`.

**Table 4. LED intervals for different image types**

| IMG_TYPE | LED interval |
|----------|--------------|
| BOOT     | 1000 ms      |
| UPGRADE  | 250 ms       |

The LED pin mapping for different BSPs is as shown below.

**Table 5. Blinky LED pin assignment**

| BSP                    | LED pin      |
|------------------------|--------------|
| CY8CKIT-062-BLE        | P1_5         |
| CY8CKIT-062-WIFI-BT    | P1_5         |
| CY8CPROTO-062-4343W    | P13_7        |
| CY8CKIT-062S2-43012    | P1_5         |
| CY8CPROTO-063-BLE      | P6_3         |
| CY8CPROTO-062S3-4343W  | P11_1        |


Additionally, it also demonstrates how [protected storage](#protected-storage) and [IPC](#inter-processor-communication-ipc) can be implemented.


### Protected storage

Protected storage is an area in the flash that can be used to store any critical data or keys that should be accessed only by the secure CM0+ processor. This area has been protected using SMPUs to allow access only to CM0+. This data can be exchanged with CM4 if needed via IPC.

In this code example, dedicated IPC channels 8 and 9 are assigned to CM0+ and CM4 respectively. Interrupts are set up on both cores to receive the notify and release interrupts.

IPC System Pipes must be placed in the shared SRAM so that both the cores can access them. IPC system pipes are placed in the `.cy_sharedmem` section of the memory by default if the section is defined in the linker script. This section has been defined in the linker script to be part of the last 8 KB of the shared SRAM.

Additionally, user IPC semaphores used for message passing need to be placed in the shared SRAM so that both the cores can access them. For this reason, another section (`.shared_ram`) is defined in the shared SRAM.

Shared SRAM distribution is as follows:

**Figure 15. Shared SRAM distribution**

![Shared SRAM distribution](./images/shared_sram_distribution.png)

The linker scripts define the placement of both these sections and their respective sizes.

```
IPC_SYSTEM_PIPES_SIZE = 0x2000;

MEMORY
{
   ...
   shared_ram        (rwx)   : ORIGIN = SHARED_SRAM_START, LENGTH = SHARED_SRAM_SIZE
   ...
}

SECTIONS
{
   ...
   ...

    .shared_ram (NOLOAD):
    {
        . = ALIGN(4);
        __shared_ram_start__ = .;
        KEEP(*(.shared_ram))
        . = ALIGN(4);
        __shared_ram_end__ = .;
    } > shared_ram

    .cy_sharedmem ORIGIN(shared_ram) + LENGTH(shared_ram) - IPC_SYSTEM_PIPES_SIZE (NOLOAD):
    {
        . = ALIGN(4);
        __public_ram_start__ = .;
        KEEP(*(.cy_sharedmem))
        . = ALIGN(4);
        __public_ram_end__ = .;
    } > shared_ram

    ...
    ...
}
```

CM0+ sets up its IPC channels and interrupts, and then proceeds to toggle the LED. When a message is received on channel 8 from CM4, the interrupt callback is triggered and the device ID data stored in the protected storage is sent to CM4 on channel 9. This is a simple demonstration of how IPCs can be used. This can be expanded based on user application.


### Configuring CM0+ project make variables

This section explains the important make variables in the Makefile that affect the CM0+ user project functionality. You can either update these variables directly in the Makefile or pass them along with the `make build` command.

In the *proj_cm0p* Makefile, the following line was added to specify the application CPU:

```
CORE=CM0P
```

If you intend to use the *design.modus*-generated code in CM0+, add `BSP_DESIGN_MODUS` to `COMPONENTS=` in the CM0+ Makefile.

The following defines were added for use in the project:

**Table 6. Variables defined in the application**

Variable | Default value | Description
-------- | -------------------- | ------------------ |
`KIT_NAME` | `APPNAME` with '-' replaced with '_' | Used to specify the kit used in the project
`CY_CORTEX_M4_APPL_ADDR` | `CM4_APP_FLASH_START` | Specifies the start address of the CM4 project. The `CM4_APP_FLASH_START` value is supplied by the *common.mk* file.
`CY_IPC_DEFAULT_CFG_DISABLE` | 1 | Disables the default IPC configuration that comes with the BSP


## CM4 FreeRTOS user project implementation

The example uses FreeRTOS to set up a *dfu_task* and call the scheduler. The *dfu_task* execution flow is as follows:

1.Initialize the DFU.
2. Set up IPC pipes on CM4 to request the unique device ID from CM0+.
3. Initialize the DFU status LED.
4. Initialize retarget-io for UART debug logs.
5. Start DFU transport.
6. Request unique device ID from CM0+ using IPC pipes.
7. Enter a `while` loop and wait for IPC callback or DFU host commands.

The DFU status LED and UART pin mapping for different BSPs are as follows:

**Table 7. DFU status LED and UART connections**

BSP                    | LED pin      | UART pin Tx        | UART pin Rx
-----------------------|--------------|--------------------|--------------
CY8CKIT-062-BLE        | P13_7        | P13_1              | P13_0
CY8CKIT-062-WIFI-BT    | P13_7        | P13_1              | P13_0
CY8CPROTO-062-4343W    | N/A          | P13_1              | P13_0
CY8CKIT-062S2-43012    | P11_1        | P12_1              | P12_0
CY8CPROTO-063-BLE      | P7_1         | P9_1               | P9_0
CY8CPROTO-062S3-4343W  | N/A          | P3_1 / B_RX        | P3_0 / B_TX

The DFU UART requires an external USB-to-UART bridge to be connected to communicate with your PC. You can either use a snapped-out KitProg3 board or MiniProg4 or any FTDI USB-UART converters for this purpose. The connections are as follows:

**MiniProg4 connections**

The MiniProg 3/4 UART connections are found on the 6x2 connector. See the [MiniProg4 user guide](https://www.infineon.com/dgdl/Infineon-CY8CKIT-005_MiniProg4_Program_and_Debug_Kit_Guide-UserManual-v01_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0f011df41849) for more information.

**Table 8. MiniProg4 UART connections**

6x2 connector pin index | 6x2 connector pin name | PSoC&trade;6 BSP
------------------------|------------------------|------------
1                       | VTARG                  | P6_VDD
12                      | GND                    | GND
8                       | RX                     | UART Pin TX
6                       | TX                     | UART Pin RX

<br>

**KitProg3 connections**

**Table 9. KitProg3 UART connections**

KitProg3 pins | PSoC&trade;6 BSP
-----------------------|------------
VTARG                  | P6_VDD
GND                    | GND
RX                     | UART pin TX
TX                     | UART pin RX


<br>

**FTDI USB-UART converters**

**Table 10. FTDI USB-UART connections**

Converter Pins         | PSoC&trade;6 BSP
-----------------------|--------------
RX                     | UART pin TX
TX                     | UART pin RX

<br>

### Protected storage

CM4 can access critical data stored in the protected storage area by requesting it from CM0+ via IPC.

In this code example, dedicated IPC channels 8 and 9 are assigned to CM0+ and CM4 respectively. Interrupts are set up on both cores to receive the notify and release interrupts.

CM4 sets up its IPC channels and interrupts, and then sends a message to CM0+ on channel 8 requesting for the device ID. The device ID sent on channel 9 by CM0+ is received by CM4 which is then printed on the serial terminal. This is a simple demonstration of how IPCs can be used. This can be expanded based on the user application.


### Device Firmware Update (DFU)

The *dfu_task* continuously monitors the UART channel for host commands to initiate the DFU transfer. When the DFU transfer is initiated by the host, the data is received via UART and written into the secondary slot.

Because this application uses MCUboot, the trailer of the upgrade image has the following format:

```
A +---------------------+
  | Header              | <- struct image_header
  +---------------------+
  | Payload             |
  +---------------------+
  | TLV area            |
  | +-----------------+ |    struct image_tlv_info with
  | | TLV area header | | <- IMAGE_TLV_PROT_INFO_MAGIC (optional)
  | +-----------------+ |
  | | Protected TLVs  | | <- Protected TLVs (not used)
B | +-----------------+ |
  | | TLV area header | | <- struct image_tlv_info with IMAGE_TLV_INFO_MAGIC
C | +-----------------+ |
  | | SHA256 hash     | | <- Hash from A - B
D | +-----------------+ |
  | | Keyhash         | | <- Indicates which pub. key for signature.
  | +-----------------+ |
  | | Signature       | | <- Signature from C - D; only hash.
  | +-----------------+ |
  +---------------------+
```

Once the upgrade image is received, the execution iterates through the trailer to get the image size, hash, and signature to do the validation. The public key located in the *ecc-public-key-p256.h* file is used for the verification. The `Cy_DFU_ValidateApp` function in the  *dfu_user.c* file implements this logic. See the following for detailed series of steps used to validate the image:

**Figure 16. DFU upgrade image validation flow**

![DFU upgrade image validation](./images/dfu_validation_flow.png)

**Note:** The public key generated using imgtool is in `DER` format. Crypto APIs require the keys to be in `ASN.1` format. Therefore, the public key is converted from DER to ASN.1 format before passing it to crypto APIs. See the `extract_pub_key` function in the *dfu_user.c* file to learn more.

### Configuring CM4 project make variables

This section explains the important make variables in the Makefile that affect the CM4 user project functionality. You can either update these variables directly in the Makefile or pass them along with the `make build` command.

The following defines were added for use in the project:

**Table 11. CM4 make variables**

Variable | Default value | Description
-------- | -------------------- | ------------------ |
`HEADER_OFFSET`   | 0 | Starting address of the CM0+/CM4 user projects or the offset at which the header of an image  begins. *Image = Header + CM0+ project + CM4 project + TLV + trailer*. <br>New relocated address = ORIGIN + HEADER_OFFSET<br/>ORIGIN is defined in the CM0+ linker script and is usually the address next to the end of the bootloader image.
`KEY_FILE_PATH` | *proj_btldr_cm0p/keys* | Path to the *keys* folder
`APP_VERSION_MAJOR`<br>`APP_VERSION_MINOR`<br>`APP_VERSION_BUILD` | 1.0.0 if `IMG_TYPE=BOOT`<br>2.0.0 if  `IMG_TYPE=UPGRADE` | Passed to the *imgtool* with the`-v` option in *MAJOR.MINOR.BUILD* format, while signing the image. Also available as macros to the application with the same names.
`COMPONENTS` | FREERTOS<br>CUSTOM_DESIGN_MODUS | Includes the FreeRTOS library and uses a custom *design.modus* file
`DISABLE_COMPONENTS` | CM0P_SLEEP<br>BSP_DESIGN_MODUS | Disables using pre-built CM0+ image and default BSP *design.modus* file
`CY_IPC_DEFAULT_CFG_DISABLE` | 1 | Disables the default IPC configuration that comes with the BSP

Each project should have its own *deps* folder. If the same library is used by both projects, it should be in the *deps* folder of both projects. If the library location is specified as the shared asset repo in the *mtb* file (which is by default), they will both automatically access it from the shared location.

When the CM4 project is built, it builds both the CM0+ and CM4 projects; the resulting build artifacts contain code for both projects. When the user image starts, the CM0+ project starts first. It is responsible for starting CM4.


## Security overview

In this code example, the bootloader is responsible for setting up the security for the entire system. See [AN221111 – PSoC&trade; 6 MCU: Designing a secured system](https://www.infineon.com/an221111) for detailed information.


### Secured boot

The boot parameters are specified using the TOC2 structure variable `cy_toc2` in the *proj_btldr_cm0p/source/main.c* file.

**Table 12. TOC2 contents**

Parameter | Description
----------|-------------
Object size | Size of the application image in bytes
Magic number | TOC2 ID (magic number = 0x1211220)
User key address | Address of the user key
SMIF configuration address | SMIF configuration structure (optional)
Application 1 address |  First user application address
Application 1 format | First user application format (only CYSAF supported)
Application 2 address |  Second user application address
Application 2 format | Second user application format (only CYSAF supported)
Secure hash objects | Number of hash objects to be included for verification
Signature key address | Address of the signature key (public key used to sign the bootloader image)
Additional objects | Additional objects to be included in secure hash
TOC2 flags | Flags in TOC to control the flash boot options
CRC | CRC16-CCITT checksum of the TOC2 structure

<br>

In this code example, the TOC2 structure is set to the following:

```
/* TOC2 in SFlash */
CY_SECTION(".cy_toc_part2") __USED static const cy_stc_ps_toc_t cy_toc2 =
{
		.objSize     = sizeof(cy_stc_ps_toc_t) - sizeof(uint32_t),  /* Object Size (Bytes) excluding CRC */
		.magicNum    = CY_PS_TOC2_MAGICNUMBER,                      /* TOC2 ID (magic number) */
		.userKeyAddr = (uint32_t)&CySecureKeyStorage,               /* User key storage address */
		.smifCfgAddr = 0UL,                                         /* SMIF config list pointer */
		.appAddr1    = CY_START_OF_FLASH,                           /* App1 (MCUBoot) start address */
		.appFormat1  = CY_PS_APP_FORMAT_CYPRESS,                    /* App1 Format */
		.appAddr2    = 0,                                           /* App2 (User App) start address */
		.appFormat2  = 0,                                           /* App2 Format */
		.shashObj    = 1UL,                                         /* Include public key in the SECURE HASH */
		.sigKeyAddr  = (uint32_t)&SFLASH->PUBLIC_KEY,               /* Address of signature verification key */
		.tocFlags    = CY_PS_FLASHBOOT_FLAGS,                       /* Flashboot flags stored in TOC2 */
		.crc         = 0UL                                          /* CRC populated by cymcuelftool */
};
```

**Note:** Application 1 here is the MCUboot bootloader and Application 2 is the user application. Passing the start address tells flash boot to validate the images before passing control to them. In this code example, because MCUboot is responsible for validating and launching the user application images, the Application 2 start address is not passed and therefore set to '0'.


### Protection units

The flash and SRAM are divided into different sections as follows:

**Figure 17. Protection unit memory map (device with 1-MB flash)**

![Protection unit memory map](images/smpu-configuration.png)

Each section is protected using the shared memory protection unit (SMPU) with a pre-defined protection context (PC). The following table shows the protection units configured in this example for a PSoC&trade; 6 MCU device with 1 MB of flash.

**Table 13. Protection units configuration for 1M devices**

Section | Bus master | Memory | SMPU  | Start address | Size | Access attributes | Secure | Protection context  |
:--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
CM0+ bootloader | CM0+ | Flash | 13 | 0x1000_0000 | 112 KB | R/X | Yes | PC = 1,2 |
Protected memory | CM0+ | Flash | 7 | 0x1001_C000 | 16 KB | R/W/X | Yes | PC = 2 |
CM0+ Project | CM0+ | Flash | 11 | 0x1002_0000 | 128 KB | R/W/X | Yes | PC = 1,2 |
CM4 Project + Secondary slot | CM4 | Flash | 9 | 0x1004_0000 | 320 KB + 448K | R/W/X | No | PC = 1,4 |
CM0+ bootloader / User Project | CM0+ | SRAM | 12 | 0x0800_0000 |  64 KB | R/W/X | Yes | PC = 1,2 |
Shared SRAM | CM0+ / CM4 | SRAM | 10  | 0x0801_0000 | 32 KB |  R/W | No | PC = 1,2,4 |
CM4 SRAM | CM4 | SRAM | 5 | 0x0801_8000 | 160 KB | R/W/X | No | PC = 4 |

<br>

Applications using other PSoC&trade; 6 MCU devices have a similar SMPU protection unit configuration with changes in start addresses and regions sizes.

**Figure 18. Protection unit memory map (device with 2-MB flash)**

![Protection unit memory map (device with 2-MB flash)](images/smpu-configuration-2m.png)

<br>

**Figure 19. Protection unit memory map (device with 512-KB  flash)**

![Protection unit memory map (device with 512-KB  flash](images/smpu-configuration-512k.png)

<br>

**Note:** If the device has 288 KB of SRAM, use `SRAM_SIZE` = 256 KB instead.

The CM0+ CPU configures all the SMPU and PC. It also configures the bus master to be assigned to a PC. To learn how to configure the SMPU, see [this blog post](https://community.infineon.com/t5/Resource-Library/Protecting-memory-regions-in-PSoC6/ta-p/246618). Once all the protection units are configured, CM0+ transitions the following bus masters to their respective PC values:

**Table 14. Protection contexts assignment**

Bus master | PC
---   | :---
CM0+ | 1
CM4 | 4
Test controller | 5

<br>

Once the CM0+ bus master's PC is set to '1', it locks all the protections in place.

<br>

### eFuse programming for debug access restrictions and lifecycle

eFuses are responsible for lifecycle transitions and access restrictions for each lifecyle. See the [AN221111 – PSoC&trade; 6 MCU: Designing a secured system](https://www.infineon.com/an221111) for information on eFuse settings and offsets.

Programming the eFuses is irreversible and care should be taken to verify the settings before blowing them. **Incorrect settings may brick the device permanently**.

To program the eFuse, the following steps should be followed.

1. Open CYPRESS&trade; Programmer. Connect to the kit and click **Erase**. This erases any prior configuration residing on the flash and SFlash.

2. Build and program the CM0+/CM4 dual-CPU user projects by selecting  either the *primary_app.hex* or *primary_app_boot.hex* file in CYPRESS&trade; Programmer.

   **Note:** It is important to have the user image programmed before the bootloader is programmed. This is because the bootloader programs the eFuses. If programmed first, the bootloader will lock down the debug ports and the user image can no longer be programmed.

3. Edit the *proj_btldr_cm0p/source/cy_ps_efuse.h* file and set the `CY_EFUSE_AVAILABLE` macro to '1'. This step causes the eFuse data to be included in the HEX file.
   ```
   #define CY_EFUSE_AVAILABLE 1
   ```

4. Verify the eFuse configuration defined in `cy_efuse_data`.

5. Clean and build the *proj_btldr_cm0p* project.

6. Open CYPRESS&trade; Programmer and select the *proj_btldr_cm0p.hex* file along with the following settings:

   1. Select **Program Security Data**.
   2. Set the voltage to 2.5 V (**Note:** This option isn't visible for all BSPs. In such cases, set the VTARG jumpers to 2.5 V on the kit.)
   3. Set **SFlash restrictions** to "Erase/Program USER/TOC/KEY allowed".

   <br>

   **Note:** CYPRESS&trade; Programmer will indicate the current operating voltage at the bottom right as highlighted in the figure. If you do not see 2.5 V even after setting the **Voltage** option to 2.5 V, check if the kit has a VDD select switch / jumper and change it to 2.5 V or 1.8 V - 3.3 V variable voltage.

   <br>

   **Figure 20. Programming eFuse using CYPRESS&trade; Programmer**

   ![Programming eFuse using CYPRESS&trade; Programmer](./images/program-efuse.png)

7. Program the kit.

   Once programming is complete, a reset will be executed. If the eFuses are configured to disable the debug ports, CYPRESS&trade; Programmer will not be able to connect to the kit indicating successful locking of the debug ports. The current lifecycle stage will be indicated in the serial terminal.

</details>

**Note:** In SECURE lifecycle mode, if secure access restrictions are set to enable debug access ports, you must configure the GPIOs for the debugger to get access to the debug ports. This is demonstrated in the `configure_swj` function in *proj_btldr_cm0p/source/main.c*. By default, this function is disabled, but can be enabled by setting the `CONFIGURE_SWJ_PINS` macro to '1'.

```
#define CONFIGURE_SWJ_PINS    (1u)
```

### MCUboot security

MCUboot checks the image integrity with SHA-256, and image authenticity with digital signature verification. Multiple signature algorithms are supported; this example enables ECDSA SECP256R1 (EC256) by default. MCUboot uses the Mbed TLS library for cryptography.

PSoC&trade; 6 MCU supports hardware-accelerated cryptography based on the Mbed TLS library via a shim layer. The [cy-mbedtls-acceleration](https://github.com/Infineon/cy-mbedtls-acceleration) library implements this layer.

Hardware-accelerated cryptography shortens the boot time by more than four times compared to the software implementation (observation results). To enable it in the bootloader project, set `USE_CRYPTO_HW=1` in the *common.mk* file. This is enabled by default.

MCUboot verifies the signature of the image in the primary slot every time before booting if `MCUBOOT_VALIDATE_PRIMARY_SLOT` is defined. In addition, it verifies the signature of the image in the secondary slot before copying it to the primary slot.

This example enables image authentication by uncommenting the following lines in the *proj_btldr_cm0p/libs/mcuboot/boot/cypress/MCUBootApp/config/mcuboot_config/mcuboot_config.h* file:

```
#define MCUBOOT_SIGN_EC256
#define NUM_ECC_BYTES (256 / 8)
.
.
.
#define MCUBOOT_VALIDATE_PRIMARY_SLOT
```

When these options are enabled, the public key is embedded within the bootloader project. The dual-CPU user project is signed using the private key during the post-build steps. The *imgtool* Python module included in the MCUboot repository is used for signing the image.

This example includes a sample key-pair under the *proj_btldr_cm0p/keys* directory. **You must not use this key-pair in your end-product.** See [Generating a key-pair](#generating-a-key-pair) for generating a new key-pair.


### Generating a key-pair

Key generation is supported directly by make commands defined in *proj_btldr_cm0p/Makefile*.

To execute the make commands, open "modus-shell" in Windows or any terminal application in macOS/Linux. In Eclipse IDE, modus-shell can be accessed directly in the terminal window.

Navigate to the *proj_btldr_cm0p* project folder.
```
cd <workspace_path>/mtb-example-psoc6-security/proj_btldr_cm0p/
```
The make commands support ECC and RSA key generation as listed below.


#### Generating an RSA key-pair

1. In the terminal, run the following command:

   ```
   make rsa_keygen
   ```
   The RSA private key *rsa_private_generated.txt* is generated in the *proj_btldr_cm0p/keys* folder. The RSA public key is generated in a file named *rsa_to_c_generated.txt*. The contents of the *rsa_private_generated.txt* file are automatically copied over to the *cypress-test-rsa-2048.pem* file.

2. Copy the `.moduleData[]`, `.expData[]`, `.barrettData[]`, `.inverseModuleData[]`, and `.rBarData[]` arrays from *rsa_to_c_generated.txt* and replace the ones in *proj_btldr_cm0p/source/cy_ps_keystorage.c*.

   The names of the files used for signing user application images can be modified by changing the `SIGN_KEY_FILE_RSA` variable in *common.mk* file.

   **Note:** If you run only the `make rsa_keygen` script but do not copy the updated C code to the *cy_ps_keystorage.c* file, your secured boot will fail because the image is signed with a newer private key that is not compatible with the older public key.

   **Figure 21. Generating the RSA key**

   ![Generating the RSA key](./images/rsa-key-generation.png)


#### Generating the ECC key-pair

1. In the terminal, run the following command:

   ```
   make ecc_keygen
   ```
   This creates the following files in the *proj_btldr_cm0p/keys* folder:

   - *cypress-test-ec-p256.pem* (private key)
   - *cypress-test-ec-p256.pub* (public key in a C-like array)
   - *ecc-public-key-p256.h* (public header file in a C-like array)

2. Copy the contents of *cypress-test-ec-p256.pem* to *ecc-private-key-p256.h* in a string-based format.

This header file can be used for establishing the identity during transactions with any host. Note that the code example does not use it; it is left to the users for implementing it based on their use case.

The `Cy_DFU_ValidateApp` function in *proj_cm4/source/dfu_user.c* part of the CM4 user project makes use of the public key generated in *ecc-public-key-p256.h* for validating the image received.

The names of the files generated can be modified by changing the `SIGN_KEY_FILE_ECC` variable in *common.mk* except the public key header file name (*ecc-public-key-p256.h*) which is kept constant and cannot be changed because it is included as a header file in the application.

**Figure 22. Generating the ECC key**

![Generating the ECC key](./images/ecc-key-generation.png)



## Pre- and post-build steps

### Bootloader project: Pre-build steps

The pre-build steps are specified through the `PREBUILD` variable in *proj_btldr_cm0p/Makefile*.

1. *Initialize the Git submodules for MCUboot:* This is required because the `make getlibs` command currently does not support initializing Git submodules while cloning a repo. This step executes only if the *libs/mcuboot/ext/mbedtls* directory (a submodule) does not exist or if the contents of the directory are empty.


### Bootloader project: Post-build steps

The post-build steps are specified through the `POSTBUILD` variable in *proj_btldr_cm0p/Makefile*. The following actions are performed:

1. Creates a backup of the original HEX and ELF files with the suffix "_unsigned" added to the filename

2. Deletes the original HEX and ELF file

3. Signs the "_unsigned.elf" file with the RSA-2048 private key using CyMCUElfTool to create the signed HEX and ELF files. These files are named the same as the original HEX and ELF files. This is because OpenOCD configurations use the `APPNAME`.hex/elf for programming and debugging.


### CM0+/CM4 dual-CPU user projects: Post-build steps (for development)

The *proj_cm4* project build includes the *proj_cm0p* image. Therefore, the the post-build steps are specified through the `POSTBUILD` variable only in *proj_cm4/Makefile*.

These steps generate the signed version of the image in HEX format using the *imgtool* Python module. The `SIGN_ARGS` variable holds the arguments passed to *imgtool*. The final image is in HEX format so that PSoC&trade; 6 MCU programmer tools can directly program the image into the device. If you are generating the image to use with a firmware update application running in the device, you may need to convert the image into binary (BIN) format.

The names of the output files generated depend on the `IMG_TYPE` variable. For `IMG_TYPE=BOOT`, the files have the suffix "_BOOT" and for `IMG_TYPE=UPGRADE`, the files have the suffix "_UPGRADE".

1. Make a copy of the *\*.hex* file into a *\*_raw.hex* file.

2. Delete the *\*.hex* file because the final signed image will be generated with the same filename so that you can directly program the file either using the `make program` command or using the launch configurations in Eclipse IDE for ModusToolbox&trade;.

3. Relocate the address and generate a new *\*_unsigned.hex* file from the *\*.elf* file using the *arm-none-eabi-objcopy* tool.

4. Sign the image using *imgtool* and generate the *\*.hex* file.

5. **Only for `IMG_TYPE=UPGRADE`**, the CYACD2 file is generated from the signed HEX file using the *hextocyacd2.py* Python script; the HEX file is deleted because it is not required. The *hextocyacd2.py* Python script is located in the *proj_btldr_cm0p/scripts* folder.


### CM0+/CM4 dual-CPU user projects: Post-build steps (for production)

When the firmware is ready for production, the `PRODUCTION` Makefile variable in *proj_cm4/Makefile* can be used to merge the HEX files of all the three projects into a single binary.

To create the merged HEX file for production, set `PRODUCTION` to '1'.
```
PRODUCTION=1
```
The name of the merged HEX file generated is supplied by the `PRODUCTION_HEX_NAME` Makefile variable. This can be changed if required.

```
PRODUCTION_HEX_NAME=production
```

To merge the HEX files, cymcuelftool is used. This tool is located inside the ModusToolbox&trade; tools directory. By default, the *production.hex* file is generated in the *proj_cm4* build directory.


## Bootloader project: Custom device configuration

The bootloader project overrides the default device configuration provided in *libs/TARGET_\<kit\>\COMPONENT_BSP_DESIGN_MODUS* with the one provided in *templates/TARGET_\<kit\>* for the supported kits. The custom configuration just enables the serial communication block (SCB) in UART mode with the alias *CYBSP_UART*. The *libs/mcuboot/boot/cypress/MCUBootApp/cy_retarget_io_pdl.c* file uses this block to implement redirecting printf to UART.


## CM4 user project: Custom device configuration

The CM4 user project overrides the default device configuration provided in *libs/TARGET_\<kit\>\COMPONENT_BSP_DESIGN_MODUS* with the one provided in *templates/TARGET_\<kit\>* for the supported kits. The custom configuration just enables the SCBs in UART mode with the alias *DFU_UART* for the DFU operation. It also enables the SCB in UART mode with the alias *CYBSP_UART* to print debug logs. The *libs/mcuboot/boot/cypress/MCUBootApp/cy_retarget_io_pdl.c* file uses this block to implement redirecting printf to UART.


## Related resources

Resources  | Links
-----------|----------------------------------
Application notes  | [AN221111](https://www.infineon.com/an221111) – PSoC&trade; 6 MCU: Designing a secured system <br> [AN228571](https://www.infineon.com/AN228571) – Getting started with PSoC&trade; 6 MCU on ModusToolbox&trade; software <br>  [AN215656](https://www.infineon.com/AN215656) – PSoC&trade; 6 MCU: Dual-CPU system design
Code examples  | [Using ModusToolbox&trade; software](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software) on GitHub
Knowledge Base Articles | [KBA236748](https://community.infineon.com/t5/Knowledge-Base-Articles/Working-with-multi-project-applications-in-ModusToolbox-3-0-KBA236748/ta-p/393788) - Working with Multi-Project Applications in ModusToolbox 3.0 <br>
Device documentation | [PSoC&trade; 6 MCU datasheets](https://documentation.infineon.com/html/psoc6/bnm1651211483724.html) <br> [PSoC&trade; 6 technical reference manuals](https://documentation.infineon.com/html/psoc6/zrs1651212645947.html)
Development kits | Select your kits from the [Evaluation Board Finder](https://www.infineon.com/cms/en/design-support/finder-selection-tools/product-finder/evaluation-board) page.
Libraries on GitHub  | [mtb-pdl-cat1](https://github.com/Infineon/mtb-pdl-cat1) – PSoC&trade; 6 Peripheral Driver Library (PDL)  <br> [mtb-hal-cat1](https://github.com/Infineon/mtb-hal-cat1) – Hardware Abstraction Layer (HAL) library <br> [retarget-io](https://github.com/Infineon/retarget-io) – Utility library to retarget STDIO messages to a UART port
Middleware on GitHub  | [psoc6-middleware](https://github.com/Infineon/modustoolbox-software#psoc-6-middleware-libraries) – Links to all PSoC&trade; 6 MCU middleware
Tools  | [Eclipse IDE for ModusToolbox&trade; software](https://www.infineon.com/modustoolbox) – ModusToolbox&trade; software is a collection of easy-to-use software and tools enabling rapid development with Infineon MCUs, covering applications from embedded sense and control to wireless and cloud-connected systems using AIROC&trade; Wi-Fi and Bluetooth&reg; connectivity devices.


## Other resources

Infineon provides a wealth of data at www.infineon.com to help you select the right device, and quickly and effectively integrate it into your design.

For PSoC&trade; 6 MCU devices, see [How to design with PSoC&trade; 6 MCU – KBA223067](https://community.infineon.com/docs/DOC-14644) in the Infineon community.

## Document history

Document title: *CE234992* – *PSoC&trade; 6 MCU: Security application template*

| Version | Description of Change                                              |
| ------- | -------------------------------------------------------------------|
| 1.0.0   | New code example                                                   |
| 2.0.0   | Updated to support ModusToolbox&trade; software v3.0 and BSPs v4.x |

---------------------------------------------------------

© Cypress Semiconductor Corporation, 2020-2022. This document is the property of Cypress Semiconductor Corporation, an Infineon Technologies company, and its affiliates ("Cypress").  This document, including any software or firmware included or referenced in this document ("Software"), is owned by Cypress under the intellectual property laws and treaties of the United States and other countries worldwide.  Cypress reserves all rights under such laws and treaties and does not, except as specifically stated in this paragraph, grant any license under its patents, copyrights, trademarks, or other intellectual property rights.  If the Software is not accompanied by a license agreement and you do not otherwise have a written agreement with Cypress governing the use of the Software, then Cypress hereby grants you a personal, non-exclusive, nontransferable license (without the right to sublicense) (1) under its copyright rights in the Software (a) for Software provided in source code form, to modify and reproduce the Software solely for use with Cypress hardware products, only internally within your organization, and (b) to distribute the Software in binary code form externally to end users (either directly or indirectly through resellers and distributors), solely for use on Cypress hardware product units, and (2) under those claims of Cypress’s patents that are infringed by the Software (as provided by Cypress, unmodified) to make, use, distribute, and import the Software solely for use with Cypress hardware products.  Any other use, reproduction, modification, translation, or compilation of the Software is prohibited.
<br>
TO THE EXTENT PERMITTED BY APPLICABLE LAW, CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD TO THIS DOCUMENT OR ANY SOFTWARE OR ACCOMPANYING HARDWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  No computing device can be absolutely secure.  Therefore, despite security measures implemented in Cypress hardware or software products, Cypress shall have no liability arising out of any security breach, such as unauthorized access to or use of a Cypress product. CYPRESS DOES NOT REPRESENT, WARRANT, OR GUARANTEE THAT CYPRESS PRODUCTS, OR SYSTEMS CREATED USING CYPRESS PRODUCTS, WILL BE FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION (collectively, "Security Breach").  Cypress disclaims any liability relating to any Security Breach, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any Security Breach.  In addition, the products described in these materials may contain design defects or errors known as errata which may cause the product to deviate from published specifications. To the extent permitted by applicable law, Cypress reserves the right to make changes to this document without further notice. Cypress does not assume any liability arising out of the application or use of any product or circuit described in this document. Any information provided in this document, including any sample design information or programming code, is provided only for reference purposes.  It is the responsibility of the user of this document to properly design, program, and test the functionality and safety of any application made of this information and any resulting product.  "High-Risk Device" means any device or system whose failure could cause personal injury, death, or property damage.  Examples of High-Risk Devices are weapons, nuclear installations, surgical implants, and other medical devices.  "Critical Component" means any component of a High-Risk Device whose failure to perform can be reasonably expected to cause, directly or indirectly, the failure of the High-Risk Device, or to affect its safety or effectiveness.  Cypress is not liable, in whole or in part, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any use of a Cypress product as a Critical Component in a High-Risk Device. You shall indemnify and hold Cypress, including its affiliates, and its directors, officers, employees, agents, distributors, and assigns harmless from and against all claims, costs, damages, and expenses, arising out of any claim, including claims for product liability, personal injury or death, or property damage arising from any use of a Cypress product as a Critical Component in a High-Risk Device. Cypress products are not intended or authorized for use as a Critical Component in any High-Risk Device except to the limited extent that (i) Cypress’s published data sheet for the product explicitly states Cypress has qualified the product for use in a specific High-Risk Device, or (ii) Cypress has given you advance written authorization to use the product as a Critical Component in the specific High-Risk Device and you have signed a separate indemnification agreement.
<br>
Cypress, the Cypress logo, and combinations thereof, WICED, ModusToolbox, PSoC, CapSense, EZ-USB, F-RAM, and Traveo are trademarks or registered trademarks of Cypress or a subsidiary of Cypress in the United States or in other countries. For a more complete list of Cypress trademarks, visit www.infineon.com. Other names and brands may be claimed as property of their respective owners.
