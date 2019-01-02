#!/bin/bash -e

WEAVEDIR=`pwd`
WORKDIR=$HOME/prereq
INSTALL_DIR=$HOME/local

BUILDTYPE=release
export CC=gcc-8
export CXX=g++-8

export LIBRARY_PATH=/usr/lib:/usr/local/lib:$INSTALL_DIR/lib:$INSTALL_DIR/lib/x86_64-linux-gnu
export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:$INSTALL_DIR/lib:$INSTALL_DIR/lib/x86_64-linux-gnu

ssh-keyscan github.com >> ~/.ssh/known_hosts

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

cd $WORKDIR
if clone-repo "libpypa" "https://github.com/vinzenz/libpypa.git"; then
    cd libpypa
    echo "Building pypa"
    ./autogen.sh
    ./configure --prefix=$INSTALL_DIR
    make -j10
    make install
fi

cd $WORKDIR
if clone-repo "pybind11" "https://github.com/pybind/pybind11.git"; then
    cd pybind11
    echo "Building pybind11"
    git checkout v2.2
    cmake -DPYBIND11_TEST=OFF -DPYBIND11_PYTHON_VERSION=3.5 -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR
    make
    make install
fi
