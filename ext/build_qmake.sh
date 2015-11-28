
PATH=$CMGEN_INSTALL_ALL_DIR/qt/bin:$PATH

set -e # exit if error

$CMGEN_EXT_DIR/compiler_vars.sh

echo Make...
make
make install
