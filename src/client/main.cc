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

/*TODO:
 *  animation duel
 *  message "client quitte"
 *  ajouter filtre sur cases atteignable
 *  scene avec gf ???
 *  prochain rdv: mercredi 16 mars 9h30
 */

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

    if ((selection.x < COORD_MIN) || (selection.x > COORD_MAX) || (selection.y < COORD_MIN) || (selection.y > COORD_MAX)) return gf::Vector2i(-1, -1);

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
    std::cout <<"envoie du coup: " << move.from_x << ", " << move.from_y << ", " << move.to_x << ", " << move.to_y << ", " << move.color << std::endl;
    gf::Packet to_send;
    to_send.is(move);
    socket.sendPacket(to_send);
}

struct Sprite_pair { gf::Sprite sprite_1 = gf::Sprite(); gf::Sprite sprite_2 = gf::Sprite(); };

/*
 * main function
 * simulate a game
 */
int main(int argc, char* argv[]) {

    gf::Queue<gf::Packet> serverPackets;
    gf::TcpSocket socket_client(argc > 1 ? argv[1] : HOST, argc > 2 ? argv[2] : PORT);
    if (!socket_client) {
        std::cerr << "Connection to server couldn't be established" << std::endl;
        return 1;
    }

    stg::Board board;
    bool myTurn(false);
    bool animate(false);
    Sprite_pair sprites_anim;
    stg::Color myColor(stg::Color::BLUE);
    stg::PLAYING_STATE state(stg::PLAYING_STATE::CONNEXION);

    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach();

    sendFirstMessage(socket_client);

    static constexpr gf::Vector2i ScreenSize(860, 680);
    gf::Window window("Stratego online", ScreenSize);
    gf::RenderWindow renderer(window);

    gf::RectF world = gf::RectF::fromSize({760, 680}); //monde du jeu
    gf::RectF extendedWorld = gf::RectF::fromSize(ScreenSize); //fenêtre

    gf::Texture T_drag;
    gf::Texture T_end_screen;
    gf::Texture T_hud_rules("resources/hud_rules.png");
    gf::Texture T_waiting_screen("resources/waiting.png");
    gf::Texture T_starting_button("resources/play_button.png");

    gf::Sprite S_end_screen(T_end_screen);
    gf::Sprite S_hud_rules(T_hud_rules);
    gf::Sprite S_waiting_screen(T_waiting_screen);
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

    gf::Sprite sprite_selected;
    bool mouse_pressed = false;

    background.setColor(gf::Color::Black);
    extendedBackground.setColor(gf::Color::Gray());

    gf::Font font("resources/arial.ttf");
    gf::Text txt("Message", font, 20);
    txt.setColor(gf::Color::White);
    txt.setPosition({0, 660});

    sprites_anim.sprite_1.setPosition({0 , -64});
    sprites_anim.sprite_2.setPosition({0 , 680});

    zone_to_place.setSize({636, 252});

    zone_to_place.setColor(gf::Color::Transparent);

    zone_to_place.setOutlineThickness(2);

    S_hud_rules.setPosition({642,0});
    zone_to_place.setPosition({2, 386});
    S_starting_button.setPosition({0, 0});

    gf::Event event;
    gf::Packet communication;
    gf::Vector2i selected(-1,-1);
    gf::Vector2i mouse_click(selected);
    gf::Vector2i mouse_position(selected);

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
                    if (animate == true) break;
                    if (event.mouseButton.button == gf::MouseButton::Left) {
                        mouse_click = event.mouseButton.coords;
                        switch (state) {

                            case stg::PLAYING_STATE::PLACEMENT:
                                mouse_pressed = true;
                                if ((mouse_click.x < 128) && (mouse_click.y < 26)) { // send the first board
                                    sendFirstBoard(board.getAllPiece(), myColor, socket_client);
                                } else {
                                    if (selected == gf::Vector2i(-1, -1)) { // if no click on memory -> put coords in memory
                                        selected = select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView));
                                        if(selected == gf::Vector2i(-1, -1)) {
                                            break;
                                        }
                                        if ((selected != gf::Vector2i(-1, -1)) && (board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::NONE)) selected = gf::Vector2i(-1, -1); //if click on a cell with no piece
                                    }
                                    if(selected != gf::Vector2i(-1, -1)) { // if click on memory -> send the move
                                        board.getPiece(selected.x, selected.y).display = false;
                                    }
                                }
                                break;

                            case stg::PLAYING_STATE::IN_GAME:
                                mouse_pressed = true;
                                if (selected == gf::Vector2i(-1, -1)) { // if no click on memory -> put coords in memory
                                    selected = select_on_board(renderer.mapPixelToCoords(mouse_click, *currentView));
                                    if(selected == gf::Vector2i(-1, -1)) {
                                        break;
                                    }
                                    board.getPiece(selected.x, selected.y).setDisplay(false);
                                    if ((selected != gf::Vector2i(-1, -1))
                                    && ((board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::NONE)
                                    || (board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::DRAPEAU)
                                    || (board.getPiece(selected.x, selected.y).getPieceName() == stg::PieceName::BOMBE))) {
                                        board.getPiece(selected.x, selected.y).setDisplay(true);
                                        selected = gf::Vector2i(-1, -1); //if click on a cell with no piece
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    break;

                case gf::EventType::MouseMoved:
                    if (animate == true) break;
                    mouse_position = event.mouseCursor.coords;
                    break;

                case gf::EventType::MouseButtonReleased: {
                    if (animate == true) break;
                    if ((mouse_click.x < 128) && (mouse_click.y < 26)) {
                        break;
                    }
                    if(selected == gf::Vector2i(-1, -1)) {
                        break;
                    }
                    board.getPiece(selected.x, selected.y).setDisplay(true);
                    switch (state) {
                        case stg::PLAYING_STATE::PLACEMENT: {
                            auto click_coord = select_on_board(renderer.mapPixelToCoords(mouse_position, *currentView));
                            if (click_coord == gf::Vector2i(-1, -1)) {
                                mouse_pressed = false;
                                selected = gf::Vector2i(-1, -1);
                                break;
                            }
                            if (board.getPiece(click_coord.x, click_coord.y).getPieceName() !=
                                stg::PieceName::NONE) { // if click on a cell with a piece, then swap piece
                                board.swapPiece(selected, click_coord);
                                selected = gf::Vector2i(-1, -1);
                            } else { // if click on a cell with no piece, then move piece
                                board.movePiece(selected, click_coord);
                                selected = gf::Vector2i(-1, -1);
                            }
                            mouse_pressed = false;
                            break;
                        }

                        case stg::PLAYING_STATE::IN_GAME:
                            mouse_pressed = false;
                            sendMove(
                                    {selected.x, selected.y, select_on_board(renderer.mapPixelToCoords(mouse_position, *currentView)).x,
                                     select_on_board(renderer.mapPixelToCoords(mouse_position, *currentView)).y, myColor},
                                    socket_client
                            );
                            selected = gf::Vector2i(-1, -1);
                            break;

                        default:
                            break;
                    }
                    break;
                }

                default:
                    break;
            }
            views.processEvent(event);
        }

        if(selected != gf::Vector2i(-1, -1) && mouse_pressed) {
            // If a piece is selected, then make it follow the mouse cursor
            auto piece = board.getPiece(selected.x, selected.y);
            auto texture_selected = board.getTexture(piece.getPieceName(), piece.getColor());
            sprite_selected.setTexture(board.manager.getTexture(gf::Path(texture_selected)));
            sprite_selected.setPosition(renderer.mapPixelToCoords(gf::Vector2i({mouse_position.x-SPRITE_SIZE/2,mouse_position.y-SPRITE_SIZE/2}), *currentView));
        }


        /*
         * Take message from server and all the things linked
         */
        if ((animate == false ) && (serverPackets.poll(communication))) {
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

                        case stg::ResponseCode::INFO:
                            txt.setString(com.message);
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
                    if (com.atk_alive && !com.def_alive) {
                        board.unsetPiece({com.to_x, com.to_y});
                        board.movePiece(gf::Vector2i({com.from_x, com.from_y}), gf::Vector2i({com.to_x, com.to_y}));

                        if (com.duel_occured) {
                            animate = true;
                            board.setPiece(com.to_x,com.to_y,{com.str_atk,com.color_atk});
                            sprites_anim.sprite_1.setTexture(gf::Texture("resources/" + board.getTexture(com.str_atk , com.color_atk)));
                            sprites_anim.sprite_2.setTexture(gf::Texture("resources/" + board.getTexture(com.str_def , com.color_def)));
                        }
                    } else if (!com.atk_alive && com.def_alive) {
                        board.unsetPiece({com.from_x, com.from_y});

                        if (com.duel_occured) {
                            animate = true;
                            board.setPiece(com.to_x,com.to_y,{com.str_def,com.color_def});
                            sprites_anim.sprite_1.setTexture(gf::Texture("resources/" + board.getTexture(com.str_atk , com.color_atk)));
                            sprites_anim.sprite_2.setTexture(gf::Texture("resources/" + board.getTexture(com.str_def , com.color_def)));
                        }
                    } else if (!com.atk_alive && !com.def_alive) {
                        animate = true;
                        board.unsetPiece({com.from_x,com.from_y});
                        board.unsetPiece({com.to_x,com.to_y});
                        sprites_anim.sprite_1.setTexture(gf::Texture("resources/" + board.getTexture(com.str_atk , com.color_atk)));
                        sprites_anim.sprite_2.setTexture(gf::Texture("resources/" + board.getTexture(com.str_def , com.color_def)));
                    }

                    if (com.win) {
                        state = stg::PLAYING_STATE::END;
                        if (myColor == stg::Color::BLUE) {
                            T_end_screen = gf::Texture("resources/blue_win.png");
                        } else {
                            T_end_screen = gf::Texture("resources/red_win.png");
                        }
                        S_end_screen = gf::Sprite(T_end_screen);
                        txt.setString("Vous avez gagné ! :)");
                    } else if (com.lose) {
                        state = stg::PLAYING_STATE::END;
                        if (myColor == stg::Color::BLUE) {
                            T_end_screen = gf::Texture("resources/red_win.png");
                        } else {
                            T_end_screen = gf::Texture("resources/blue_win.png");
                        }
                        S_end_screen = gf::Sprite(T_end_screen);
                        txt.setString("Vous avez perdu ! :(");
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
                            zone_to_place.setOutlineColor(gf::Color::Blue);
                        } else {
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

        if (animate == true) {
            sprites_anim.sprite_1.setPosition({0 , sprites_anim.sprite_1.getPosition().y + 2});
            sprites_anim.sprite_2.setPosition({0 , sprites_anim.sprite_2.getPosition().y - 2});

        }

        if (sprites_anim.sprite_1.getPosition().y > 320) {
            animate = false;
            sprites_anim.sprite_1.setPosition({0 , -64});
            sprites_anim.sprite_2.setPosition({0 , 680});
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
                renderer.draw(S_hud_rules);
                renderer.draw(txt);
                break;

            case stg::PLAYING_STATE::PLACEMENT:
            case stg::PLAYING_STATE::IN_GAME:
                board.render(renderer, currentView);
                renderer.draw(S_hud_rules);
                renderer.draw(txt);
                if (state == stg::PLAYING_STATE::PLACEMENT) renderer.draw(zone_to_place);
                std::cout << "Rendu" << std::endl;
                if (selected != gf::Vector2i({-1,-1})) {
                    renderer.draw(sprite_selected);
                }
                std::cout << "Fin rendu" << std::endl;
                renderer.draw(sprites_anim.sprite_1);
                renderer.draw(sprites_anim.sprite_2);
                renderer.setView(screenView);
                if (state == stg::PLAYING_STATE::PLACEMENT) renderer.draw(S_starting_button);
                break;

            case stg::PLAYING_STATE::END:
                renderer.draw(S_end_screen);
                renderer.draw(S_hud_rules);
                renderer.draw(txt);
                break;
            default:
                break;
        }
        
        renderer.setView(screenView);
        renderer.display();
    }

    return 0;
}


