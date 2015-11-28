::
:: CMGen
:: Copyright (C) 2015  Dmitriy Ka
::
:: This program is free software; you can redistribute it and/or
:: modify it under the terms of the GNU General Public License
:: as published by the Free Software Foundation; either version 2
:: of the License, or (at your option) any later version.
::
:: This program is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
:: GNU General Public License for more details.
::
:: You should have received a copy of the GNU General Public License
:: along with this program; if not, write to the Free Software
:: Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
::

setlocal enableextensions enabledelayedexpansion&set BUILDBIN=%~dp0&&set BUILDBIN=!BUILDBIN:~0,-1!

cd /d %BUILDBIN%\tools

@echo on

:: MSYS

@if EXIST %BUILDBIN%\tools\msys\usr\bin\bash.exe (
	echo MSYS is already installed, skipping
	goto SKIP_MSYS
)

@echo Downloading MSYS...
cscript %BUILDBIN%\httpget.js http://downloads.sourceforge.net/project/msys2/Base/x86_64/msys2-x86_64-20150916.exe msys2.exe
::if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Installing MSYS...
set TARGET_DIR=%BUILDBIN%\tools\msys
%BUILDBIN%\tools\msys2.exe --script %BUILDBIN%\qtscript.js
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\msys2.exe
cd /d %BUILDBIN%\tools\msys\usr\bin
@echo Installing MSYS packages...
pacman -Sy --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
pacman -S curl --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
pacman -S wget --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
pacman -S patch --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
pacman -S make --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
pacman -S nasm --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
pacman -S yasm --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
pacman -S unzip --noconfirm
if NOT '%ERRORLEVEL%'=='0' goto ERROR
ren link.exe msyslink.exe
if NOT '%ERRORLEVEL%'=='0' goto ERROR
cd /d %BUILDBIN%\tools
:SKIP_MSYS

:: CMAKE

@if EXIST %BUILDBIN%\tools\cmake\bin\cmake.exe (
	echo CMake is already installed, skipping
	goto SKIP_CMAKE
)

@echo Downloading CMake...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\cmake.zip -L https://cmake.org/files/v3.3/cmake-3.3.2-win32-x86.zip
if NOT '%ERRORLEVEL%'=='0' goto ERROR

@echo Unpacking CMake...
mkdir %BUILDBIN%\tools\cmake
%BUILDBIN%\tools\msys\usr\bin\bsdtar.exe -xf %BUILDBIN%\tools\cmake.zip --strip=1 -C %BUILDBIN%\tools\cmake
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\cmake.zip
del /Q %BUILDBIN%\tools\cmake\share\cmake-3.3\Modules\FindZLIB.cmake
del /Q %BUILDBIN%\tools\cmake\share\cmake-3.3\Modules\FindFreetype.cmake
del /Q %BUILDBIN%\tools\cmake\share\cmake-3.3\Modules\FindThreads.cmake

:SKIP_CMAKE

:: PERL

@if EXIST %BUILDBIN%\tools\perl\perl\bin\perl.exe (
	echo Perl is already installed, skipping
	goto SKIP_PERL
)

@echo Downloading Perl...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\perl.zip -L http://strawberryperl.com/download/5.20.3.1/strawberry-perl-5.20.3.1-32bit-portable.zip
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Unpacking Perl...
mkdir %BUILDBIN%\tools\perl
%BUILDBIN%\tools\msys\usr\bin\bsdtar.exe -xf %BUILDBIN%\tools\perl.zip -C %BUILDBIN%\tools\perl
:: msiexec /qb /L*v perl-log.txt /i perl.msi ADDLOCAL="PERL_FEATURE" PERL_PATH=No PERL_EXT=No INSTALLDIR=%BUILDBIN%\tools\perl
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\perl.zip
:SKIP_PERL

:: HG

@if EXIST %BUILDBIN%\tools\Hg\hg.exe (
	echo Hg is already installed, skipping
	goto SKIP_HG
)

@echo Downloading Hg...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\tortoisehg.msi -L http://bitbucket.org/tortoisehg/files/downloads/tortoisehg-2.7.1-hg-2.5.2-x86.msi
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Unpacking Hg...
mkdir %BUILDBIN%\tools\hgtmp
mkdir %BUILDBIN%\tools\hg
msiexec /a tortoisehg.msi /qb TARGETDIR=%BUILDBIN%\tools\hgtmp
if NOT '%ERRORLEVEL%'=='0' goto ERROR
mkdir hg
xcopy %BUILDBIN%\tools\hgtmp\pfiles\tortoisehg\* %BUILDBIN%\tools\hg /s /i
rmdir /S /Q %BUILDBIN%\tools\hgtmp
del /Q %BUILDBIN%\tools\tortoisehg.msi

:SKIP_HG

:: GIT

@if EXIST %BUILDBIN%\tools\PortableGit\bin\git.exe (
	echo Git is already installed, skipping
	goto SKIP_GIT
)

