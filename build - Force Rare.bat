@echo off
REM ── Build GuaranteedBossDrops.dll ──
REM
REM Usage:
REM   build.bat            (build normally)
REM   build.bat deploy     (build + copy to game mods folder)
REM
REM Deploy assumes MEWGENICS_DIR is set, e.g.:
REM   set MEWGENICS_DIR=C:\Program Files (x86)\Steam\steamapps\common\Mewgenics

setlocal

REM ── Locate Visual Studio via vswhere ──
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found. Is Visual Studio installed?
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (
    echo ERROR: Could not find a Visual Studio installation.
    pause
    exit /b 1
)

if not exist "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" (
    echo ERROR: vcvarsall.bat not found at "%VSDIR%\VC\Auxiliary\Build\"
    pause
    exit /b 1
)

echo Setting up x64 MSVC environment...
call "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1

REM ── Build ──────────────────────────────────────────────────────
echo.
echo Building dllmain.dll...
cl /LD /O2 /GS- /W3 /D_CRT_SECURE_NO_WARNINGS /DFORCE_RARE GuaranteedBossDrops.c /Fe:GuaranteedBossDrops.dll

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo Build succeeded: GuaranteedBossDrops.dll

REM Clean intermediate files
del /Q GuaranteedBossDrops.obj GuaranteedBossDrops.lib GuaranteedBossDrops.exp 2>nul

REM ── Deploy (optional) ──────────────────────────────────────────
if /I "%1"=="deploy" (
    if not defined MEWGENICS_DIR (
        echo.
        echo WARNING: MEWGENICS_DIR not set. Cannot deploy.
        echo Set it to your Mewgenics install directory, e.g.:
        echo   set MEWGENICS_DIR=C:\Program Files ^(x86^)\Steam\steamapps\common\Mewgenics
        pause
        exit /b 1
    )
    if not exist "%MEWGENICS_DIR%\mods" (
        mkdir "%MEWGENICS_DIR%\mods"
    )
    copy /Y ExposeCatData.dll "%MEWGENICS_DIR%\mods\GuaranteedBossDrops.dll"
    echo Deployed to %MEWGENICS_DIR%\mods\GuaranteedBossDrops.dll
)

pause
