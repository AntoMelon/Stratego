#include <iostream>
#include <thread>

#include "../common/protocol.h"
#include "../common/board.h"

#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <gf/Queue.h>

gf::Queue<gf::Packet> serverPackets;

void threadPackets(gf::TcpSocket& socket,gf::Queue<gf::Packet>& queue) {
    gf::Packet packet;
    for(;;) {
        gf::SocketStatus status = socket.recvPacket(packet);
        if (status == gf::SocketStatus::Close) {
            return;
        }
        queue.push(packet);
    }
}

int main() {

    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690");
    bool inGame = false;

    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach();

    stg::ClientHello data;
    data.name = "IT WORKS";

    gf::Packet data2;
    data2.is(data);

    socket_client.sendPacket(data2);

    while (!inGame) {
        gf::Packet response;

        if (serverPackets.poll(response)) {
            switch(response.getType()) {
                case stg::ServerMessage::type:
                stg::ServerMessage msg = response.as<stg::ServerMessage>();

                switch(msg.code) {
                    case stg::ResponseCode::WAITING:
                    std::cout << msg.message << std::endl;
                    break;

                    case stg::ResponseCode::STARTING:
                    std::cout << msg.message << std::endl;
                    inGame = true;
                    break;

                    default:
                    break;
                }

                break;
            }
        }
    }

    return 0;
}
