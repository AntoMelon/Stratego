#include <iostream>

#include <gf/TcpListener.h>
#include <gf/Packet.h>
#include <gf/TcpSocket.h>
#include "../common/protocol.h"

int main() {

    gf::TcpListener listener("42690");

    for(;;) {
        gf::TcpSocket client = listener.accept();

        if(client) {
            std::cout << "A client has connected" << std::endl;
            gf::Packet packet;

            client.recvPacket(packet);
            stg::ClientHello hello = packet.as<stg::ClientHello>();
            std::cout << "Hello " << hello.name << std::endl;
        }
    }



}
