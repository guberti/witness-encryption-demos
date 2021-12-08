#!/usr/bin/env bash

#abort if any command fails
set -e

builddir=$(readlink -f build)
if [[ "$1" == "debug" ]]; then
    debug="-DCMAKE_BUILD_TYPE=Debug"
elif [[ "$1" == "clean" ]]; then
    rm -rf CMakeCache CMakeFiles/ libaesrand
    exit 0
else
    debug=""
fi

export CPPFLAGS=-I$builddir/include
export CFLAGS=-I$builddir/include
export LDFLAGS=-L$builddir/lib

build () {
    echo building $1
    path=$1
    url=$2
    branch=$3
    if [ ! -d $path ]; then
        git clone $url $path -b $branch;
    else
        pushd $path; git pull origin $branch; popd
    fi
    pushd $1
    cmake -DCMAKE_INSTALL_PREFIX="${builddir}" .
    make
    make install
    popd
    echo
}

echo
echo builddir = $builddir
echo

build libaesrand    https://github.com/5GenCrypto/libaesrand master

cmake "${debug}" .
make
