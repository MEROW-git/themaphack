@echo off
setlocal
if not defined VSCMD set "VSCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
if not exist "%VSCMD%" (
    echo Visual Studio C++ Build Tools not found. Set VSCMD to VsDevCmd.bat. 1>&2
    exit /b 1
)
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
call "%VSCMD%" -no_logo
if errorlevel 1 exit /b 1
if not exist artifacts\tests mkdir artifacts\tests
cl /nologo /std:c++17 /EHsc /W4 /Fo:artifacts\tests\startup_forwarder_test.obj /Fe:artifacts\tests\startup_forwarder_test.exe tests\startup_forwarder_test.cpp
if errorlevel 1 exit /b 1
artifacts\tests\startup_forwarder_test.exe
if errorlevel 1 exit /b 1
echo PASS: startup forwarding tests
