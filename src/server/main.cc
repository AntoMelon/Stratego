#include <gf/TcpListener.h>
#include <gf/Packet.h>
#include <gf/TcpSocket.h>
#include "../common/protocol.h"

int main() {

    gf::TcpListener listener("42690");

    for(;;) {
        gf::TcpSocket client = listener.accept();

        if(client) {
            gf::Packet packet;
            client.recvPacket(packet);
            stg::Request req = packet.as<stg::Request>();
            cout << req.type;
        }
    }



}
