ECHO "---------------building game---------------"

cl /EHsc src\*.c /I "../dep/include/" /I "../sketchengine/src/" /MT /Z7 /WL /link /OUT:bin\game.exe /LIBPATH:"../dep/lib" sketchengine.lib SDL2.lib OpenGL32.lib SDL2main.lib SDL2_ttf.lib freetype_debug.lib glew32.lib assimp-vc143-mtd.lib

REM (assimp-vc143-mtd.lib zlibstaticd.lib)

del *.obj

if %ERRORLEVEL% NEQ 0 goto some_error_happened
goto success

:some_error_happened
echo game build issues baby!
goto end

:success
echo game built successfully
goto end

:end