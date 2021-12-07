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

    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690"); //parametre de connxeion
    bool inGame = false; //booleen jeu

    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach(); // indépendance thread/execution

    stg::ClientHello data; //ressources à enoyer
    data.name = "IT WORKS"; //parametre du nom

    gf::Packet data2; //créer packet
    data2.is(data); // serialiser packet

    socket_client.sendPacket(data2); //envoie du packet

    while (!inGame) { //en jeu
        gf::Packet response; //packet reponse

        if (serverPackets.poll(response)) { //si packet reçu
            switch(response.getType()) { //e fonction du type
                case stg::ServerMessage::type: //message
                stg::ServerMessage msg = response.as<stg::ServerMessage>(); //deserialiser

                switch(msg.code) { //code reçu
                    case stg::ResponseCode::WAITING: //en attente
                    std::cout << msg.message << std::endl; //afficher en attente
                    break;

                    case stg::ResponseCode::STARTING: //commencer
                    std::cout << msg.message << std::endl; //afficher debut
                    inGame = true; //jeu commencer
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
