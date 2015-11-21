
@if "%CMGEN_PLATFORM%"=="msvc" (
	set _MSVC_VCVARS=x86
	if "%CMGEN_ARCH_BITNESS%"=="64" (
		set _MSVC_VCVARS=x64
	)
	call "%CMGEN_MSVC_DIR%\vcvarsall.bat" x86	
)

@if NOT '%ERRORLEVEL%'=='0' exit /b 1
