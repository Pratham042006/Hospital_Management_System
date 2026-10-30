@echo off
REM Cross-platform build and test script for Windows (MSVC / Visual Studio)
REM Author: Prathik Nambiar (PES2UG24CS371)

echo ==============================================
echo  Hospital Management System - Windows Build Script
echo  Target: Windows (MSVC)
echo ==============================================

set BUILD_DIR=build

echo ==^> Configuring CMake for MSVC...
cmake -S . -B %BUILD_DIR%

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    exit /b %ERRORLEVEL%
)

echo ==^> Compiling Release build...
cmake --build %BUILD_DIR% --config Release

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed.
    exit /b %ERRORLEVEL%
)

echo ==^> Running test suite...
ctest --test-dir %BUILD_DIR% -C Release --output-on-failure

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] One or more tests failed.
    exit /b %ERRORLEVEL%
)

echo ==============================================
echo  Build and tests completed successfully!
echo  Run executable: %BUILD_DIR%\Release\hms.exe
echo ==============================================
