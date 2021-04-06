#! /bin/bash

SHA1=`git rev-parse --short=8 HEAD`

echo "#define HEAD_COMMIT_SHA1 0x"$SHA1 > ../Core/Inc/head_commit_sha1.h
