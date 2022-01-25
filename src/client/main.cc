#include <iostream>
#include <thread>

#include "../common/protocol.h"
#include "./libs/board.h"

#include <gf/Queue.h>
#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/RenderTarget.h>
#include <gf/Event.h>
#include <gf/Color.h>
#include <gf/Sprite.h>
#include <gf/Shapes.h>
#include <gf/ViewContainer.h>
#include <gf/Views.h>


#define OFFSET_X 0
#define OFFSET_Y 150

//etape du jeu
enum PLAYING_STATE {CONNEXION, PLACEMENT, IN_GAME, END};

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

void sendFirstBoard(std::vector<stg::Piece> _board, stg::Color _color, gf::TcpSocket &_socket) {
    stg::ClientBoardSubmit firstBoard;
    firstBoard.color = _color;
    firstBoard.board = _board;
    gf::Packet packet_board;
    packet_board.is(firstBoard);
    _socket.sendPacket(packet_board);
}

void sendMove(gf::Vector2i _from, gf::Vector2i _to, gf::TcpSocket &_socket) {
    stg::ClientMoveRequest move;
    move.from_x = _from.x;
    move.from_y = _from.y;
    move.to_x = _to.x;
    move.to_y = _to.y;
    gf::Packet packet_move;
    packet_move.is(move);
    _socket.sendPacket(packet_move);
}



