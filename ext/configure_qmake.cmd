
@set PATH=%CMGEN_BIN_ALL_DIR%\qt\bin;%CMGEN_JOM_DIR%;%PATH%

call %CMGEN_EXT_DIR%\compiler_vars.cmd

echo QMake...
qmake %CMGEN_QMAKEFILE% %CMGEN_OPTIONS%
