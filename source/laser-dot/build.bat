@echo off
rem Builds dinput8.dll (the 3D laser dot). /Brepro: the same SHA256 every time.
rem "build.bat log": a test build with the log and Ctrl+O measuring (%TEMP%\uzdoom_laserdot.log).
set LOGFLAG=
if /i "%~1"=="log" set LOGFLAG=/DLASERDOT_LOG=1
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"
cl /nologo /O2 /MT /EHsc /W4 /Brepro %LOGFLAG% /LD laser_dot.cpp /Fe:dinput8.dll /link /NOLOGO /Brepro /DEF:dinput8.def shell32.lib user32.lib || exit /b 1
