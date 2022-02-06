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
#include <gf/Text.h>

/*
 * Packet thread
 */
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

gf::Vector2i select_on_board(gf::Vector2i selection) {

    selection = gf::Vector2i(selection.x / SPRITE_SIZE, selection.y / SPRITE_SIZE);

    if ((selection.x < COORD_MIN) || (selection.x > COORD_MAX) || (selection.y < COORD_MIN) || (selection.x > COORD_MAX)) return gf::Vector2i(-1, -1);

    if (((selection.y == 4) || (selection.y == 5)) && ((selection.x == 2) || (selection.x == 3) || (selection.x == 6) || (selection.x == 7))) return gf::Vector2i(-1, -1);

    return selection;

}

/*
 * send the first message to the server
 */
void sendFirstMessage(gf::TcpSocket &socket) {
    stg::ClientHello data({"client"});
    gf::Packet data2;
    data2.is(data);
    socket.sendPacket(data2);
}

/*
 * send first board to the server
 */
void sendFirstBoard(std::vector<stg::Piece> board, stg::Color color, gf::TcpSocket &socket) {
    stg::ClientBoardSubmit firstBoard({color, board});
    gf::Packet packet_board;
    packet_board.is(firstBoard);
    socket.sendPacket(packet_board);
}

/*
 * send a try of move to the server
 */
void sendMove(stg::ClientMoveRequest move, gf::TcpSocket &socket) {
    gf::Packet to_send;
    to_send.is(move);
    socket.sendPacket(to_send);
}

/*
 * main function
 * simulate a game
 */
