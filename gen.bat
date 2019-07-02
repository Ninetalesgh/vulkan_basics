@echo off

if [%1] == [] goto NoPlatform


set CMAKE="C:\Program Files\CMake\bin\cmake.exe"
set GENERATOR=%1
set PLATFORM=%2
set SYSTEM_TYPE=%3
set SOURCE_DIR=%cd%\code
set BUILD_DIR=%SOURCE_DIR%\..\build
set STARTUP_PROJECT=%4

echo cmake executable: %CMAKE%
echo generator: %GENERATOR%
echo platform: "%PLATFORM% %SYSTEM_TYPE%"
echo source directory: %SOURCE_DIR%
echo build directory: %BUILD_DIR%\%PLATFORM%
echo startup project: %STARTUP_PROJECT%

set CMAKE_CACHE=%BUILD_DIR%\CMakeCache.txt
set CPACK_CACHE=%SOURCE_DIR%\CPackConfig.cmake
set CTEST_CACHE=%SOURCE_DIR%\CTestConfig.cmake
if EXIST %CMAKE_CACHE% del %CMAKE_CACHE% 
if EXIST %CPACK_CACHE% del %CPACK_CACHE%
if EXIST %CTEST_CACHE% del %CTEST_CACHE%

if NOT EXIST %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

if NOT EXIST %PLATFORM% mkdir %PLATFORM%
cd %PLATFORM%

%CMAKE% -G %GENERATOR% -A %SYSTEM_TYPE% %SOURCE_DIR%^
 -DBREWING_STATION_TARGET_PLATFORM=%PLATFORM%^
 -DBREWING_STATION_STARTUP_PROJECT=%STARTUP_PROJECT%


goto error
goto success

:NoPlatform
echo PLEASE RUN cmake_android or cmake_windows NOT gen

:error
pause
:success
