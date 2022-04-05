REM https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160
REM https://gist.github.com/jsancheznet/23b1fc8c592455f1bb84
REM OpenGL32.lib SDL2.lib SDL2main.lib glew32.lib /link /LIBPATH:"../dep/lib"
@ECHO OFF
ECHO "---------------building sketchengine lib---------------"
cl /c /EHsc src\*.c /I "../dep/include/" /MT /Z7 /WL /link
if %ERRORLEVEL% NEQ 0 goto some_error_happened_sketchengine

lib -nologo /OUT:..\dep\lib\sketchengine.lib *.obj

MOVE /Y vc140.pdb ..\dep\lib\
del *.obj
goto success

:some_error_happened_sketchengine
echo sketchengine build issues baby!
goto end

:success
echo sketch engine built successfully
goto end

:end