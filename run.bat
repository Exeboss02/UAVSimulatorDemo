@echo off
if exist "out\build\x64-debug\Debug\lillaspel.exe" (
    cd "out\build\x64-debug\Debug\"
    call "lillaspel.exe"
) else (
    echo executable file didn't exist, build first
)