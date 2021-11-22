# OpenIPMC-FW

OpenIPMC-FW is a reference firmware for the OpenIPMC-HW mini-DIMM IPMC mezzanine, and has been developed as a project in STM32CubeIDE. The firmware includes by default a port of OpenIPMC, enabling the firmware to perform IPMI management for the ATCA board.

We want to underline that OpenIPMC-FW is a *reference* firmware, with no knowledge of the ATCA board it will be used for. It needs to be adapted for the specific use case of each ATCA board. For example, the user should implement the functions for the readout of the specific current, voltage and temperature sensors installed on the board.

## Documentation

For more details and developments instructions, you can find the documentation pages at [openipmc.gitlab.io/openipmc-fw/](https://openipmc.gitlab.io/openipmc-fw/)

## Licensing

OpenIPMC-FW is Copyright 2020-2021 of Andre Cascadan, Luigi Calligaris. OpenIPMC-FW is released under GNU General Public License version 3. Please refer to the LICENSE document included in this repository.
