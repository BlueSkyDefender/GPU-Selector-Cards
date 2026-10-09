@echo off
rem Builds dinput8.dll (the 3D laser dot). /Brepro: the same SHA256 every time.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"
cl /nologo /O2 /MT /EHsc /W4 /Brepro /LD laser_dot.cpp /Fe:dinput8.dll /link /NOLOGO /Brepro /DEF:dinput8.def shell32.lib user32.lib || exit /b 1
