@echo off
REM Build the resource file
windres resource.rc -o resource.o

REM Compile the main C file
gcc App_Usage_Tracker.c resource.o -o AppUsageTracker.exe 

REM Check if the compilation was successful
if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed.
    exit /b %ERRORLEVEL%
)

echo Build successful.
