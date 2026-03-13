@echo off
if /i "%1"=="release" (
    if exist "out\build\x64-release\Debug\lillaspel.exe" (
        cd "out\build\x64-debug\Debug\"
        call "lillaspel.exe"
    ) else (
        echo executable file didn't exist, build first
    )
) else (
    if exist "out\build\x64-debug\Debug\lillaspel.exe" (
        cd "out\build\x64-debug\Debug\"
        call "lillaspel.exe"
    ) else (
        echo executable file didn't exist, build first
    )
)
