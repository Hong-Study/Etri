@echo off
setlocal enabledelayedexpansion
rem �ý���ȯ�溯�� �����ϰ� ��� �����ϱ�

set "PythonPath=C:\Python38\"
set "PythonScriptPath=C:\Python38\Scripts\"
set "NewPath=%PythonScriptPath%;%PythonPath%"
set "Check=1"

rem ȯ�� ���� ���� ;�� �и��Ͽ� �� ���� �ϳ��� ���
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