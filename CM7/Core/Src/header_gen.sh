#! /bin/bash

# This script is used to generate needed header files just before running the compiler.
#
# The suggested way to execute this script automatically by the STM32CubeIDE is adding 
# "../Core/Src/header_gen.sh" in the "Pre-build steps" configuration.
#
#    - Open CM7 project Properties Menu 
#    - On the left tree, select "C/C++ Build" -> "Settings"
#    - In Build Steps tab, add "../Core/Src/header_gen.sh" in "Pre-build steps"->"Command" field



# Generates a header file with 8-long hash of the current commit (HEAD)

SHA1=`git rev-parse --short=8 HEAD`

if [ $? = 0 ]
then
    echo "#define HEAD_COMMIT_SHA1 0x"$SHA1    > ../Core/Inc/head_commit_sha1.h
else
    # If git command fails just define hash as "00000000"
    echo "#define HEAD_COMMIT_SHA1 0x00000000" > ../Core/Inc/head_commit_sha1.h
fi


# Generates a header file with compiling epoch

EPOCH=`date '+%s'`

if [ $? = 0 ]
then
    echo "#define COMPILING_EPOCH "$EPOCH    > ../Core/Inc/compiling_epoch.h
else
    # If git command fails just define as the IPMI starting time (00h00, January 1th, 1996)
    echo "#define COMPILING_EPOCH 820454400" > ../Core/Inc/compiling_epoch.h
fi
