
PATH=$CMGEN_INSTALL_ALL_DIR/qt/bin:$PATH

set -e # exit if error

$CMGEN_EXT_DIR/compiler_vars.sh

echo QMake...
qmake $CMGEN_QMAKEFILE $CMGEN_OPTIONS