int main() {

    gf::Queue<gf::Packet> serverPackets;
    gf::TcpSocket socket_client(HOST, PORT);
    if (!socket_client) {
        std::cerr << "Connection to server couldn't be established" << std::endl;
        return 1;
    }

    stg::Board board;
    bool myTurn(false);
    stg::Color myColor(stg::Color::BLUE);
    stg::PLAYING_STATE state(stg::PLAYING_STATE::CONNEXION);

    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach();

    sendFirstMessage(socket_client);

    static constexpr gf::Vector2i ScreenSize(860, 680);
    gf::Window window("Stratego online", ScreenSize);
    gf::RenderWindow renderer(window);

    gf::RectF world = gf::RectF::fromSize({640, 680}); //monde du jeu
    gf::RectF extendedWorld = gf::RectF::fromSize(ScreenSize);

    gf::Texture T_drag;
    gf::Texture T_Uplay;
    gf::Texture T_waiting_screen("resources/waiting.png");
    gf::Texture T_starting_button("resources/play_button.png");
    gf::Texture T_cadre_selection(gf::Path("resources/selected_indicator.png"));

    gf::Sprite S_Uplay(T_Uplay);
    gf::Sprite S_waiting_screen(T_waiting_screen);
    gf::Sprite S_selected_box(T_cadre_selection);
    gf::Sprite S_starting_button(T_starting_button);

    gf::ViewContainer views;
    gf::FitView fitView(world);
    gf::ScreenView screenView;
    views.addView(fitView);
    views.addView(screenView);
    views.setInitialFramebufferSize(ScreenSize);
    gf::AdaptativeView *currentView = &fitView;

    gf::RectangleShape zone_to_place;
    gf::RectangleShape background(world);
    gf::RectangleShape extendedBackground(extendedWorld);

    background.setColor(gf::Color::Black);
    extendedBackground.setColor(gf::Color::Gray());

    gf::Font font("resources/arial.ttf");
    gf::Text txt("Message", font, 20);
    txt.setColor(gf::Color::White);
    txt.setPosition({0, 660});

    zone_to_place.setSize({636, 252});

    zone_to_place.setColor(gf::Color::Transparent);

    zone_to_place.setOutlineThickness(2);

    S_Uplay.setPosition({0, 100});
    zone_to_place.setPosition({2, 386});
    S_starting_button.setPosition({0, 0});

    gf::Event event;
    gf::Packet communication;
    gf::Vector2i selected(-1,-1);
    gf::Vector2i mouse_click(selected);

    /*
     * main loop
     * display windows and launch every function linked to an event or a call from the server
     */
    while (window.isOpen()) {
        /*
         * Take event from user and do all the things linked
         */
        while (window.pollEvent(event)) {
            switch (event.type) {

                case gf::EventType::Closed: // close the window
                    window.close();
                    break;

                case gf::EventType::KeyPressed:
                    switch (event.key.keycode) {
                        case gf::Keycode::F11:
                            window.toggleFullscreen();
                            break;
                        default:
                            break;
                    }
                    break;

                case gf::EventType::MouseButtonPressed: //if click on the window
                    if (event.mouseButton.button == gf::MouseButton::Left) {
                        mouse_click = event.mouseButton.coords;
                        switch (state) {

                            case stg::PLAYING_STATE::PLACEMENT:
                                if ((mouse_click.x < 128) && (mouse_click.y < 26)) { // send the first board
                                    sendFirstBoard(board.getAllPiece(), myColor, socket_client);
                                } else {
                                    if (selected == gf::Vector2i(-1, -1)) { // if no click on memory -> put coords in memory
                                        selected = select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView));
                                        if ((selected != gf::Vector2i(-1, -1)) && (board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::NONE)) selected = gf::Vector2i(-1, -1); //if click on a cell with no piece
                                    } else { // if click on memory -> move piece on board
                                        board.movePiece(selected, select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView)));
                                        txt.setString(board.getTexture(board.getPiece(selected.x, selected.y).getPieceName(), myColor));
                                        selected = gf::Vector2i(-1, -1);
                                    }
                                }
                                break;

                            case stg::PLAYING_STATE::IN_GAME:
                                if (selected == gf::Vector2i(-1, -1)) { // if no click on memory -> put coords in memory
                                    selected = select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView));
                                    if ((selected != gf::Vector2i(-1, -1)) && (board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::NONE)) selected = gf::Vector2i(-1, -1); //if click on a cell with no piece
                                } else { // send move to the server
                                    sendMove(
                                            {selected.x, selected.y, select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView)).x, select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView)).y, myColor},
                                            socket_client
                                            );
                                    selected = gf::Vector2i(-1, -1);
                                }
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

        /*
         * Take message from server and all the things linked
         */
        if (serverPackets.poll(communication)) {
            switch(communication.getType()) {

                case stg::ServerMessage::type: // take message in function of code
                {

                    stg::ServerMessage com = communication.as<stg::ServerMessage>();
                    switch (com.code) {

                        case stg::ResponseCode::BOARD_OK:
                            txt.setString(com.message);
                            if (myColor == stg::Color::BLUE) {
                                board.setOtherSide(stg::Color::RED);
                            } else {
                                board.setOtherSide(stg::Color::BLUE);
                            }
                            state = stg::PLAYING_STATE::IN_GAME;
                            break;

                        case stg::ResponseCode::BOARD_ERR:
                            txt.setString(com.message);
                            state = stg::PLAYING_STATE::PLACEMENT;
                            break;

                        case stg::ResponseCode::MOVE_ERR:
                            txt.setString(com.message);
                            break;

                        case stg::ResponseCode::STARTING:
                            if (state == stg::PLAYING_STATE::CONNEXION) {
                                txt.setString(com.message);
                                state = stg::PLAYING_STATE::PLACEMENT;
                            }
                            break;

                        case stg::ResponseCode::WAITING:
                            if (state == stg::PLAYING_STATE::CONNEXION) {
                                txt.setString(com.message);
                            }
                            break;

                        case stg::ResponseCode::NOT_YOUR_TURN:
                            if (state == stg::PLAYING_STATE::IN_GAME) {
                                txt.setString(com.message);
                            }
                        break;

                        default:
                            txt.setString("Information reçue non-reconnue ou non-permise en jeu.");
                            break;
                    }
                    break;
                }

                case stg::ServerMoveNotif::type: // if a client do a move
                {
                    stg::ServerMoveNotif com = communication.as<stg::ServerMoveNotif>();
                    std::cout << "Base: " << com.from_x << ";" << com.from_y << std::endl << "Cible: " << com.to_x << ";" << com.to_y << std::endl;
                    if (board.getPiece(com.to_x, com.to_y).getPieceName() != stg::PieceName::NONE){
                        board.movePiece(gf::Vector2i({com.from_x, com.from_y}), gf::Vector2i({com.to_x, com.to_y}));
                    } else {
                        if (com.atk_alive) {
                            board.unsetPiece({com.to_x, com.to_y});
                            board.movePiece(gf::Vector2i({com.from_x, com.from_y}), gf::Vector2i({com.to_x, com.to_y}));
                        }
                        if (com.def_alive) {
                            board.unsetPiece({com.from_x, com.from_y});
                        }
                    }
                    myTurn = !myTurn;
                    break;
                }

                case stg::ServerAssignColor::type: // assign a color to the client (only one time)
                {
                    if (state == stg::PLAYING_STATE::CONNEXION) {
                        stg::ServerAssignColor assign = communication.as<stg::ServerAssignColor>();
                        myColor = assign.color;
                        myTurn = assign.starting;
                        if( myColor == stg::Color::BLUE) {
                            T_Uplay = gf::Texture("resources/uplay_blue.png");
                            zone_to_place.setOutlineColor(gf::Color::Blue);
                        } else {
                            T_Uplay = gf::Texture("resources/uplay_red.png");
                            zone_to_place.setOutlineColor(gf::Color::Red);
                        }
                        board.setPieceFromColor(myColor);
                    }
                    break;
                }

                default:
                    txt.setString("Information reçue non-reconnue ou non-permise en jeu.");
                    break;
            }
        }

        /*
         * display all element in the window
         */
        renderer.clear();
        renderer.setView(*currentView);
        renderer.draw(extendedBackground);
        renderer.draw(background);

        switch(state) {
            case stg::PLAYING_STATE::CONNEXION:
                renderer.draw(S_waiting_screen);
                renderer.draw(txt);
                break;

            case stg::PLAYING_STATE::PLACEMENT:
                board.render(renderer, currentView);
                renderer.draw(zone_to_place);
                renderer.draw(txt);
                if (selected != gf::Vector2i({-1,-1})) {
                    S_selected_box.setPosition(gf::Vector2i({selected.x * SPRITE_SIZE, selected.y * SPRITE_SIZE}));
                    renderer.draw(S_selected_box);
                }
                renderer.setView(screenView);
                renderer.draw(S_starting_button);
                renderer.draw(S_Uplay);
                break;

            case stg::PLAYING_STATE::IN_GAME:
                board.render(renderer, currentView);
                renderer.draw(txt);
                if (selected != gf::Vector2i({-1,-1})) {
                    S_selected_box.setPosition(gf::Vector2i({selected.x*SPRITE_SIZE, selected.y*SPRITE_SIZE}));
                    renderer.draw(S_selected_box);
                }
                renderer.setView(screenView);
                renderer.draw(S_Uplay);
                break;

            case stg::PLAYING_STATE::END:
            default:
                break;
        }

        renderer.display();
    }

    return 0;
}


