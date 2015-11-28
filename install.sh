#!/bin/sh

set -e # exit if error

which -s brew  || ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
which -s boost || brew install boost
which -s cmake || brew install cmake
which -s git   || brew install git
which -s hg    || brew install hg
which -s p7zip || brew install p7zip
which -s scons || brew install scons
which -s wget  || brew install wget

BUILDBIN="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -f $BUILDBIN/tools/cmake/bin/cmake ]; then
	echo CMake is already installed, skipping
else
	echo Downloading CMake...
	curl -o $BUILDBIN/tools/cmake.tar.gz -L https://cmake.org/files/v3.3/cmake-3.3.2-Darwin-x86_64.tar.gz

	echo Unpacking CMake...
	mkdir $BUILDBIN/tools/cmake
	bsdtar -xf $BUILDBIN/tools/cmake.tar.gz --strip=1 -C $BUILDBIN/tools/cmake
	rm $BUILDBIN/tools/cmake.tar.gz
	rm $BUILDBIN/tools/cmake/share/cmake-3.3/Modules/FindZLIB.cmake
	rm $BUILDBIN/tools/cmake/share/cmake-3.3/Modules/FindFreetype.cmake
	rm $BUILDBIN/tools/cmake/share/cmake-3.3/Modules/FindThreads.cmake
fi
