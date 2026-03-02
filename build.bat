@echo off

if /i "%1"=="release" (
    if exist "out\build\x64-release\Release\lillaspel.exe" (
        move "out\build\x64-release\Release\lillaspel.exe" "out\build\x64-release\Release\lillaspel_old.exe"
    )

    :: Make the preset for the project
    call cmake --preset x64-release || exit /b %errorlevel%

    :: Build new exe
    call cmake --build --preset x64-release || exit /b %errorlevel%
)

:: Rename old exe to ensure that if build fails, the old exe wont be run and misinterpreted as a new build
if exist "out\build\x64-debug\Debug\lillaspel.exe" (
    move "out\build\x64-debug\Debug\lillaspel.exe" "out\build\x64-debug\Debug\lillaspel_old.exe"
)

:: Make the preset for the project
call cmake --preset x64-debug || exit /b %errorlevel%

:: Build new exe
call cmake --build --preset x64-debug || exit /b %errorlevel%