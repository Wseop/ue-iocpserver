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
- `IOCP 모델`과 `JobQueue`를 기반으로 Packet과 Job(Task)들을 MultiThread로 처리
```c++
// Service.cpp
for (uint32 i = 0; i < thread::hardware_concurrency(); i++)
{
    gThreadManager->Launch([this]()
	{
	    while (true)
	    {
		_iocpCore->Dispatch(10);
		gThreadManager->ExecuteJob();
		gThreadManager->DistributeReservedJob();
	    }
	});
}
```

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

<img src="https://github.com/Wseop/ue-iocpserver/assets/18005580/f063c7af-9118-40c1-9a2c-da658c5d3d1f" width="500" height="300" />

- Push는 동시에 가능
- Pop 및 Job 실행은 `하나의 Thread`가 전담
</Details>

<Details>
<Summary>3. Packet 수신 (Recv) </Summary>

<img src="https://github.com/Wseop/ue-iocpserver/assets/18005580/f553abc8-a116-4a4a-91d0-8b3f0d44f653" width="560" height="400" />

- 하나의 완성된 Packet만큼 수신하였다면, Header를 Parsing하여 Packet Type과 Payload Size를 추출
- Packet Type에 따라, 적절한 Handler 함수를 호출. Protobuf로 역직렬화(ParseFromArray())

</Details>

<Details>
<Summary>4. Packet 송신 (Send) </Summary>

<img src="https://github.com/Wseop/ue-iocpserver/assets/18005580/9decce14-9a71-4043-b098-2ba676210b80" width="545" height="400" />

- 전송할 데이터(Payload)를 Protobuf로 직렬화(SerializeToArray())하여 SendBuffer 생성
```c++
// PacketHandler.h
template<typename T>
static inline shared_ptr<SendBuffer> MakeSendBuffer(PacketType packetType, T* payload)
{
    // Packet 크기 계산
    uint32 payloadSize = static_cast<uint32>(payload->ByteSizeLong());
    uint32 packetSize = payloadSize + sizeof(PacketHeader);

    // SendBuffer 생성
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(packetSize);

    // SendBuffer에 Header 추가
    PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
    header->packetType = packetType;
    header->packetSize = packetSize;

    // SendBuffer에 Payload 추가
    assert(payload->SerializeToArray(header + 1, payloadSize));

    return sendBuffer;
}
```
- 생성된 SendBuffer를 SendQueue에 Push
- 하나의 Thread가 전담하여 SendBuffer들을 전송
```c++
// Session.cpp - Send()
{
    // Queue에 넣는건 MultiThread로 동작
    lock_guard<mutex> lock(_mutex);

    _sendQueue.push(sendBuffer);
}
// SendBuffer 전송은 하나의 Thread가 전담
if (_bSendRegistered.exchange(true) == false)
    RegisterSend();
```
</Details>

<hr />

### AppServer
- 서버 앱
- Listen 수행
- 게임방 관리, 플레이어 생성 및 이동, Broadcast 등의 기능 수행

<hr />

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
