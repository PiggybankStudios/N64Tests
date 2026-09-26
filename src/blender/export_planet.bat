@echo off
setlocal enabledelayedexpansion

REM TODO: Check if blender.exe is already in the PATH and use it if so?
REM TODO: Check if the blender.exe we are using is a supported version
REM TODO: Remove hardcoded path to my install of Blender. Maybe we could loop over all blender folders at the default install location and choose the newest one?
set BLENDER_PATH="C:\Program Files\Blender Foundation\Blender 3.6\blender.exe"

if "%~1"=="" goto :PrintUsage
set "IN_PATH=%~1"
if "%~2"=="" (
	set "OUT_PATH=%~dpn1.gltf"
) else (
	set "OUT_PATH=%~2"
)
if not "%~3"=="" goto :PrintUsage

REM NOTE: Enable me only for testing. This is dangerous to do in a real folder
REM call :DeleteOldFiles "%OUT_PATH%"

REM --background           // Run Blender in headless mode (no UI)
REM --factory-startup      // Ignore user preferences
REM --python-exit-code 1   // Makes script exceptions return a non-zero exit code
REM --python               // Run a python script in the Blender environment
%BLENDER_PATH% --background --factory-startup --python-exit-code 1 "%IN_PATH%" --python export_planet.py -- "%OUT_PATH%"
exit /b %ERRORLEVEL%

:PrintUsage
echo Usage: %~nx0 [path/to/model.blend] [path/to/output.gltf]
exit /b 1

:DeleteOldFiles
if not exist "%~dp1" (
	echo Output folder doesn't exist: %~dp1
	exit /b 1
)
echo Deleting %~dp1*.gltf^|png^|bin
del /q "%~dp1*.gltf"
del /q "%~dp1*.png"
del /q "%~dp1*.bin"
del /q "%~dp1*.json"
exit /b 0
