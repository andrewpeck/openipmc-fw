#! /bin/bash

# This script is used to generate needed header file just before running the compiler.
#
# The suggested way to execute this script automatically by the STM32CubeIDE is adding 
# "../Core/Src/header_gen.sh && " in the Command Line Pattern.
#
#    - Open CM7 project Properties Menu 
#    - On the left tree, select "C/C++ Build" -> "Settings"
#    - In Tool Settings tab, Select "MCU GCC Compiler"
#    - Edit the "Command line pattern" adding "../Core/Src/header_gen.sh && " in the beginning. Ex:
#
#              ../Core/Src/header_gen.sh && ${COMMAND} ${INPUTS} ${FLAGS} .... etc ....



# Generates a header file with 8-long hash of the current commit (HEAD)

SHA1=`git rev-parse --short=8 HEAD`

if [ $? = 0 ]
then
    echo "#define HEAD_COMMIT_SHA1 0x"$SHA1    > ../Core/Inc/head_commit_sha1.h
else
    # If git command fails just define hash as "00000000"
    echo "#define HEAD_COMMIT_SHA1 0x00000000" > ../Core/Inc/head_commit_sha1.h
fi
