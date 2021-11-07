#!/bin/bash

# Usage: ./build.sh [-debug/release/clean]
#
# Creates a folder at /$(pwd)/[debug/release]/

if [[ $@ == *clean* ]]; then
    echo "Cleaning..."
    rm -rf debug
    rm -rf release
    echo "Cleaned"
    exit 0
fi

rootdir=$(pwd)
BUILD_DIR=$rootdir/debug
BUILD_TYPE=Debug

if [[ $@ == *debug* ]]; then
    BUILD_DIR=$(pwd)/debug
    BUILD_TYPE=Debug
fi

if [[ $@ == *release* ]]; then
    BUILD_DIR=$(pwd)/release
    BUILD_TYPE=Release
fi

echo "Building..."
mkdir -p $BUILD_DIR
cd $BUILD_DIR
# Find appropriate cmake command based on OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux 
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    make
elif [[ "$OSTYPE" == "msys" ]]; then
    # MinGW for Windows
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    MSBuild.exe gestalt-server.sln
fi

if [ $? -ne 0 ]; then
    echo "Make library failed."
    cd $rootdir
    exit 1
fi
cd $rootdir