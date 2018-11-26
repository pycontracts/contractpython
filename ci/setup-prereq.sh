#!/bin/bash -e

WEAVEDIR=`pwd`
WORKDIR=$HOME/prereq
INSTALL_DIR=$HOME/local

BUILDTYPE=release
export CC=gcc-8
export CXX=g++-8

export LIBRARY_PATH=/usr/lib:/usr/local/lib:$INSTALL_DIR/lib
export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:$INSTALL_DIR/lib

function clone-repo() {
    dir=$1
    url=$2

    if [ -d $dir ]; then
        return 1
    else
        git clone $url $dir
        return 0
    fi
}

ssh-keyscan github.com >> ~/.ssh/known_hosts

cd $WORKDIR
if clone-repo "bitstream" "https://github.com/kaimast/bitstream.git"; then
    cd bitstream
    echo "Building bitstream"
    meson build -Dbuildtype=$BUILDTYPE --prefix=$INSTALL_DIR
    cd build
    ninja -v
    ninja install -v
fi

cd $WORKDIR
if clone-repo "libdocument" "https://github.com/kaimast/libdocument.git"; then
    cd libdocument
    echo "Building libdocument"
    meson build -Dbuildtype=$BUILDTYPE --prefix=$INSTALL_DIR
    cd build
    ninja -v
    ninja install -v
fi
