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
DEPLOY_DIR=$rootdir/../../apps/cor-ui/Assets/Plugins/
BUILD_DIR=$rootdir/debug
BUILD_TYPE=Debug
WIN_BUILD_ARGS="//t:Rebuild //p:Configuration=Debug"

if [[ $@ == *debug* ]]; then
    BUILD_DIR=$(pwd)/debug
    BUILD_TYPE=Debug
    BUILD_SUFFIX=_d
    WIN_BUILD_ARGS="//t:Rebuild //p:Configuration=Debug"
fi

if [[ $@ == *release* ]]; then
    BUILD_DIR=$(pwd)/release
    BUILD_TYPE=Release
    BUILD_SUFFIX=
    WIN_BUILD_ARGS="//t:Rebuild //p:Configuration=Release"
fi

echo "Building..."
mkdir -p $BUILD_DIR
cd $BUILD_DIR
# Find appropriate cmake command based on OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    make
    BUILD_RESULT=$BUILD_DIR/gestalt_server$BUILD_SUFFIX.so
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # Darwin
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    make
    BUILD_RESULT=$BUILD_DIR/libgestalt_server$BUILD_SUFFIX.dylib
elif [[ "$OSTYPE" == "msys" ]]; then
    # MinGW for Windows
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    echo $WIN_BUILD_ARGS
    echo $BUILD_TYPE
    MSBuild.exe gestalt-server.sln $WIN_BUILD_ARGS
    if [[ $@ == *debug* ]]; then
        BUILD_RESULT=$BUILD_DIR/Debug/gestalt_server$BUILD_SUFFIX.dll
    elif [[ $@ == *release* ]]; then
        BUILD_RESULT=$BUILD_DIR/Release/gestalt_server$BUILD_SUFFIX.dll
    fi
    
fi

if [ $? -ne 0 ]; then
    echo "Make library failed."
    cd $rootdir
    exit 1
fi
cd $rootdir

# Deploy the .dll/.so to cor-ui
if [[ $@ == *deploy* ]]; then
    if [ ! -d $DEPLOY_DIR ]; then
        mkdir -p $DEPLOY_DIR
    fi
    cp -v $BUILD_RESULT $DEPLOY_DIR
    if [ $? -ne 0 ]; then
        echo "Deploy failed"
        cd $rootdir
        exit 1
    fi
fi
