@echo off
setlocal enabledelayedexpansion

REM 파이썬 다운로드 URL 및 파일 이름
set "python_url=https://www.python.org/ftp/python/3.8.6/python-3.8.6-amd64.exe"
set "python_installer=python-3.8.6-amd64.exe"

REM 설치 디렉토리 지정
set "install_dir=C:\Python38"

set "PythonPath=C:\Python38\"
set "PythonScriptPath=C:\Python38\Scripts\"
set "NewPath=%PythonScriptPath%;%PythonPath%;"
set "PythonV=1"
set "Check=1"

REM 파이썬이 설치되어 있는가
where python > nul 2>&1
if %errorlevel% equ 0 (
    python -c "import sys; exit(0) if sys.version.startswith('3.8') else exit(1)" > nul 2>&1
    if %errorlevel% equ 0 (
        echo 파이썬 3.8 버전이 이미 설치되어 있습니다.
        set "PythonV=0"
    )
)

REM 파이썬 설치
REM 꼭 띄어쓰기를 하자
if !PythonV! equ 1 (
    REM 파이썬 설치 디렉토리가 없으면 생성
    if not exist "!install_dir!" mkdir "!install_dir!"
    REM 파이썬 설치 파일 다운로드
    echo 다운로드 중: !python_url!
    curl -o !install_dir!\!python_installer! !python_url!

    echo 다운로드 완료, 설치 시작

    REM 파이썬 설치 실행
    start /wait !install_dir!\!python_installer! /quiet TargetDir=!install_dir!
    
    if !errorlevel! equ 0 (   
        echo 파이썬 3.8 버전 설치가 완료되었습니다.
    ) else (
        echo 파이썬 3.8 버전 설치에 실패했습니다.
    )
)

REM 환경 변수 설정
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

REM 변경된 PATH 값을 설정
setx PATH "%Newpath%" /m

echo 환경 변수가 제일 위에 추가되었습니다.

python -m pip install --upgrade pip
pip install pillow==7.2
pip install folium
pip install staticmap
pause

endlocal