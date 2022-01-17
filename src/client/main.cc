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

enum PLAYING_STATE {
    CONNEXION,
    PLACEMENT,
    IN_GAME,
    END
};

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


int main() {

    gf::Queue<gf::Packet> serverPackets;

    stg::Color myColor;
    PLAYING_STATE state = PLAYING_STATE::CONNEXION;
    bool myTurn = false;

    static constexpr gf::Vector2i ScreenSize(860, 640);

    gf::Window window("Stratego online", ScreenSize);
    gf::RenderWindow renderer(window);
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
    // future bouton aide/règles
    gf::Texture hud("src/client/resources/play_button.png");
    gf::Sprite hud_s(hud);
    hud_s.setPosition({0, 0});
    //paramètrage des vues
    gf::RectangleShape frame(maxiViewport.getSize() * ScreenSize);
    frame.setPosition(maxiViewport.getPosition() * ScreenSize);
    frame.setColor(gf::Color::Transparent);
    renderer.clear(gf::Color::Black);

    stg::Board board;
    //gf::Texture cadre_selection(gf::Path("./resources/selected_indicator.png"));

    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690"); //parametre de connxeion

    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach(); // indépendance thread/execution

    stg::ClientHello data; //ressources à envoyer
    data.name = "client"; //parametre du nom

    gf::Packet data2; //créer packet
    data2.is(data); // serialiser packet

    socket_client.sendPacket(data2); //envoie du packet

    //zone de placement des pions
    gf::RectangleShape zone_to_place;
    zone_to_place.setSize({636, 252});
    zone_to_place.setColor(gf::Color::Transparent);
    zone_to_place.setOutlineThickness(2);

    while (state == PLAYING_STATE::CONNEXION) { //en attente du début de partie

        gf::Packet response; //packet reponse

        if (serverPackets.poll(response)) { //si packet reçu
            switch(response.getType()) { //e fonction du type
                case stg::ServerMessage::type: //message
                {
                    stg::ServerMessage msg = response.as<stg::ServerMessage>(); //deserialiser

                    switch(msg.code) { //code reçu
                        case stg::ResponseCode::WAITING: //en attente
                            std::cout << msg.message << std::endl; //afficher en attente
                            break;

                        case stg::ResponseCode::STARTING: //commencer
                            std::cout << msg.message << std::endl; //afficher debut
                            state = PLAYING_STATE::PLACEMENT; //jeu commencer
                            break;

                        default:
                            break;
                    }

                    break;
                }

                case stg::ServerAssignColor::type:
                {
                    stg::ServerAssignColor assign = response.as<stg::ServerAssignColor>();
                    myColor = assign.color;
                    myTurn = assign.starting;

                    //std::cout << "You " << (myTurn ? "start " : "don't start ") << "and you are player " << myColor << std::endl;
                    break;
                }

                default:
                    break;
            }
        }
    }

    //set fore placement
    int x = 0, y = 0;
    if (myColor == stg::Color::RED) {
        zone_to_place.setOutlineColor(gf::Color::Red);
        zone_to_place.setPosition({2, 384});
    } else {
        zone_to_place.setOutlineColor(gf::Color::Blue);
        zone_to_place.setPosition({2, 2});
        y = 6;
    }
    while (x < 8) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::ECLAIREUR,myColor));
        ++x;
    }
    while (x < 10) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::COLONEL,myColor));
        ++x;
    }
    x = 0;
    ++y;
    while (x < 6) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::BOMBE,myColor));
        ++x;
    }
    while (x < 10) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::SERGENT,myColor));
        ++x;
    }
    x = 0;
    ++y;
    while (x < 5) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::DEMINEUR,myColor));
        ++x;
    }
    while (x < 9) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::LIEUTENANT,myColor));
        ++x;
    }
    board.setPiece(x,y,stg::Piece(stg::PieceName::MARECHAL,myColor));
    x = 0;
    ++y;
    while (x < 4) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::CAPITAINE,myColor));
        ++x;
    }
    while (x < 7) {
        board.setPiece(x,y,stg::Piece(stg::PieceName::COMMANDANT,myColor));
        ++x;
    }
    board.setPiece(x,y,stg::Piece(stg::PieceName::GENERAL,myColor));
    ++x;
    board.setPiece(x,y,stg::Piece(stg::PieceName::ESPION,myColor));
    ++x;
    board.setPiece(x,y,stg::Piece(stg::PieceName::DRAPEAU,myColor));

    //game
    gf::Vector2i selected = gf::Vector2i(-1,-1);
    gf::Vector2i mouse_click = selected;

    while (window.isOpen()) {
        gf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case gf::EventType::Closed:
                    window.close();
                    break;
                case gf::EventType::Resized:
                    if (window.getSize().x < 860) {
                        window.setSize(gf::Vector2i(840, window.getSize().y));
                    }
                    if (window.getSize().y < 640) {
                        window.setSize(gf::Vector2i(window.getSize().x, 640));
                    }
                    break;
                case gf::EventType::MouseButtonPressed:
                    if (event.mouseButton.button == gf::MouseButton::Left) {
                        mouse_click = event.mouseButton.coords;
                        if(selected == gf::Vector2i(-1,-1)) {
                            selected = renderer.mapPixelToCoords(mouse_click, *currentView);
                            selected = gf::Vector2i(selected.x/64, selected.y/64);
                        } else {
                            board.movePiece(selected, gf::Vector2i(renderer.mapPixelToCoords(mouse_click, *currentView).x/64, renderer.mapPixelToCoords(mouse_click, *currentView).y/64));
                            selected = gf::Vector2i(-1,-1);
                        }
                    }
                    if (state == PLAYING_STATE::PLACEMENT) {
                        if (event.mouseButton.button == gf::MouseButton::Left) {
                            if ((event.mouseButton.coords.x < 128) && (event.mouseButton.coords.y < 26)) {
                                std::cout << "click" << std::endl;
                                stg::ClientBoardSubmit firstBorad; //ressources à envoyer
                                firstBorad.color = myColor; //parametre du nom
                                std::cout << "ressource ok" << std::endl;
                                gf::Packet packet_board; //créer packet
                                packet_board.is(firstBorad); // serialiser packet
                                std::cout << "packet ok" << std::endl;
                                socket_client.sendPacket(packet_board); //envoie du packet
                                std::cout << "envoyer!" << std::endl;
                            }
                        }
                        state = IN_GAME;
                    }
                    break;
                default:
                    break;
            }

            views.processEvent(event);
        }

        if (state == PLAYING_STATE::PLACEMENT || state == PLAYING_STATE::IN_GAME) {

            renderer.clear();

            //gestion des vues
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
                renderer.draw(hud_s);
            }

            /*if(selected != gf::Vector2i(-1,-1)) {
                gf::Sprite sprite;
                sprite.setTexture(cadre_selection);
                sprite.setPosition(gf::Vector2i(selected.x*64,selected.y*64));
                renderer.draw(sprite);
            }*/

            renderer.display();

        }

    }

    return 0;
}


