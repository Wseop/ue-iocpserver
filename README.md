# IocpServer
IOCP를 활용하여 구현해 본 게임서버입니다.<br>
클라이언트 접속 및 플레이어 생성 그리고 이동 동기화 등의 기능을 간단하게 구현해보았습니다.
# 개발 환경
- 서버 : visual studio 2022, c++20
- 클라이언트 : UnrealEngine 5.3.2
- 라이브러리 : protobuf 21.12, spdlog
# Project Description
## IocpServer
### ServerCore
- 서버 라이브러리
- IOCP와 JobQueue를 기반으로 MultiThread 환경에서 작업들을 처리
```c++
for (uint32 i = 0; i < thread::hardware_concurrency(); i++)
{
	gThreadManager->Launch([this]()
		{
			while (true)
			{
				_iocpCore->Dispatch(10);  // IOCP 큐에 들어온 작업 처리
				gThreadManager->ExecuteJob();  // JobQueue에 들어있는 작업들 처리
				gThreadManager->DistributeReservedJob();  // 예약된 작업들 처리
			}
		});
}
```
- 서버와 클라이언트간 패킷 송수신은 Protobuf로 직렬화
### AppServer
- 서버 앱
- Listen 수행
- 게임방 관리, 플레이어 생성 및 이동, Broadcast 등의 기능 수행
### DummyClient
- 서버 테스트 목적으로 구현한 클라이언트 앱
- 다수의 플레이어를 생성하여 접속, 이동 테스트를 수행
## UE_Client
- 플레이어 접속, 이동 동기화 등을 시각적으로 확인하기 위해 구현한 클라이언트 앱
# 실행 예시
- TODO
# Reference
- [[게임 프로그래머 입문 올인원] C++ & 자료구조/알고리즘 & STL & 게임 수학 & Windows API & 게임 서버](https://www.inflearn.com/course/%EA%B2%8C%EC%9E%84-%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%98%EB%A8%B8-%EC%9E%85%EB%AC%B8-%EC%98%AC%EC%9D%B8%EC%9B%90-rookiss)
- [[C++과 언리얼로 만드는 MMORPG 게임 개발 시리즈] Part4: 게임 서버](https://www.inflearn.com/course/%EC%96%B8%EB%A6%AC%EC%96%BC-3d-mmorpg-4)
- [[C++과 언리얼로 만드는 MMORPG 게임 개발 시리즈] Part5: UE5 & IOCP 서버 연동](https://www.inflearn.com/course/unreal-3d-mmorpg-5)
