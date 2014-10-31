#!/bin/sh -f

if ( test "`uname -s`" = "Darwin" )
then
    echo
else
    sudo add-apt-repository --yes ppa:kalakris/cmake
    sudo apt-get update -qq
    sudo apt-get install cmake
fi
