@echo off
setlocal

REM ���̽� �ٿ�ε� URL �� ���� �̸�
set "python_url=https://www.python.org/ftp/python/3.8.6/python-3.8.6-amd64.exe"
set "python_installer=python-3.8.6-amd64.exe"

REM ��ġ ���丮 ����
set "install_dir=C:\Python38"

python -c "import sys; exit(0) if sys.version.startswith('3.8') else exit(1)" > nul 2>&1
if %errorlevel% equ 0 (
    echo ���̽� 3.8 ������ �̹� ��ġ�Ǿ� �ֽ��ϴ�.
    python -m pip install --upgrade pip
    pip install pillow==7.2
    pip install folium
    pip install staticmap
    pause
) else (
    REM ���̽� ��ġ ���丮�� ������ ����
    if not exist "%install_dir%" mkdir "%install_dir%"

    REM ���̽� ��ġ ���� �ٿ�ε�
    echo �ٿ�ε� ��: %python_url%
    curl -o %install_dir%\%python_installer% %python_url%

    echo �ٿ�ε� �Ϸ�, ��ġ ����

    REM ���̽� ��ġ ����
    start /wait %install_dir%\%python_installer% /quiet TargetDir=%install_dir%

    if %errorlevel% equ 1 (
         REM ȯ�� ���� ����
        setx PATH "%PATH%;%install_dir%;" /M
        python -m pip install --upgrade pip
        pip install pillow==7.2
        pip install folium
        pip install staticmap
        echo ���̽� 3.8 ���� ��ġ�� �Ϸ�Ǿ����ϴ�.
    ) else (
        echo ���̽� 3.8 ���� ��ġ�� �����߽��ϴ�.
    )
)

pause

endlocal