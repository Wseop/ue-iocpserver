#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "IocpObject.h"

IocpCore::IocpCore()
{
    _iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    assert(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
    assert(::CloseHandle(_iocpHandle));
}

bool IocpCore::Register(shared_ptr<IocpObject> iocpObject)
{
    return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0);
}

void IocpCore::Dispatch(uint32 timeoutMs)
{
    DWORD numOfBytes = 0;
    ULONG_PTR dummyKey = 0;
    IocpEvent* iocpEvent = nullptr;

    if (::GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &dummyKey, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs) == false)
    {
        int32 errorCode = ::WSAGetLastError();

        switch (errorCode)
        {
        case WAIT_TIMEOUT:
        case ERROR_NETNAME_DELETED:
            break;
        default:
            spdlog::error("Dispatch Error {}", errorCode);
            break;
        }
    }

    if (iocpEvent)
        iocpEvent->GetOwner()->Dispatch(iocpEvent, numOfBytes);
}
