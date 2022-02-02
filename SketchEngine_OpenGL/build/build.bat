color
cls
cl /EHsc ..\src\*.c /I "../dep/include/" OpenGL32.lib SDL2.lib SDL2main.lib glew32.lib /MT /Zi /link /OUT:SketchEngine_OpenGL.exe /LIBPATH:"../dep/lib"
del *.obj