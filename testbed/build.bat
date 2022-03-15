@ECHO OFF
REM https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160

REM build sketchengine and testbed
PUSHD ..\sketchengine
CALL build_lib.bat
POPD
if %ERRORLEVEL% NEQ 0 goto some_error_happened_sketchengine

ECHO "---------------building testbed---------------"
cl /EHsc src\*.c /I "../dep/include/" /I "../sketchengine/src/" sketchengine.lib OpenGL32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib freetype_debug.lib glew32.lib /MT /Zi /WL /link /OUT:bin\testbed.exe /LIBPATH:"../dep/lib" /NODEFAULTLIB:library
del *.obj
if %ERRORLEVEL% NEQ 0 goto some_error_happened_testbed
goto success

:some_error_happened_sketchengine
REM build_lib.bat already prints out an error
goto end

:some_error_happened_testbed
echo testbed build issues baby!
goto end

:success
echo testbed built successfully
goto end

:end