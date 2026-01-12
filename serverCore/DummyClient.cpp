// DummyClient.cpp

#include "ClientSession.h"
#include "PacketHandler.h"
#include "pch.h"
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>

using boost::asio::ip::tcp;
using namespace std;

class DummyClient : public enable_shared_from_this<DummyClient> {
    public:
        DummyClient(boost::asio::io_context &io_context, const string &host,
                    const string &port, int id)
            : _socket(io_context),
              _resolver(io_context),
              _timer(io_context),
              _host(host),
              _port(port),
              _id(id),
              _msgCount(0) {}

        void Start() {
            cout << "=== [DummyClient " << _id
                 << "] Start() 호출됨 ===" << endl;
            DoConnect();
        }

    private:
        tcp::socket _socket;
        tcp::resolver _resolver;
        boost::asio::steady_timer _timer;
        string _host;
        string _port;
        int _id;
        int _msgCount;
        shared_ptr<ClientSession> _session;

        void DoConnect() {
            cout << "=== [DummyClient " << _id
                 << "] DoConnect() 시작 ===" << endl;

            auto self(shared_from_this());
            auto endPoints = _resolver.resolve(_host, _port);

            boost::asio::async_connect(
                _socket, endPoints,
                [this, self](boost::system::error_code ec, tcp::endpoint) {
                    cout << "=== [DummyClient " << _id
                         << "] async_connect 콜백 실행 ===" << endl;

                    if (!ec) {
                        cout << "[Client " << _id << "] 서버 연결 성공\n";

                        // 세션 생성 및 시작
                        _session =
                            make_shared<ClientSession>(std::move(_socket), _id);
                        _session->Start();

                        cout << "=== [DummyClient " << _id
                             << "] 123 패킷 전송 시작 ===" << endl;

                        // C_Test(123) 패킷 전송
                        Protocol::C_Test cTestPkt;
                        cTestPkt.set_temp(123);
                        auto sendBuffer =
                            PacketHandler::MakeSendBuffer(cTestPkt, PKT_C_TEST);
                        _session->Send(sendBuffer);

                        cout << "=== [DummyClient " << _id
                             << "] 123 패킷 전송 완료 ===" << endl;
                    } else {
                        cout << "[Client " << _id
                             << "] Connect Error: " << ec.message() << endl;
                    }
                });
        }
};

int main() {
    try {
        cout << "=== main() 시작 ===" << endl;

        boost::asio::io_context io_context;

        // 클라이언트 생성
        auto client1 =
            make_shared<DummyClient>(io_context, "127.0.0.1", "7777", 1);

        cout << "=== client1->Start() 호출 전 ===" << endl;
        client1->Start();
        cout << "=== client1->Start() 호출 후 ===" << endl;

        cout << "클라이언트 실행 중...\n";

        io_context.run();

        cout << "=== io_context.run() 종료 ===" << endl;
    } catch (exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}
