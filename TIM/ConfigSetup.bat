pushd %~dp0

XCOPY /Y config.json "../binary/Debug"
XCOPY /Y config.json "../binary/Release"

XCOPY /Y CreateMap.py "../binary/Debug"
XCOPY /Y CreateMap.py "../binary/Release"