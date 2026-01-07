
#include "pch.h"

class ClientSession : public enable_shared_from_this<ClientSession> {
    public:
        ClientSession(boost_tcp::socket socket, uint8 id)
            : _socket(move(socket)),
              _id(id) {}

        void Start() { cout << "Client Start." << endl; }

    private:
        boost_tcp::socket _socket;
        uint8 _id;
};
