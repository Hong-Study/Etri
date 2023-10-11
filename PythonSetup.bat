@echo off
setlocal enabledelayedexpansion

REM ���̽� �ٿ�ε� URL �� ���� �̸�
set "python_url=https://www.python.org/ftp/python/3.8.6/python-3.8.6-amd64.exe"
set "python_installer=python-3.8.6-amd64.exe"

REM ��ġ ���丮 ����
set "install_dir=C:\Python38"

set "PythonPath=C:\Python38\"
set "PythonScriptPath=C:\Python38\Scripts\"
set "NewPath=%PythonScriptPath%;%PythonPath%;"
set "PythonV=1"
set "Check=1"

REM ���̽��� ��ġ�Ǿ� �ִ°�
where python > nul 2>&1
if !errorlevel! equ 0 (
    python -c "import sys; exit(0) if sys.version.startswith('3.8') else exit(1)" > nul 2>&1
    if !errorlevel! equ 0 (
        echo ���̽� 3.8 ������ �̹� ��ġ�Ǿ� �ֽ��ϴ�.
        set "PythonV=0"
    )
)

REM ���̽� ��ġ
REM �� ���⸦ ����
if !PythonV! equ 1 (
    REM ���̽� ��ġ ���丮�� ������ ����
    if not exist "!install_dir!" mkdir "!install_dir!"
    REM ���̽� ��ġ ���� �ٿ�ε�
    echo �ٿ�ε� ��: !python_url!
    curl -o !install_dir!\!python_installer! !python_url!

    echo �ٿ�ε� �Ϸ�, ��ġ ����

    REM ���̽� ��ġ ����
    start /wait !install_dir!\!python_installer! /quiet TargetDir=!install_dir!
    
    if !errorlevel! equ 0 (   
        echo ���̽� 3.8 ���� ��ġ�� �Ϸ�Ǿ����ϴ�.
    ) else (
        echo ���̽� 3.8 ���� ��ġ�� �����߽��ϴ�.
    )
)

REM ȯ�� ���� ����
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

REM ����� PATH ���� ����
setx PATH "%Newpath%" /m

echo ȯ�� ������ ���� ���� �߰��Ǿ����ϴ�.

python -m pip install --upgrade pip
pip install pillow==7.2
pip install folium
pip install staticmap
pause

endlocal