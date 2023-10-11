# 환경소개
- OS : windows10
- Language : C++, python, WINAPI

- Python version : 3.8.6
- Need Libs : staticmap, pillow, request
※ PythonSetup 배치 파일 실행시, 관리자 권한으로 실행시켜야 함.
※ 배치파일 실행하게 되면, C:\Python38 폴더 생성 및 환경 변수 저장까지 완료

# 코드 설명

## Accept Code
[TIMServer.h](TIM/TIMServer.h) : 네트워크 통신이 구현되어 있는 클래스로, 연결 및 종료, 패킷 동작 등을 수행합니다.
![image](docs/Image/AcceptServerFlow.png)


### TIRD Flow Chart
[TirdSession.h](TIM/TirdSession.h) : TIRD 기기를 나타내는 클래스로, TIRD 기기가 접속하면 생성되는 객체입니다.
![image](docs/Image/AcceptServerFlow.png)


### TIFD Flow Chart
[TifdSession.h](TIM/TifdSession.h) : TIFD 기기를 나타내는 클래스로, TIFD 기기가 접속하면 생성되는 객체입니다.
![image](docs/Image/AcceptServerFlow.png)


### GUI
[WinApi.h](TIM/WinApi.h) : WINAPI를 사용하여 GUI를 나타내는 클래스이다. 정보는 [TIMServer.h]에서 넘겨받아 [JobQueue.h](TIM/JobQueue.h) 방식으로 실행됩니다.
[PythonMap.h](TIM/PythonMap.h) : Python 프로그램을 적제하여 실행할 수 있도록 하는 클래스입니다.
[CreateMap.py](TIM/CreateMap.py) : Python 프로그램으로 [PythonMap.h]에 적재할 함수가 들어가 있습니다.

### Utils
[JsonParser.h](TIM/JsonParser.h) : 간단하게 제작한 JsonParser 클래스로, 처음 설정 정보를 가져올 때 사용합니다.
[FileUtils.h](TIM/FileUtils.h) : 로그 저장을 간편하게 하기 위해 제작한 유틸 클래스입니다.
[Utils.h](TIM/Utils.h) : 필요한 유틸 기능 함수들이 들어가있는 파일입니다.


### 자료형
[Packet.h](TIM/Packet.h) : 패킷 형태가 들어가있습니다.
[Enums.h](TIM/Enums.h) : Enum 형태들이 들어가있습니다.
[config.json](TIM/config.json) : 초기 설정값이 들어가 있습니다.


