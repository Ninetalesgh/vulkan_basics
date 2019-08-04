@echo off

if [%1] == [] goto NoPlatform


set CMAKE="C:\Program Files\CMake\bin\cmake.exe"
set GENERATOR=%1
set PLATFORM=%2
set SYSTEM_TYPE=%3
set ROOT_DIR=%cd%
set SOURCE_DIR=%cd%\code
set BUILD_DIR=%cd%\build
set STARTUP_PROJECT=%4

echo CMake executable: %CMAKE%
echo Generator: %GENERATOR%
echo Platform: "%PLATFORM% %SYSTEM_TYPE%"
echo Source directory: %SOURCE_DIR%
echo Build directory: %BUILD_DIR%\%PLATFORM%
echo Startup project: %STARTUP_PROJECT%

set CMAKE_CACHE=%BUILD_DIR%\%PLATFORM%\CMakeCache.txt
set CPACK_CACHE=%SOURCE_DIR%\%PLATFORM%\CPackConfig.cmake
set CTEST_CACHE=%SOURCE_DIR%\%PLATFORM%\CTestConfig.cmake
::if EXIST %CMAKE_CACHE% del %CMAKE_CACHE% 
::if EXIST %CPACK_CACHE% del %CPACK_CACHE%
::if EXIST %CTEST_CACHE% del %CTEST_CACHE%

if NOT EXIST %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

if NOT EXIST %PLATFORM% mkdir %PLATFORM%
cd %PLATFORM%

echo =======================================================================================================================
echo =======================================================================================================================
echo ================================================ LAUNCHING CMAKE ======================================================
echo =======================================================================================================================
echo =======================================================================================================================

%CMAKE% -G %GENERATOR% -A %SYSTEM_TYPE% %SOURCE_DIR%^
 -DBREWING_STATION_TARGET_PLATFORM=%PLATFORM%^
 -DBREWING_STATION_STARTUP_PROJECT=%STARTUP_PROJECT%^
 -DBREWING_STATION_ROOT_DIR=%ROOT_DIR%


goto error
goto success

:NoPlatform
echo PLEASE RUN cmake_android or cmake_windows NOT gen

:error
pause
:success
