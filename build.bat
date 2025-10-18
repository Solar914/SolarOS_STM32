@echo off
REM SolarOS STM32 Build Script - Ultra Simplified (under 50 lines)

echo ======== SolarOS STM32 Build ========

REM Find uvConvertor
set UV_PATH=
REM Try PATH first
where uvConvertor-CLI.exe >nul 2>&1 && set UV_PATH=uvConvertor-CLI.exe
REM Try environment variable
if "%UV_PATH%"=="" if defined UVCONVERTOR_PATH set UV_PATH=%UVCONVERTOR_PATH%
REM Try common folder names in common drive locations
if "%UV_PATH%"=="" for %%R in (C D E F) do for %%D in (tools uvConvertor) do if exist "%%R:\%%D\uvConvertor-CLI.exe" set UV_PATH=%%R:\%%D\uvConvertor-CLI.exe
if "%UV_PATH%"=="" (echo ERROR: uvConvertor not found. Set UVCONVERTOR_PATH or add to PATH && exit /b 1)

REM Find Keil (optional)
set KEIL_PATH=
REM Try environment variable first
if defined KEIL_PATH set KEIL_PATH=%KEIL_PATH%
REM Try common locations
if "%KEIL_PATH%"=="" for %%R in (C D E F) do for %%V in (Keil_v5 Keil) do if exist "%%R:\%%V\UV4\UV4.exe" set KEIL_PATH=%%R:\%%V\UV4\UV4.exe

REM Create build directory
if not exist build mkdir build

echo Using: %UV_PATH%

REM Compile with Keil if available
if not "%KEIL_PATH%"=="" (
    echo [1/2] Keil compilation...
    "%KEIL_PATH%" -b soalrOS.uvprojx -j0 >nul
    echo Done
)

REM Generate compile_commands.json
echo [2/2] Generating compile_commands.json...
"%UV_PATH%" --file soalrOS.uvprojx --output build

REM Fix armcc parameters for clangd
powershell -Command "$j=Get-Content build\compile_commands.json|ConvertFrom-Json;$j|%%{$_.arguments=$_.arguments|?{$_ -ne '--li' -and $_ -notlike '--omf_browse*'}|%%{$_ -replace '^-Id:','-I'}};$j|ConvertTo-Json -Depth 10|Set-Content build\compile_commands.json -Encoding UTF8" >nul

if exist build\compile_commands.json (
    for %%A in (build\compile_commands.json) do echo SUCCESS: %%~zA bytes
    echo.
    echo Files generated:
    echo - build\compile_commands.json ^(clangd database^)
    for %%f in (Objects\*.axf) do for %%A in ("%%f") do echo - %%f ^(%%~zA bytes^)
    echo.
    echo Restart VS Code to enable navigation
) else (
    echo ERROR: Failed to generate compile_commands.json
    exit /b 1
)