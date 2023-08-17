@echo off
setlocal enabledelayedexpansion
rem 시스템환경변수 생성하고 경로 지정하기

set "PythonPath=C:\Python38\"
set "PythonScriptPath=C:\Python38\Scripts\"
set "NewPath=%PythonScriptPath%;%PythonPath%"
set "Check=1"

rem 환경 변수 값을 ;로 분리하여 각 값을 하나씩 출력
for %%i in ("!PATH:;=" "!") do (
    set "Item=%%~i"
    if "!Item!"=="!PythonPath!" (
        echo !PythonPath!
        set "Check=0"
    )
    if "!Item!"=="!PythonScriptPath!" (
        echo !PythonScriptPath!
        set "Check=0"
    ) 
    if !Check! equ 1 (
        set "NewPath=!NewPath!;!Item!"
    )
    set "Check=1"
)

echo !NewPath!


pause