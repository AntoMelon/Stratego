#include "../common/protocol.h"
#include <gf/TcpSocket.h>
#include <gf/Packet.h>

int main() {

    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690");

    /*stg::ErrorRequest req;
     *
     */
    /*gf::Packet data = gf::Packet();
    data.is("Lel");*/

    stg::ClientHello data;
    data.name = "Test";

    gf::Packet data2;
    data2.is(data);

    //gf::Packet::is(data);
    socket_client.sendPacket(data2);

    return 0;
}
