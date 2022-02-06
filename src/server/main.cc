#include <iostream>
#include <random>
#include <thread>
#include <chrono>

#include <map>

#include <gf/TcpListener.h>
#include <gf/Packet.h>
#include <gf/TcpSocket.h>
#include <gf/Queue.h>

#include "../common/protocol.h"
#include "./server_board.h"

using namespace std::chrono_literals;

//thread de gestion des packets
auto threadPackets = [] (gf::TcpSocket &socket,gf::Queue<gf::Packet> &queue) {
    gf::Packet packet;
    for(;;) {
        gf::SocketStatus status = socket.recvPacket(packet);
        if (status == gf::SocketStatus::Close) {
            return;
        }
        queue.push(packet);
    }
};


bool isSubmittedBoardOk(std::vector<stg::Piece> &board) {
    if (board.size() != 40) {
        std::cout << "wrong size: " << board.size() << std::endl;
        return false;
    }

    std::map<stg::PieceName,int> expected = {{stg::PieceName::DRAPEAU,1},
                                                {stg::PieceName::ESPION,1},
                                                {stg::PieceName::ECLAIREUR,8},
                                                {stg::PieceName::DEMINEUR,5},
                                                {stg::PieceName::SERGENT,4},
                                                {stg::PieceName::LIEUTENANT,4},
                                                {stg::PieceName::CAPITAINE,4},
                                                {stg::PieceName::COMMANDANT,3},
                                                {stg::PieceName::COLONEL,2},
                                                {stg::PieceName::GENERAL,1},
                                                {stg::PieceName::MARECHAL,1},
                                                {stg::PieceName::BOMBE,6}};

    std::map<stg::PieceName,int> found = {{stg::PieceName::DRAPEAU,0},
                                                {stg::PieceName::ESPION,0},
                                                {stg::PieceName::ECLAIREUR,0},
                                                {stg::PieceName::DEMINEUR,0},
                                                {stg::PieceName::SERGENT,0},
                                                {stg::PieceName::LIEUTENANT,0},
                                                {stg::PieceName::CAPITAINE,0},
                                                {stg::PieceName::COMMANDANT,0},
                                                {stg::PieceName::COLONEL,0},
                                                {stg::PieceName::GENERAL,0},
                                                {stg::PieceName::MARECHAL,0},
                                                {stg::PieceName::BOMBE,0}};


    for (auto p : board) {
        found[p.getPieceName()] += 1;
    }

    for (auto pair : expected) {
        if (pair.second != found.find(pair.first)->second) {
            std::cout << found.find(pair.first)->second << " " << pair.first << " found out of " << pair.second << std::endl;
            return false;
        }
    }

    return true;
}

/*
 * @brief To deal with paquets sent by a client, interpret and treat requests
 * @param gf::TcpSocket &sender : socket from the sender
 * @param gf::TcpSocket &other : socket from other client
 * @param gf::Packet &packet : packet sent
 * @param stg::ServerBoard &board : board of the game
 */
bool dealWithMoveRequest(gf::TcpSocket &sender, gf::TcpSocket &other, gf::Packet &packet, stg::ServerBoard &board) {

    gf::Packet to_send;
    std::cout<<"Reçu"<< std::endl;

    if (packet.getType() == stg::ClientMoveRequest::type) {
            stg::ClientMoveRequest request = packet.as<stg::ClientMoveRequest>();

            stg::Color sender_color = request.color;

            std::cout << "From (" << request.from_x << "," << request.from_y << ") to (" << request.to_x << "," << request.to_y <<")" << std::endl;

            if (!board.isMoveAllowed(request.from_x,request.from_y,request.to_x,request.to_y)) {
                stg::ServerMessage response;
                response.code = stg::ResponseCode::MOVE_ERR;
                response.message = "Déplacement interdit.";

                to_send.is(response);
                sender.sendPacket(to_send);
                return true;
            }

            stg::ServerMoveNotif result;
            result.from_x = request.from_x;
            result.from_y = request.from_y;
            result.to_x = request.to_x;
            result.to_y = request.to_y;
            result.win = false;
            result.lose = false;

            stg::Piece piece_atk = board.getPiece(result.from_x,result.from_y);
            stg::Piece piece_def = board.getPiece(result.to_x,result.to_y);

            result.str_atk = piece_atk.getPieceName();
            result.str_def = piece_def.getPieceName();

            int battle_result = piece_atk.battleResult(piece_def);

            result.atk_alive = true;
            result.def_alive = true;

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

            bool stillRedFlag = board.stillHasFlag(stg::Color::RED);
            bool stillBlueFlag = board.stillHasFlag(stg::Color::BLUE);

            if (sender_color == stg::Color::RED) {
                result.win = !stillBlueFlag;
                result.lose = !stillRedFlag;
            } else {
                result.win = !stillRedFlag;
                result.lose = !stillBlueFlag;
            }

            to_send.is(result);
            sender.sendPacket(to_send);

            if (sender_color == stg::Color::RED) {
                result.win = !stillRedFlag;
                result.lose = !stillBlueFlag;
            } else {
                result.win = !stillBlueFlag;
                result.lose = !stillRedFlag;
            }

            to_send.is(result);
            other.sendPacket(to_send);

        return stillRedFlag && stillBlueFlag;
    }

    return true;
}

