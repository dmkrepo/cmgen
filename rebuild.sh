#!/bin/sh

set -e # exit if error

BUILDBIN="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo Building CMGen...
cd $BUILDBIN/cmgen-build
$BUILDBIN/tools/cmake/CMake.app/Contents/bin/cmake -GXcode $BUILDBIN/source
$BUILDBIN/tools/cmake/CMake.app/Contents/bin/cmake --build . --config Release