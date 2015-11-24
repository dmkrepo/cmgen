
if "%CMGEN_PLATFORM%"=="msvc" (
	if "%CMGEN_ARCH_BITNESS%"=="64" (
		call "%CMGEN_MSVC_DIR%\vcvarsall.bat" amd64
	) else (
		call "%CMGEN_MSVC_DIR%\vcvarsall.bat" x86
	)
)

@if NOT '%ERRORLEVEL%'=='0' exit /b 1