@echo Downloading Git...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\git.exe -L https://github.com/git-for-windows/git/releases/download/v2.6.2.windows.1/PortableGit-2.6.2-64-bit.7z.exe
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Unpacking Git...
cd /d %BUILDBIN%\tools
%BUILDBIN%\tools\git.exe -y -dm1
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\git.exe
:SKIP_GIT

:: PYTHON

@if EXIST %BUILDBIN%\tools\python\python.exe (
	echo Python is already installed, skipping
	goto SKIP_PYTHON
)

@echo Downloading Python...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\python.msi -L http://downloads.activestate.com/ActivePython/releases/2.7.10.12/ActivePython-2.7.10.12-win64-x64.msi
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Installing Python...
mkdir %BUILDBIN%\tools\python
::Full install: msiexec /qb /L*v python-log.txt /i python.msi ADDLOCAL="core,pywin32" INSTALLDIR=%BUILDBIN%\tools\python
msiexec /a python.msi /qb TARGETDIR=%BUILDBIN%\tools\pytmp
if NOT '%ERRORLEVEL%'=='0' goto ERROR
mkdir python
xcopy %BUILDBIN%\tools\pytmp\WINVOL\Python27\* %BUILDBIN%\tools\python /s /i
del /Q %BUILDBIN%\tools\python.msi
rmdir /S /Q %BUILDBIN%\tools\pytmp
:SKIP_PYTHON

:: SCONS

@if EXIST %BUILDBIN%\tools\python\scons.bat (
	echo SCons is already installed, skipping
	goto SKIP_SCONS
)

@echo Downloading SCons...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\scons.zip -L http://prdownloads.sourceforge.net/scons/scons-2.4.0.zip
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Unpacking SCons...
mkdir %BUILDBIN%\tools\scons
%BUILDBIN%\tools\msys\usr\bin\bsdtar.exe -xf %BUILDBIN%\tools\scons.zip --strip=1 -C %BUILDBIN%\tools\scons
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Installing SCons...
%BUILDBIN%\tools\python\python %BUILDBIN%\tools\scons\setup.py install
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\scons.zip
:SKIP_SCONS

:: JOM

@if EXIST %BUILDBIN%\tools\jom\jom.exe (
	echo JOM is already installed, skipping
	goto SKIP_JOM
)

@echo Downloading jom...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\jom.zip -L http://download.qt.io/official_releases/jom/jom_1_0_15.zip
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Unpacking jom...
mkdir %BUILDBIN%\tools\jom
%BUILDBIN%\tools\msys\usr\bin\bsdtar.exe -xf %BUILDBIN%\tools\jom.zip -C %BUILDBIN%\tools\jom
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\jom.zip
:SKIP_JOM

:: RUBY

@if EXIST %BUILDBIN%\tools\ruby\bin\ruby.exe (
	echo Ruby is already installed, skipping
	goto SKIP_RUBY
)

@echo Downloading Ruby...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\ruby.exe -L http://dl.bintray.com/oneclick/rubyinstaller/rubyinstaller-2.2.3-x64.exe
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Installing Ruby...
mkdir %BUILDBIN%\tools\ruby
%BUILDBIN%\tools\ruby.exe /SILENT /LOG /DIR=%BUILDBIN%\tools\ruby
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\ruby.exe
:SKIP_RUBY

:: 7ZIP

@if EXIST %BUILDBIN%\tools\7zip\7za.exe (
	echo 7zip is already installed, skipping
	goto SKIP_7ZIP
)

@echo Downloading 7zip...
%BUILDBIN%\tools\msys\usr\bin\curl.exe -o %BUILDBIN%\tools\7zip.zip -L http://www.7-zip.org/a/7za920.zip
if NOT '%ERRORLEVEL%'=='0' goto ERROR
@echo Unpacking 7zip...
mkdir %BUILDBIN%\tools\7zip
%BUILDBIN%\tools\msys\usr\bin\bsdtar.exe -xf %BUILDBIN%\tools\7zip.zip -C %BUILDBIN%\tools\7zip
if NOT '%ERRORLEVEL%'=='0' goto ERROR
del /Q %BUILDBIN%\tools\7zip.zip
:SKIP_7ZIP

:: CMGEN

@if EXIST %BUILDBIN%\cmgen\cmgen.exe (
	echo CMGen is already installed, skipping
	goto SKIP_CMGEN
)

@echo Configuring CMGen...
rmdir /S /Q %BUILDBIN%\cmgen-build
mkdir %BUILDBIN%\cmgen-build

cd /d %BUILDBIN%\cmgen-build
%BUILDBIN%\tools\cmake\bin\cmake.exe -G"Visual Studio 14 2015" %BUILDBIN%\source
if NOT '%ERRORLEVEL%'=='0' goto ERROR

@echo Building CMGen...
%BUILDBIN%\tools\cmake\bin\cmake.exe --build . --config Release
if NOT '%ERRORLEVEL%'=='0' goto ERROR

:SKIP_CMGEN

goto EXIT

:ERROR

@echo Error occured
exit /B 1

:EXIT