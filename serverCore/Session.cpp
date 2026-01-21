#include "Session.h"
#include "IoContext.h"
#include "PacketHeader.h"
#include "SendBuffer.h"

Session::Session(uint16 id, boost_io_context &io_context)
    : _id(id),
      _socket(io_context),
      _recvBuffer(BUFFER_SIZE) {}

Session::~Session() {}

void Session::Connect() {
    OnConnected();
    AsyncRecv();
}

void Session::DisConnect() {
    if (!_connected.exchange(false))
        return;

    boost_error_code ec;

    if (_socket.is_open()) {
        _socket.close(ec);

        if (ec) {
            cout << "Disconnect Fail " << _id << " " << ec.message() << endl;
        }
    }
    OnDisConnected();
    cout << "Disconnect Success " << _id << endl;
}

void Session::AsyncRecv() {
    SessionRef session = GetSessionptr();
    _socket.async_read_some(
        buffer(_recvBuffer.WritePos(), _recvBuffer.FreeSize()),
        [this, session](boost_error_code ec, size_t len) {
            if (ec) {
                HandleError(ec);
                return;
            }

            OnRecv(static_cast<int32>(len));
        });
}

void Session::OnRecv(int32 numOfBytes) {
    if (!_recvBuffer.OnWrite(numOfBytes)) {
        DisConnect();
        return;
    }

    int32 dataSize = _recvBuffer.DataSize();

    uint32 processLen = ProcessRead(_recvBuffer.ReadPos(), dataSize);

    if (processLen < 0 || processLen > dataSize ||
        !_recvBuffer.OnRead(processLen)) {
        DisConnect();
        return;
    }

    _recvBuffer.Clean();

    AsyncRecv();
}

uint32 Session::ProcessRead(BYTE *readBufferPtr, int32 len) {
    int32 processLen = 0;

    while (true) {
        int32 dataSize = len - processLen;

        if (dataSize < sizeof(PacketHeader))
            break;

        PacketHeader *header =
            reinterpret_cast<PacketHeader *>(&readBufferPtr[processLen]);

        if (dataSize < header->size)
            break;

        OnRead(&readBufferPtr[processLen], header->size);

        processLen += header->size;
    }

    return processLen;
}

void Session::Send(SendBufferRef sendBuffer) {
    bool processSend = false;
    {
        WRITE_LOCK;

        _sendQueue.push_back(sendBuffer);

        if (!_isSending.exchange(true)) {
            processSend = true;
        }
    }

    if (processSend)
        ProcessWrite();
}

void Session::ProcessWrite() {
    {
        WRITE_LOCK;
        if (_sendQueue.empty()) {
            _isSending.store(false);
            return;
        }

        _sendingBuffers.clear();
        _sendingBuffers.swap(_sendQueue);
    }

    vector<const_buffer> boostBuffers;
    boostBuffers.reserve(_sendingBuffers.size());

    uint32 sendSize = 0;

    for (const auto &sendBuffer : _sendingBuffers) {
        uint32 writeSize = sendBuffer->WriteSize();
        sendSize += writeSize;

        boostBuffers.push_back(buffer(sendBuffer->Buffer(), writeSize));
    }

    async_write(
        _socket, boostBuffers,
        [this, session = GetSessionptr()](boost_error_code ec, size_t len) {
            session->_sendingBuffers.clear();
            if (ec) {
                cout << "error" << endl;
                HandleError(ec);
                return;
            }

            WRITE_LOCK;
            if (session->_sendQueue.empty())
                session->_isSending.store(false);
            else
                ProcessWrite();
        });
}

void Session::HandleError(boost_error_code ec) {
    switch (ec.value()) {
    case 0:
        break;
    case error::operation_aborted:
        cout << "Session Aborted. Disconnect In Progress." << endl;
        break;
    default:
        DisConnect();
        break;
    }
}
