#include <iostream>

#include "../common/protocol.h"
#include <gf/TcpSocket.h>
#include <gf/Packet.h>

int main() {

    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690");
    bool inGame = false;

    stg::ClientHello data;
    data.name = "IT WORKS";

    gf::Packet data2;
    data2.is(data);

    socket_client.sendPacket(data2);

    while (!inGame) {
        gf::Packet response;
        socket_client.recvPacket(response);

        stg::ServerMessage resp = response.as<stg::ServerMessage>();

        if (resp.code == 0) {
            std::cout << resp.message << std::endl;
        } else if (resp.code == 1) {
            std::cout << resp.message << std::endl;
            inGame = true;
        }
    }

    return 0;
}
