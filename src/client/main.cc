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
#include <gf/Shapes.h>
#include <gf/ViewContainer.h>
#include <gf/Views.h>


#define OFFSET_X 0
#define OFFSET_Y 150

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
    bool myTurn;

    static constexpr gf::Vector2i ScreenSize(640, 640);

    gf::Window window("Stratego online", ScreenSize);
    gf::RenderWindow renderer(window);
    gf::RectF world = gf::RectF::fromPositionSize({ 0, 0 }, { 640, 640}); //monde du jeu
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
    gf::Texture hud("src/client/resources/help.png");
    gf::Sprite hud_s(hud);
    hud_s.setPosition({0, 0});
    //paramètrage des vues
    gf::RectangleShape frame(maxiViewport.getSize() * ScreenSize);
    frame.setPosition(maxiViewport.getPosition() * ScreenSize);
    frame.setColor(gf::Color::Transparent);
    frame.setOutlineColor(gf::Color::Red);
    frame.setOutlineThickness(2.0f);
    renderer.clear(gf::Color::Black);

    stg::Board board;
    //gf::Texture cadre_selection(gf::Path("./resources/selected_indicator.png"));

    gf::TcpSocket socket_client = gf::TcpSocket("localhost", "42690"); //parametre de connxeion
    bool inGame = false; //booleen jeu

    std::thread packetsThread(threadPackets,std::ref(socket_client),std::ref(serverPackets));
    packetsThread.detach(); // indépendance thread/execution

    stg::ClientHello data; //ressources à envoyer
    data.name = "client"; //parametre du nom

    gf::Packet data2; //créer packet
    data2.is(data); // serialiser packet

    socket_client.sendPacket(data2); //envoie du packet

    while (!inGame) { //en jeu
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
                            inGame = true; //jeu commencer
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

                    std::cout << "You " << (myTurn ? "start " : "don't start ") << "and you are player " << myColor << std::endl;
                    break;
                }

                default:
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

    gf::Vector2i selected = gf::Vector2i(-1,-1);

    while (window.isOpen()) {
        gf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case gf::EventType::Closed:
                    window.close();
                    break;
                case gf::EventType::MouseButtonPressed:
                    if (event.mouseButton.button == gf::MouseButton::Left) {
                        if(selected == gf::Vector2i(-1,-1)) {
                            selected = board.convert_mouse_coord_to_case(event.mouseButton.coords.x, event.mouseButton.coords.y, window.getSize().x, window.getSize().y);
                        } else {
                            board.movePiece(selected, board.convert_mouse_coord_to_case(event.mouseButton.coords.x, event.mouseButton.coords.y, window.getSize().x, window.getSize().y));
                            selected = gf::Vector2i(-1,-1);
                        }
                    }
                    break;
                default:
                    break;
            }

            views.processEvent(event);
        }

        renderer.clear();

        //gestion des vues
        currentView->setViewport(maxiViewport);
        renderer.setView(*currentView);
        renderer.draw(extendedBackground);
        renderer.draw(background);
        board.render(renderer, currentView);
        gf::RectI viewport = renderer.getViewport(*currentView);
        frame.setPosition(viewport.getPosition());
        frame.setSize(viewport.getSize());

        //afficher dans la fenêtre
        renderer.setView(screenView);
        renderer.draw(frame);
        renderer.draw(hud_s);

        /*if(selected != gf::Vector2i(-1,-1)) {
            gf::Sprite sprite;
            sprite.setTexture(cadre_selection);
            sprite.setPosition(gf::Vector2i(selected.x*64,selected.y*64));
            renderer.draw(sprite);
        }*/

        renderer.display();

    }

    return 0;
}


