#include <iostream>
#include <random>

#include <gf/TcpListener.h>
#include <gf/Packet.h>
#include <gf/TcpSocket.h>

#include "../common/protocol.h"
#include "./server_board.h"

void dealWithRequest(gf::TcpSocket &sender, gf::TcpSocket &other, gf::Packet &packet, stg::ServerBoard &board) {
    gf::Packet to_send;

    switch (packet.getType()) {
        case stg::ClientMoveRequest::type:
        {
            stg::ClientMoveRequest request = packet.as<stg::ClientMoveRequest>();

            if (!board.isMoveAllowed(request.from_x,request.from_y,request.to_x,request.to_y)) {
                stg::ServerMessage response;
                response.code = stg::ResponseCode::MOVE_ERR;
                response.message = "Non-allowed move";

                to_send.is(response);
                sender.sendPacket(to_send);
                return;
            }

            stg::ServerMoveNotif result;
            result.from_x = request.from_x;
            result.from_y = request.from_y;
            result.to_x = request.to_x;
            result.to_y = request.to_y;

            stg::Piece piece_atk = board.getPiece(result.from_x,result.from_y);
            stg::Piece piece_def = board.getPiece(result.to_x,result.to_y);

            result.str_atk = piece_atk.getPieceName();
            result.str_def = piece_def.getPieceName();

            int battle_result = piece_atk.battleResult(piece_def);

            switch (battle_result) {
                case -1:
                result.atk_alive = false;
                result.def_alive = true;
                break;

                case 0:
                result.atk_alive = false;
                result.def_alive = false;
                break;

                case 1:
                result.atk_alive = true;
                result.def_alive = false;
                break;

                default:
                break;
            }

            board.movePiece(result.from_x,result.from_y,result.to_x,result.to_y);

            to_send.is(result);

            sender.sendPacket(to_send);
            other.sendPacket(to_send);
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

    
    stg::ServerBoard board;

    while (inGame) {
        player1.recvPacket(packet);
        dealWithRequest(player1,player2,packet,board);

        player2.recvPacket(packet);
        dealWithRequest(player2,player1,packet,board);
    }

}