int main() {

    gf::Queue<gf::Packet> serverPackets;

    //paramètres définissant le joueur
    stg::Color myColor;
    PLAYING_STATE state = PLAYING_STATE::CONNEXION;
    bool myTurn = false;

    //parametre de socket
    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690");
    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach();
    stg::ClientHello data;
    data.name = "client";
    gf::Packet data2;
    data2.is(data);
    socket_client.sendPacket(data2);

    //Plateau de jeu
    stg::Board board;

    //boucle de connexion
    while (state == PLAYING_STATE::CONNEXION) {
        gf::Packet response;
        if (serverPackets.poll(response)) {
            switch(response.getType()) {
                case stg::ServerMessage::type:
                {
                    stg::ServerMessage msg = response.as<stg::ServerMessage>();
                    switch(msg.code) {
                        case stg::ResponseCode::WAITING:
                            std::cout << msg.message << std::endl;
                            break;
                        case stg::ResponseCode::STARTING:
                            std::cout << msg.message << std::endl;
                            state = PLAYING_STATE::PLACEMENT;
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case stg::ServerAssignColor::type: {
                    stg::ServerAssignColor assign = response.as<stg::ServerAssignColor>();
                    myColor = assign.color;
                    myTurn = assign.starting;
                    break;
                }
                default:
                    break;
            }
        }
    }

    //paramètre de la fenetre
    static constexpr gf::Vector2i ScreenSize(860, 640);
    gf::Window window("Stratego online", ScreenSize);
    gf::RenderWindow renderer(window);

    //monde de jeu
    gf::RectF world = gf::RectF::fromPositionSize({ 0, 0 }, {640, 640}); //monde du jeu
    gf::RectF extendedWorld = world.grow(100);

    //vue
    gf::ViewContainer views;
    gf::FitView fitView(world); //vue adaptée à la fenêtre
    views.addView(fitView);
    gf::ScreenView screenView; //vue de la fenêtre complète
    views.addView(screenView);
    views.setInitialFramebufferSize(ScreenSize);
    gf::AdaptativeView *currentView = &fitView; // vue concernée par les prochain changement
    gf::RectF maxiViewport = gf::RectF::fromPositionSize({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    //background
    gf::RectangleShape background(world);
    background.setColor(gf::Color::White);
    gf::RectangleShape extendedBackground(extendedWorld);
    extendedBackground.setColor(gf::Color::Gray());
    // hud
    gf::Texture T_starting_button("resources/play_button.png");
    gf::Sprite S_starting_button(T_starting_button);
    S_starting_button.setPosition({0, 0});
    gf::Texture T_Uplay;
    if( myColor == stg::Color::BLUE) {
        T_Uplay = gf::Texture("resources/uplay_blue.png");
    } else {
        T_Uplay = gf::Texture("resources/uplay_red.png");
    }
    gf::Sprite S_Uplay(T_Uplay);
    S_Uplay.setPosition({0, 100});
    //paramètrage des vues
    gf::RectangleShape frame(maxiViewport.getSize() * ScreenSize);
    frame.setPosition(maxiViewport.getPosition() * ScreenSize);
    frame.setColor(gf::Color::Transparent);
    //cadre de selection
    gf::Texture cadre_selection(gf::Path("resources/selected_indicator.png"));
    //zone de placement des pions
    gf::RectangleShape zone_to_place;
    zone_to_place.setSize({636, 252});
    zone_to_place.setColor(gf::Color::Transparent);
    zone_to_place.setOutlineThickness(2);

    //ajouter les pieces au plateau
    if (myColor == stg::Color::RED) {
        zone_to_place.setOutlineColor(gf::Color::Red);
        zone_to_place.setPosition({2, 384});
    } else {
        zone_to_place.setOutlineColor(gf::Color::Blue);
        zone_to_place.setPosition({2, 2});
    }
    board.setPieceFromColor(myColor);

    //game
    gf::Vector2i selected = gf::Vector2i(-1,-1);
    gf::Vector2i mouse_click = selected;

    while (window.isOpen()) {
        gf::Event event;
        gf::Packet communication;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case gf::EventType::Closed: //fermeture de fenêtre
                    window.close();
                    break;
                case gf::EventType::Resized: //redimmension de la fenêtre
                    if (window.getSize().x < 860) {
                        window.setSize(gf::Vector2i(840, window.getSize().y));
                    }
                    if (window.getSize().y < 640) {
                        window.setSize(gf::Vector2i(window.getSize().x, 640));
                    }
                    break;
                case gf::EventType::MouseButtonPressed: //bouton de souris
                    if (event.mouseButton.button == gf::MouseButton::Left) {
                        switch (state) {
                            case PLAYING_STATE::CONNEXION: //étape de connexion
                                break;
                            case PLAYING_STATE::PLACEMENT: //étape de placement
                                if ((event.mouseButton.coords.x < 128) && (event.mouseButton.coords.y < 26)) {
                                    sendFirstBoard(board.getAllPiece(), myColor, socket_client);
                                } else {
                                    mouse_click = event.mouseButton.coords;
                                    if (selected == gf::Vector2i(-1, -1)) {
                                        selected = renderer.mapPixelToCoords(mouse_click, *currentView);
                                        selected = gf::Vector2i(selected.x / 64, selected.y / 64);
                                        if ((selected.x < 0) || (selected.x > 9) || (selected.y < 0) || (selected.x > 9)) {
                                            selected = gf::Vector2i(-1, -1);
                                        }
                                    } else {
                                        board.movePiece(selected, gf::Vector2i(renderer.mapPixelToCoords(mouse_click, *currentView).x / 64, renderer.mapPixelToCoords(mouse_click, *currentView).y / 64));
                                        selected = gf::Vector2i(-1, -1);
                                    }
                                }
                                break;
                            case PLAYING_STATE::IN_GAME: //étape en jeu
                                mouse_click = event.mouseButton.coords;
                                if (selected == gf::Vector2i(-1, -1)) {
                                    selected = renderer.mapPixelToCoords(mouse_click, *currentView);
                                    selected = gf::Vector2i(selected.x / 64, selected.y / 64);
                                    if ((selected.x < 0) || (selected.x > 9) || (selected.y < 0) || (selected.x > 9)) {
                                        selected = gf::Vector2i(-1, -1);
                                    }
                                } else {
                                    sendMove(selected, gf::Vector2i(renderer.mapPixelToCoords(mouse_click, *currentView).x / 64, renderer.mapPixelToCoords(mouse_click, *currentView).y / 64), socket_client);
                                    selected = gf::Vector2i(-1, -1);
                                }
                                break;
                            case PLAYING_STATE::END: //étape de partie finie
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            views.processEvent(event);
        }
        if (serverPackets.poll(communication)) { //reception de packets
            switch(communication.getType()) {
                case stg::ServerMessage::type:
                {
                    stg::ServerMessage com = communication.as<stg::ServerMessage>();
                    switch (com.code) {
                        case stg::ResponseCode::BOARD_OK: //plateau bon
                            std::cout << com.message << std::endl;
                            state = PLAYING_STATE::IN_GAME;
                            break;
                        case stg::ResponseCode::BOARD_ERR: //erreur dans le plateau
                            std::cout << com.message << std::endl;
                            state = PLAYING_STATE::PLACEMENT;
                            break;
                        case stg::ResponseCode::MOVE_ERR: //problème dans le mouvement
                            std::cout << com.message << std::endl;
                            break;
                        case stg::ResponseCode::STARTING:
                        case stg::ResponseCode::WAITING:
                        default:
                            std::cout << "Information reçue non-reconnue ou non-permise en jeu." << std::endl;
                            break;
                    }
                    break;
                }
                case stg::ServerMoveNotif::type: //mouvement effectué par un joueur
                {
                    stg::ServerMoveNotif com = communication.as<stg::ServerMoveNotif>();
                    board.movePiece(gf::Vector2i({com.from_x, com.from_y}), gf::Vector2i({com.to_x, com.to_y}));
                    myTurn = !myTurn;
                    break;
                }
                case stg::ServerAssignColor::type: // événements non pris en compte durant ce stade de jeu
                default:
                    std::cout << "Information reçue non-reconnue ou non-permise en jeu." << std::endl;
                    break;
            }
        }

        if (state == PLAYING_STATE::PLACEMENT || state == PLAYING_STATE::IN_GAME) { // rendu graphique

            renderer.clear();

            //afficher dans monde
            currentView->setViewport(maxiViewport);
            renderer.setView(*currentView);
            renderer.draw(extendedBackground);
            renderer.draw(background);
            board.render(renderer, currentView);
            if (state == PLAYING_STATE::PLACEMENT) {
                renderer.draw(zone_to_place);
            }
            gf::RectI viewport = renderer.getViewport(*currentView);
            frame.setPosition(viewport.getPosition());
            frame.setSize(viewport.getSize());

            //afficher dans la fenêtre
            renderer.setView(screenView);
            renderer.draw(frame);
            if (state == PLAYING_STATE::PLACEMENT) {
                renderer.draw(S_starting_button);
            }
            renderer.draw(S_Uplay);
            renderer.display();
        }

    }

    return 0;
}


