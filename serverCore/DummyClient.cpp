#include "ClientSession.h"
#include "Types.h"
#include "pch.h"
#include <memory>

class DummyClient : public enable_shared_from_this<DummyClient> {
    public:
        DummyClient(boost_io_context &io_context, const string &host,
                    const string &port, int id)
            : _socket(io_context),
              _resolver(io_context),
              _host(host),
              _port(port),
              _id(id) {}

        void Start() { DoConnect(); }

    private:
        boost_tcp::socket _socket;
        boost_tcp::resolver _resolver;
        string _host;
        string _port;
        uint8 _id;
        shared_ptr<ClientSession> _session;

        void DoConnect() {
            auto self(shared_from_this());
            auto endPoint = _resolver.resolve(_host, _port);

            async_connect(
                _socket, endPoint,
                [this, self](boost_error_code ec, boost_tcp::endpoint) {
                    if (!ec) {
                        _session =
                            make_shared<ClientSession>(move(_socket), _id);
                        _session->Start();
                    } else {
                        cout << "Client Connect Error : " << ec.message()
                             << endl;
                    }
                });
        }
};

int main() {
    try {
        boost_io_context io_context;

        auto client1 =
            make_shared<DummyClient>(io_context, "127.0.0.1", "7777", 1);

        client1->Start();

        io_context.run();

    } catch (exception &e) {
        cerr << "Exception : " << e.what() << endl;
    }

    return 0;
}