bool dealWithRequestIfInitialBoard(gf::TcpSocket& sender, gf::Packet& packet, stg::ServerBoard& board) {
    gf::Packet to_send;

    stg::ClientBoardSubmit submit = packet.as<stg::ClientBoardSubmit>();

    bool boardOk = isSubmittedBoardOk(submit.board);

    stg::ServerMessage response;
            
    if (boardOk) {
        response.code = stg::ResponseCode::BOARD_OK;
        response.message = "Plateau validé.";

        std::cout << "Tries to import" << std::endl;
        board.importSubmittedBoard(submit.color,submit.board);
        std::cout << "Imported" << std::endl;
    } else {
        response.code = stg::ResponseCode::BOARD_ERR;
        response.message = "Plateau validé.";
    }

    to_send.is(response);
    sender.sendPacket(to_send);

    return boardOk;
}

int main() {

    gf::TcpListener listener(PORT);
    gf::TcpSocket player1, player2;
    gf::Packet packet;
    stg::PLAYING_STATE state = stg::PLAYING_STATE::CONNEXION;

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
            resp.message = "Vous êtes connecté. En attente d'un adversaire...";
            packet.is(resp);
            player1.sendPacket(packet);
        }
        
        if (player2) {
            std::cout << "A client has connected" << std::endl;

            player2.recvPacket(packet);
            stg::ClientHello hello = packet.as<stg::ClientHello>();
            std::cout << "Hello " << hello.name << std::endl;

            inGame = true;

            state = stg::PLAYING_STATE::PLACEMENT;

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
    resp.message = "Début de partie. Placez vos pièces.";
    packet.is(resp);


    //lancer thread de gestions des packets
    gf::Queue<gf::Packet> packetsFromP1, packetsFromP2;

    std::thread packetsThreadP1(threadPackets,std::ref(player1),std::ref(packetsFromP1));
    packetsThreadP1.detach();

    std::thread packetsThreadP2(threadPackets,std::ref(player2),std::ref(packetsFromP2));
    packetsThreadP2.detach();


    player1.sendPacket(packet);
    player2.sendPacket(packet);

    
    stg::ServerBoard board;
    bool board1Received = false;
    bool board2Received = false;

    /*
     * boucle automate
     */

    gf::Packet clientPacket;

    bool bothFlags = true;

    while (inGame) {
        if (packetsFromP1.poll(clientPacket)) {
            switch (state) {
                case stg::PLAYING_STATE::PLACEMENT:
                board1Received = dealWithRequestIfInitialBoard(player1,clientPacket,board);
                std::cout << "Plateau validé en attente de l'adversaire..." << board1Received << std::endl;
                break;

                case stg::PLAYING_STATE::IN_GAME:
                {
                    if (turn == 0) {
                        bothFlags = dealWithMoveRequest(player1,player2,clientPacket,board);
                    } else {
                        gf::Packet wrongTurn;
                        stg::ServerMessage wrT;
                        wrT.code = stg::ResponseCode::NOT_YOUR_TURN;
                        wrT.message = "It is not your to play.";

                        wrongTurn.is(wrT);

                        player1.sendPacket(wrongTurn);
                    }
                    break;
                }

                default:
                break;
            }
        }

        if (packetsFromP2.poll(clientPacket)) {
            switch (state) {
                case stg::PLAYING_STATE::PLACEMENT:
                board2Received = dealWithRequestIfInitialBoard(player2,clientPacket,board);
                std::cout << "Plateau validé en attente de l'adversaire..." << board2Received << std::endl;
                break;

                case stg::PLAYING_STATE::IN_GAME:
                {
                    if (turn == 1) {
                        bothFlags = dealWithMoveRequest(player2,player1,clientPacket,board);
                    } else {
                        gf::Packet wrongTurn;
                        stg::ServerMessage wrT;
                        wrT.code = stg::ResponseCode::NOT_YOUR_TURN;
                        wrT.message = "It is not your to play.";

                        wrongTurn.is(wrT);

                        player2.sendPacket(wrongTurn);
                    }
                }
                break;

                default:
                    break;
            } 
        }


        if (board1Received && board2Received && state == stg::PLAYING_STATE::PLACEMENT) {
            std::cout << "Both boards were validated" << std::endl;
            state = stg::PLAYING_STATE::IN_GAME;
        }

        if (!bothFlags && state == stg::PLAYING_STATE::IN_GAME) {
            std::cout << "One flag has been taken. Ending game..." << std::endl;
            state = stg::PLAYING_STATE::END;
            inGame = false;
        }

        std::this_thread::sleep_for(16ms);

    }

}
