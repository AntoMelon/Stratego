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

#define OFFSET_X 0
#define OFFSET_Y 150

// dafont + possibilité de licence cc
// TODO: selection de case cible avec relevé de bouton de souris (drag and drop)
// TODO: ajouter sleep dans le serveur
// TODO: virer les underscore
// TODO: fromSize dans doc pour éviter {0, 0}
// scene ?
// ref -> git sokoban
// TODO: retirer viewPort -> doc SFML

/*
 * different stage of the game
 */
enum PLAYING_STATE {CONNEXION, PLACEMENT, IN_GAME, END};

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

gf::Vector2i select_on_board(gf::Vector2i _selection) {

    _selection.x = _selection.x / SPRITE_SIZE;
    _selection.y = _selection.y / SPRITE_SIZE;

    if ((_selection.x < 0) || (_selection.x > 9) || (_selection.y < 0) || (_selection.x > 9)) return gf::Vector2i(-1, -1);

    return _selection;

}

/*
 * send the first message to the server
 */
void sendFirstMessage(gf::TcpSocket &_socket) {
    stg::ClientHello data;
    data.name = "client";
    gf::Packet data2;
    data2.is(data);
    _socket.sendPacket(data2);
}

/*
 * send first board to the server
 */
void sendFirstBoard(std::vector<stg::Piece> _board, stg::Color _color, gf::TcpSocket &_socket) {
    stg::ClientBoardSubmit firstBoard;
    firstBoard.color = _color;
    firstBoard.board = _board;
    gf::Packet packet_board;
    packet_board.is(firstBoard);
    _socket.sendPacket(packet_board);
    std::cout << "Should send board" << std::endl;
}

/*
 * send a try of move to the server
 */
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

/*
 * main function
 * simulate a game
 */
int main() {

    gf::Queue<gf::Packet> serverPackets;
    gf::TcpSocket socket_client = gf::TcpSocket(HOST, PORT);
    if (!socket_client) {
        std::cerr << "Connection to server couldn't be established" << std::endl;
        return 1;
    }

    stg::Board board;
    bool myTurn = false;
    stg::Color myColor = stg::Color::BLUE;
    PLAYING_STATE state = PLAYING_STATE::CONNEXION;

    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach();

    sendFirstMessage(socket_client);

    static constexpr gf::Vector2i ScreenSize(860, 680);
    gf::Window window("Stratego online", ScreenSize);
    gf::RenderWindow renderer(window);

    gf::RectF world = gf::RectF::fromPositionSize({ 0, 0 }, {640, 680}); //monde du jeu
    gf::RectF extendedWorld = world.grow(100);

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
    gf::RectF maxiViewport = gf::RectF::fromPositionSize({ 0.0f, 0.0f }, { 1.0f, 1.0f });

    gf::RectangleShape zone_to_place;
    gf::RectangleShape background(world);
    gf::RectangleShape extendedBackground(extendedWorld);
    gf::RectangleShape frame(maxiViewport.getSize() * ScreenSize);

    background.setColor(gf::Color::Black);
    frame.setColor(gf::Color::Transparent);
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
    frame.setPosition(maxiViewport.getPosition() * ScreenSize);

    gf::Event event;
    gf::Packet communication;
    gf::Vector2i selected = gf::Vector2i(-1,-1);
    gf::Vector2i mouse_click = selected;

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

                            case PLAYING_STATE::PLACEMENT:
                                if ((mouse_click.x < 128) && (mouse_click.y < 26)) { // send the first board
                                    sendFirstBoard(board.getAllPiece(), myColor, socket_client);
                                } else {
                                    if (selected == gf::Vector2i(-1, -1)) { // if no click on memory -> put coords in memory
                                        selected = select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView));
                                        if ((selected != gf::Vector2i(-1, -1)) && (board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::NONE)) selected = gf::Vector2i(-1, -1); //if click on a cell with no piece
                                    } else { // if click on memory -> move piece on board
                                        board.movePiece(selected, select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView)));
                                        selected = gf::Vector2i(-1, -1);
                                    }
                                }
                                break;

                            case PLAYING_STATE::IN_GAME:
                                if (selected == gf::Vector2i(-1, -1)) { // if no click on memory -> put coords in memory
                                    selected = select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView));
                                    if ((selected != gf::Vector2i(-1, -1)) && (board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::NONE)) selected = gf::Vector2i(-1, -1); //if click on a cell with no piece
                                } else { // send move to the server
                                    sendMove(selected, select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView)), socket_client);
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
                            state = PLAYING_STATE::IN_GAME;
                            break;

                        case stg::ResponseCode::BOARD_ERR:
                            txt.setString(com.message);
                            state = PLAYING_STATE::PLACEMENT;
                            break;

                        case stg::ResponseCode::MOVE_ERR:
                            txt.setString(com.message);
                            break;

                        case stg::ResponseCode::STARTING:
                            if (state == PLAYING_STATE::CONNEXION) {
                                txt.setString(com.message);
                                state = PLAYING_STATE::PLACEMENT;
                            }
                            break;

                        case stg::ResponseCode::WAITING:
                            if (state == PLAYING_STATE::CONNEXION) {
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
                    board.movePiece(gf::Vector2i({com.from_x, com.from_y}), gf::Vector2i({com.to_x, com.to_y}));
                    myTurn = !myTurn;
                    break;
                }

                case stg::ServerAssignColor::type: // assign a color to the client (only one time)
                {
                    if (state == PLAYING_STATE::CONNEXION) {
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
        currentView->setViewport(maxiViewport);
        renderer.setView(*currentView);
        renderer.draw(extendedBackground);
        renderer.draw(background);

        gf::RectI viewport;

        switch(state) {
            case PLAYING_STATE::CONNEXION:
                renderer.draw(S_waiting_screen);
                renderer.draw(txt);
                break;

            case PLAYING_STATE::PLACEMENT:
                board.render(renderer, currentView);
                renderer.draw(zone_to_place);
                renderer.draw(txt);
                if (selected != gf::Vector2i({-1,-1})) {
                    S_selected_box.setPosition(gf::Vector2i({selected.x * SPRITE_SIZE, selected.y * SPRITE_SIZE}));
                    renderer.draw(S_selected_box);
                }
                viewport = renderer.getViewport(*currentView);
                frame.setPosition(viewport.getPosition());
                frame.setSize(viewport.getSize());
                renderer.setView(screenView);
                renderer.draw(frame);
                renderer.draw(S_starting_button);
                renderer.draw(S_Uplay);
                break;

            case PLAYING_STATE::IN_GAME:
                board.render(renderer, currentView);
                renderer.draw(txt);
                if (selected != gf::Vector2i({-1,-1})) {
                    S_selected_box.setPosition(gf::Vector2i({selected.x*SPRITE_SIZE, selected.y*SPRITE_SIZE}));
                    renderer.draw(S_selected_box);
                }
                viewport = renderer.getViewport(*currentView);
                frame.setPosition(viewport.getPosition());
                frame.setSize(viewport.getSize());
                renderer.setView(screenView);
                renderer.draw(frame);
                renderer.draw(S_Uplay);
                break;

            case PLAYING_STATE::END:
            default:
                break;
        }

        renderer.display();
    }

    return 0;
}


