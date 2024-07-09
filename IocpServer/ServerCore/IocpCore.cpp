#include "pch.h"
#include "IocpCore.h"
#include "IocpObject.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
    _iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    assert(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
    assert(::CloseHandle(_iocpHandle));
}

bool IocpCore::registerObject(shared_ptr<IocpObject> iocpObject)
{
    return ::CreateIoCompletionPort(iocpObject->getHandle(), _iocpHandle, 0, 0);
}

void IocpCore::dispatchEvent(uint32 timeoutMs)
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
        iocpEvent->getOwner()->processEvent(iocpEvent, numOfBytes);
}
