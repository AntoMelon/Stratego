#include "../common/protocol.h"
#include <gf/TcpSocket.h>
#include <gf/Packet.h>

int main() {

    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690");

    /*stg::ErrorRequest req;
     *
     */
    gf::Packet data = gf::Packet();
    data.is("Lel");

    std::string test = std::string("Test");

    socket_client.sendPacket(data);

    return 0;
}
