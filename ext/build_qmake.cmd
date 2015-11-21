
@set PATH=%CMGEN_BIN_ALL_DIR%\qt\bin;%CMGEN_JOM_DIR%;%PATH%

call %CMGEN_EXT_DIR%\compiler_vars.cmd

@echo JOM...
jom -j %CMGEN_BUILD_CPUS%
jom install
