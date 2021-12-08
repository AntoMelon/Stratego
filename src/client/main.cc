#include <iostream>
#include <thread>

#include "../common/protocol.h"
#include "../common/board.h"

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

namespace stg {

    enum TileType{
        Land,
        River
    };

    enum PieceType{
        Flag,
        Spy,
        Scout,
        Deminer,
        Sergeant,
        Lieutenant,
        Captain,
        Commander,
        Colonel,
        General,
        Marshal,
        Bomb
    };

    struct Tile {
        TileType type;
        gf::Sprite sprite;
    };

    struct GraphicBoard {
        std::vector<std::vector<Tile>> board;
    };

    struct Square {
        TileType type;
        bool walkable;
    };

    /*struct Piece {
        PieceType type;
        int strength;
        bool canMove;
    };*/

}

int main() {

    gf::Vector2i windowSize(640, 640);
    gf::Window window("Stratego en réseau", windowSize);
    gf::RenderWindow renderer(window);

    gf::Texture land = gf::Texture(gf::Path("./resources/field.png"));
    gf::Texture land_top_side = gf::Texture(gf::Path("./resources/field_top.png"));
    gf::Texture land_bot_side = gf::Texture(gf::Path("./resources/field_bottom.png"));   //TODO : Poser la question d'un moyen plus efficace
    gf::Texture land_right_side = gf::Texture(gf::Path("./resources/field_right.png"));
    gf::Texture land_left_side = gf::Texture(gf::Path("./resources/field_left.png"));
    gf::Texture land_top_left_edge = gf::Texture(gf::Path("./resources/field_top_left.png"));
    gf::Texture land_top_right_edge = gf::Texture(gf::Path("./resources/field_top_right.png"));
    gf::Texture land_bot_left_edge = gf::Texture(gf::Path("./resources/field_bottom_left.png"));
    gf::Texture land_bot_right_edge = gf::Texture(gf::Path("./resources/field_bottom_right.png"));

    gf::Texture base_piece_blue = gf::Texture(gf::Path("./resources/blue_piece.png"));
    gf::Texture base_piece_red = gf::Texture(gf::Path("./resources/red_piece.png"));

    stg::GraphicBoard board;
    for (int i = 0; i < 10; i++) {
        std::vector<stg::Tile> row;
        for (int j = 0; j < 10; j++) {
            stg::Tile tile;
            tile.type = stg::Land;
            bool left = false;
            bool right = false;
            bool top = false;
            bool bot = false;
            if(j==0) {
                top = true;
            }
            if (i==0) {
                left = true;
            }
            if (i==9) {
                right = true;
            }
            if (j==9) {
                bot = true;
            }
            if(left && top) {
                tile.sprite = gf::Sprite(land_top_left_edge);
            }
            else if(left && bot) {
                tile.sprite = gf::Sprite(land_bot_left_edge);
            }
            else if(right && top) {
                tile.sprite = gf::Sprite(land_top_right_edge);
            }
            else if(right && bot) {
                tile.sprite = gf::Sprite(land_bot_right_edge);
            }
            else if(left) {
                tile.sprite = gf::Sprite(land_left_side);
            }
            else if(right) {
                tile.sprite = gf::Sprite(land_right_side);
            }
            else if(top) {
                tile.sprite = gf::Sprite(land_top_side);
            }
            else if(bot) {
                tile.sprite = gf::Sprite(land_bot_side);
            }
            else {
                tile.sprite = gf::Sprite(land);
            }
            tile.sprite.setPosition(gf::Vector2i(i * 64,j * 64));
            row.push_back(tile);
        }
        board.board.push_back(row);
    }

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

        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                renderer.draw(board.board[i][j].sprite);
            }
        }

        for(int i = 0; i < 10; i++) {
            for(int j = 0; j < 4; j++) {
                gf::Sprite sprite;
                sprite.setTexture(base_piece_blue);
                sprite.setPosition(gf::Vector2i(i * 64,j * 64));
                renderer.draw(sprite);
            }
        }

        for(int i = 0; i < 10; i++) {
            for(int j = 6; j < 10; j++) {
                gf::Sprite sprite;
                sprite.setTexture(base_piece_red);
                sprite.setPosition(gf::Vector2i(i * 64,j * 64));
                renderer.draw(sprite);
            }
        }

        renderer.display();
    }

    return 0;
}

