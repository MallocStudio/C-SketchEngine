@ECHO off
PUSHD .\bin
    for /r %%a in (.) do (
        PUSHD %%a
            echo deleting assets in:
            cd
            DEL *.assets
            DEL *.level
            DEL *.mesh
        POPD
    )
    @REM DEL /s *.assets
    @REM DEL /s *.level
    @REM DEL /s *.mesh
POPD