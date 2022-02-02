color
cls
cl /EHsc ../src/*.c /I "../lib/include/" SDL2_ttf.lib SDL2.lib SDL2main.lib /MT /Zi /link /out:SketchEngine2.exe /libpath:"../lib/lib/"
del *.obj