#include <iostream>
#include <random>

#include <gf/TcpListener.h>
#include <gf/Packet.h>
#include <gf/TcpSocket.h>

#include "../common/protocol.h"
#include "../common/board.h"

void dealWithRequest(gf::TcpSocket &sender, gf::TcpSocket &other, gf::Packet &packet) {
    switch (packet.getType()) {
        case stg::ClientMoveRequest::type:
        {
            stg::ClientMoveRequest request = packet.as<stg::ClientMoveRequest>();
        }
        break;

        default:
        break;
    }
}

int main() {

    gf::TcpListener listener("42690");
    gf::TcpSocket player1, player2;
    gf::Packet packet;

    bool inGame = false;

    while(!inGame) {
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
        }
        
        if (player2) {
            std::cout << "A client has connected" << std::endl;

            player2.recvPacket(packet);
            stg::ClientHello hello = packet.as<stg::ClientHello>();
            std::cout << "Hello " << hello.name << std::endl;

            inGame = true;

        }
    }

    std::random_device r;

    std::default_random_engine engine(r());
    std::uniform_int_distribution<int> uniform_dist(0,1);

    int turn = uniform_dist(engine);
    int p1Color = uniform_dist(engine);

    stg::ServerAssignColor toP1, toP2;
    toP1.color = p1Color == 0 ? stg::Color::RED : stg::Color::BLUE;
    toP1.starting = turn == 0;

    toP2.color = p1Color == 0 ? stg::Color::BLUE : stg::Color::RED;
    toP2.starting = turn == 1;


    packet.is(toP1);
    player1.sendPacket(packet);

    packet.is(toP2);
    player2.sendPacket(packet);


    stg::ServerMessage resp;
    resp.code = stg::ResponseCode::STARTING;
    resp.message = "Both players have connected. Starting the game.";
    packet.is(resp);

    player1.sendPacket(packet);
    player2.sendPacket(packet);

    while (inGame) {
        player1.recvPacket(packet);
        dealWithRequest(player1,player2,packet);

        player2.recvPacket(packet);
        dealWithRequest(player2,player1,packet);
    }

}
