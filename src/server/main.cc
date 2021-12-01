#include <iostream>

#include <gf/TcpListener.h>
#include <gf/Packet.h>
#include <gf/TcpSocket.h>
#include "../common/protocol.h"

int main() {

    gf::TcpListener listener("42690");
    gf::TcpSocket player1, player2;
    gf::Packet packet;

    //bool treated1 = false;
    //bool treated2 = false;

    while(!player1 || !player2) {
        gf::TcpSocket client = listener.accept();
        if (!player1) {
            player1 = std::move(client);
        } else {
            player2 = std::move(client);
        }

        if(player1 && !player2) {
            std::cout << "A client has connected" << std::endl;

            player1.recvPacket(packet);
            stg::ClientHello hello = packet.as<stg::ClientHello>();
            std::cout << "Hello " << hello.name << std::endl;

            stg::ServerMessage resp;
            resp.code = stg::ResponseCode::WAITING;
            resp.message = "You are the first player connected. Waiting for another one...";
            packet.is(resp);
            player1.sendPacket(packet);

            //treated1 = true;
        }
        
        if (player2) {
            std::cout << "A client has connected" << std::endl;

            player2.recvPacket(packet);
            stg::ClientHello hello = packet.as<stg::ClientHello>();
            std::cout << "Hello " << hello.name << std::endl;

            stg::ServerMessage resp;
            resp.code = stg::ResponseCode::STARTING;
            resp.message = "Both players have connected. Starting the game.";
            packet.is(resp);

            player1.sendPacket(packet);
            player2.sendPacket(packet);

            //treated2 = true;
        }
    }

    std::cout << "Both clients have connected" << std::endl;

}
