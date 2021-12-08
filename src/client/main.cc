#include <iostream>
#include <thread>

#include "../common/protocol.h"
#include "./libs/board.h"

#include <gf/Queue.h>
#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Event.h>
#include <gf/Color.h>
#include <gf/Sprite.h>

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

    gf::Vector2i windowSize(640, 640);
    gf::Window window("Stratego en réseau", windowSize);
    gf::RenderWindow renderer(window);

    stg::Board board;

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

    //Tests sur les pièces (PLAYGROUND)
    board.setPiece(0,0,stg::Piece(stg::PieceName::MARECHAL,stg::Color::BLUE));
    board.setPiece(1,0,stg::Piece(stg::PieceName::GENERAL,stg::Color::BLUE));
    board.setPiece(2,0,stg::Piece(stg::PieceName::COLONEL,stg::Color::BLUE));
    board.setPiece(3,0,stg::Piece(stg::PieceName::COMMANDANT,stg::Color::BLUE));
    board.setPiece(4,0,stg::Piece(stg::PieceName::CAPITAINE,stg::Color::BLUE));
    board.setPiece(5,0,stg::Piece(stg::PieceName::LIEUTENANT,stg::Color::BLUE));
    board.setPiece(6,0,stg::Piece(stg::PieceName::SERGENT,stg::Color::BLUE));
    board.setPiece(7,0,stg::Piece(stg::PieceName::DEMINEUR,stg::Color::BLUE));
    board.setPiece(8,0,stg::Piece(stg::PieceName::ESPION,stg::Color::BLUE));
    board.setPiece(9,0,stg::Piece(stg::PieceName::BOMBE,stg::Color::BLUE));
    board.setPiece(4,1,stg::Piece(stg::PieceName::DRAPEAU,stg::Color::BLUE));
    board.setPiece(5,1,stg::Piece(stg::PieceName::PION,stg::Color::BLUE));
    board.setPiece(0,1,stg::Piece(stg::PieceName::ECLAIREUR,stg::Color::BLUE));

    board.setPiece(0,9,stg::Piece(stg::PieceName::MARECHAL,stg::Color::RED));
    board.setPiece(1,9,stg::Piece(stg::PieceName::GENERAL,stg::Color::RED));
    board.setPiece(2,9,stg::Piece(stg::PieceName::COLONEL,stg::Color::RED));
    board.setPiece(3,9,stg::Piece(stg::PieceName::COMMANDANT,stg::Color::RED));
    board.setPiece(4,9,stg::Piece(stg::PieceName::CAPITAINE,stg::Color::RED));
    board.setPiece(5,9,stg::Piece(stg::PieceName::LIEUTENANT,stg::Color::RED));
    board.setPiece(6,9,stg::Piece(stg::PieceName::SERGENT,stg::Color::RED));
    board.setPiece(7,9,stg::Piece(stg::PieceName::DEMINEUR,stg::Color::RED));
    board.setPiece(8,9,stg::Piece(stg::PieceName::ESPION,stg::Color::RED));
    board.setPiece(9,9,stg::Piece(stg::PieceName::BOMBE,stg::Color::RED));
    board.setPiece(4,8,stg::Piece(stg::PieceName::DRAPEAU,stg::Color::RED));
    board.setPiece(5,8,stg::Piece(stg::PieceName::PION,stg::Color::RED));
    board.setPiece(0,8,stg::Piece(stg::PieceName::ECLAIREUR,stg::Color::RED));

    while (window.isOpen()) {

        gf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case gf::EventType::Closed:
                    window.close();
                    break;
                default:
                    break;
            }
        }

        renderer.clear(gf::Color::White);

        board.render(renderer);

        renderer.display();
    }

    return 0;
}

