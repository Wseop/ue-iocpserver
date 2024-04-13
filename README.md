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
- IOCP 모델과 JobQueue를 기반으로 Packet과 Job(Task)들을 MultiThread로 처리

<Details>
<Summary>1. IOCP 모델</Summary>

<img src="https://github.com/Wseop/ue-iocpserver/assets/18005580/394b0d4d-c4f2-4840-80a5-4973807b0c1e" width="420" height="300" />

```c++
// IocpCore.cpp
void IocpCore::Dispatch(uint32 timeoutMs)
{
    ULONG_PTR dummyKey = 0;
    IocpEvent* iocpEvent = nullptr;
    DWORD numOfBytes = 0;

    if (::GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &dummyKey, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs) == false)
    {
        int32 errorCode = ::WSAGetLastError();
        switch (errorCode)
        {
        case WAIT_TIMEOUT:
            return;
        case ERROR_NETNAME_DELETED:
            break;
        default:
            spdlog::error("IocpCore : Dispatch Error[{}]", errorCode);
            return;
        }
    }

    if (iocpEvent)
        iocpEvent->GetOwner()->Dispatch(iocpEvent, numOfBytes);
}
```
```c++
// Listener.cpp
void Listener::Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes)
{
    assert(iocpEvent->GetEventType() == EventType::Accept);
    ProcessAccept(iocpEvent);
}
```
```c++
// Session.cpp
void Session::Dispatch(IocpEvent* iocpEvent, uint32 numOfBytes)
{
    switch (iocpEvent->GetEventType())
    {
    case EventType::Connect:
	ProcessConnect();
	break;
    case EventType::Disconnect:
	ProcessDisconnect();
	break;
    case EventType::Recv:
	ProcessRecv(numOfBytes);
	break;
    case EventType::Send:
	ProcessSend(numOfBytes);
	break;
    default:
	spdlog::error("Session[{}] : Invalid EventType[{}]", _sessionId, static_cast<uint8>(iocpEvent->GetEventType()));
	break;
    }
}
```
</Details>

<Details>
<Summary>2. JobQueue</Summary>


</Details>

<Details>
<Summary>3. MultiThread 환경에서 처리</Summary>


</Details>

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
## UE_Client 3개 접속
[35c1595f-990a-4c11-bed3-899d343dbc05.webm](https://github.com/Wseop/ue-iocpserver/assets/18005580/e6494696-f03c-422f-84a8-43b1773214b5)

## UE_Client 2개와 DummyClient를 사용한 이동 동기화 테스트
[2cdbecc6-66ca-4cee-8c98-0e10656da40d.webm](https://github.com/Wseop/ue-iocpserver/assets/18005580/aa8781ac-a8f9-439b-9f90-19bcb68bfc00)
- DummyClient가 소환한 50명의 플레이어가 하나의 플레이어를 따라다니는 상황을 구현.
# Reference
- [[게임 프로그래머 입문 올인원] C++ & 자료구조/알고리즘 & STL & 게임 수학 & Windows API & 게임 서버](https://www.inflearn.com/course/%EA%B2%8C%EC%9E%84-%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%98%EB%A8%B8-%EC%9E%85%EB%AC%B8-%EC%98%AC%EC%9D%B8%EC%9B%90-rookiss)
- [[C++과 언리얼로 만드는 MMORPG 게임 개발 시리즈] Part4: 게임 서버](https://www.inflearn.com/course/%EC%96%B8%EB%A6%AC%EC%96%BC-3d-mmorpg-4)
- [[C++과 언리얼로 만드는 MMORPG 게임 개발 시리즈] Part5: UE5 & IOCP 서버 연동](https://www.inflearn.com/course/unreal-3d-mmorpg-5)
