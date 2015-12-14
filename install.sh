#!/bin/sh

set -e # exit if error

which -s brew     || ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
which -s boost    || brew install boost
which -s cmake    || brew install cmake
which -s git      || brew install git
which -s hg       || brew install hg
which -s p7zip    || brew install p7zip
which -s scons    || brew install scons
which -s wget     || brew install wget
which -s autoconf || brew install autoconf
which -s automake || brew install automake
which -s libtool  || brew install libtool


BUILDBIN="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -f $BUILDBIN/tools/cmake/CMake.app/Contents/bin/cmake ]; then
	echo CMake is already installed, skipping
else
	echo Downloading CMake...
	curl -o $BUILDBIN/tools/cmake.tar.gz -L https://cmake.org/files/v3.4/cmake-3.4.1-Darwin-x86_64.tar.gz

	echo Unpacking CMake...
	mkdir $BUILDBIN/tools/cmake
	bsdtar -xf $BUILDBIN/tools/cmake.tar.gz --strip=1 -C $BUILDBIN/tools/cmake
	rm $BUILDBIN/tools/cmake.tar.gz
	#rm $BUILDBIN/tools/cmake/CMake.app/Contents/share/cmake-3.3/Modules/FindZLIB.cmake
	#rm $BUILDBIN/tools/cmake/CMake.app/Contents/share/cmake-3.3/Modules/FindFreetype.cmake
fi

if [ -f $BUILDBIN/cmgen/cmgen ]; then
    echo CMGen is already installed, skipping
else
    echo Building CMGen...
    rm -rf $BUILDBIN/cmgen-build
    mkdir $BUILDBIN/cmgen-build
    cd $BUILDBIN/cmgen-build
    $BUILDBIN/tools/cmake/CMake.app/Contents/bin/cmake -G Xcode $BUILDBIN/source
    $BUILDBIN/tools/cmake/CMake.app/Contents/bin/cmake --build . --config Release
fi
