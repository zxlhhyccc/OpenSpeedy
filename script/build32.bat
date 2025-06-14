@echo off
echo =================================
echo Build 32 bit Qt5.15 static library
echo =================================

if "%VCINSTALLDIR%"=="" (
    echo Initializing Visual Studio 2022 32-bit environment...
    call %VC_TOOLS_32BIT%
) else (
    echo Visual Studio environment already initialized
    echo VCINSTALLDIR: %VCINSTALLDIR%
)

set SCRIPT_DIR=%~dp0
set SOURCE_DIR=%SCRIPT_DIR%..\
set BUILD_DIR=%BUILD_DIR_32BIT%
set QT_QMAKE_EXECUTABLE=%QT_QMAKE_EXECUTABLE_32BIT%

REM 将命令执行结果存储在变量中
for /f "delims=" %%i in ('"%QT_QMAKE_EXECUTABLE%" -query QT_INSTALL_PREFIX') do set QT_INSTALL_PREFIX=%%i
echo "%QT_QMAKE_EXECUTABLE%"
echo "%QT_INSTALL_PREFIX%"

cmake.exe ^
-DQT_QMAKE_EXECUTABLE:FILEPATH="%QT_QMAKE_EXECUTABLE%" ^
-DCMAKE_PREFIX_PATH:PATH="%QT_INSTALL_PREFIX%" ^
-DCMAKE_BUILD_TYPE=Release ^
-S %SOURCE_DIR% ^
-B %BUILD_DIR%

cmake.exe --build "%BUILD_DIR%" --config Release
