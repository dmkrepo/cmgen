
@set PATH=%CMGEN_INSTALL_ALL_DIR%\qt\bin;%CMGEN_JOM_DIR%;%PATH%

call %CMGEN_EXT_DIR%\compiler_vars.cmd

@echo JOM...
jom
@if not '%ERRORLEVEL%'=='0' exit /B 1
jom install
@if not '%ERRORLEVEL%'=='0' exit /B 1
