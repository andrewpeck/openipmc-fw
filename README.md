# OpenIPMC-FW

OpenIPMC-FW is a reference firmware designed for OpenIPMC-HW mini-DIMM card, and was developed on STM32CubeIDE.

OpenIPMC-FW includes a port of OpenIPMC to perform IPMI messaging for ATCA boards.

## STM32CubeIDE Workspace Setup

The following steps instructs how to add this project into the  **STM32CubeIDE 1.5.0** workspace.

1. Clone the project into the chosen workspace directory:

        $ git clone git@gitlab.com:openipmc/openipmc-fw.git
        
2. Clone *submodules*:

        $ cd openipmc-fw
        $ git submodule update --init --recursive

3. In STM32CubeIDE, go to *File* &rarr; *Import*.

4. In *Import* menu, go to *General* &rarr; *Existing Projects into Workspace*

5. In *Select root directory* box, enter (or browse) the workspace directory path. 

    **NOTE:** Do not enter the project directory path here, but the directory which contains `openipmc-fw`.
    
6. In *Projects* box, mark the *openipmc-fw* and the two respective nested projects for CM4 and CM7 cores:

    - openipmc-fw
    - openipmc-fw_CM4
    - openipmc-fw_CM7
    
7. Press *Finish*. When import is concluded, you should see in the *Project Explorer* a structure like this:

    ![ProjectExplorer](images/project_explorer.png)

## Compiling and Debugging

A basic debugging configuration for Cortex-M7 is presented bellow. For better information about debugging refers to the Application Note [AN5361](https://www.st.com/resource/en/application_note/dm00629855-getting-started-with-projects-based-on-dualcore-stm32h7-microcontrollers-in-stm32cubeide-stmicroelectronics.pdf).



1. In *Project Explorer* tab, Right click on *openipmc-fw_CM4*. Click in *Build Project*.

2. Repeat for *openipmc-fw_CM7*.

3. In *Project Explorer* tab, Right click on *openipmc-fw_CM7*. Click *Debug As* &rarr; *1 STM32 Cortex-M C/C++ Application*.

4. Despite we are going to debug only the CM7 core, the CM4 firmware (which has no practical purpose in this project so far) also need to be loaded. In tab *Startup*, click *Add...* and configure as below:

    ![DebugAddCM4](images/debug_add_cm4.png)

    ![DebugStartup](images/debug_startup.png)

5. In tab *Debugger* mark *ST-LINK S/N* and click *Scan*. If IDE recognizes a ST-LINK connected, the serial number will appear in the field.

    ![DebugDebugger](images/debug_debugger.png)

6. Click *Ok* to start debugging. Debugger will halt at the beginning of the `main()` function. Click ![ResumeArrow](images/ResumeArrow.png) (Resume) to start.

## Licensing

OpenIPMC-FW is Copyright 2020-2021 of Andre Cascadan, Luigi Calligaris. OpenIPMC-FW is released under GNU General Public License version 3. Please refer to the LICENSE document included in this repository.
