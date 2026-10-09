@echo off
rem Builds GPU Selector's 3D fix for Doomsday 2.3.2 build 3869 (see stereo_projection.cpp).
rem %1 = output folder (default: build\ here), %2 = folder for the build's own files
rem /Brepro: the same source always makes the same file, so the card's SHA256 stays right.
setlocal
set OUT=%~1
set WORK=%~2
if "%OUT%"=="" set OUT=%~dp0build
if "%WORK%"=="" set WORK=%TEMP%\doomsday3d_build
if not exist "%OUT%" mkdir "%OUT%"
if not exist "%WORK%" mkdir "%WORK%"
where cl >nul 2>nul || call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
python "%~dp0make_forwarders.py" "%WORK%" || exit /b 1
lib /nologo /def:"%WORK%\original.def" /machine:x64 /out:"%WORK%\deng_appfw_doomsday.lib" || exit /b 1
cl /nologo /O2 /MD /EHsc /W4 /LD /Brepro /I"%WORK%" /Fo"%WORK%\\" "%~dp0stereo_projection.cpp" "%~dp0crosshair.cpp" "%WORK%\deng_appfw_doomsday.lib" user32.lib shell32.lib /Fe:"%OUT%\deng_appfw.dll" /link /NOLOGO /Brepro /IMPLIB:"%WORK%\deng_appfw.lib" || exit /b 1
del /q "%OUT%\deng_appfw.exp" "%OUT%\deng_appfw.lib" 2>nul
exit /b 0
