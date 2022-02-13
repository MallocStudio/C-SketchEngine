@ECHO OFF
REM https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160

REM build sketchengine and testbed
PUSHD ..\sketchengine
CALL build_lib.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "---------------building testbed---------------"
cl /EHsc src\*.c /I "../dep/include/" /I "../sketchengine/src/" sketchengine.lib OpenGL32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib freetype_debug.lib glew32.lib /MT /Zi /link /OUT:bin\testbed.exe /LIBPATH:"../dep/lib"
del *.obj
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
ECHO "testbed built successfully"