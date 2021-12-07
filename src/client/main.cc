#include <iostream>

//#include "../common/protocol.h"
#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Event.h>
#include <gf/Color.h>
#include <gf/Sprite.h>

namespace stg {

    enum TileType{
        Land,
        River
    };

    struct Tile {
        TileType type;
        gf::Sprite sprite;
    };

    struct GraphicBoard {
        std::vector<std::vector<Tile>> board;
    };

}

int main() {

    gf::Vector2i windowSize(640, 640);
    gf::Window window("Stratego en réseau", windowSize);
    gf::RenderWindow renderer(window);

    gf::Texture texture_land = gf::Texture(gf::Path("./resources/field.png"));

    stg::GraphicBoard board;
    for (int i = 0; i < 10; i++) {
        std::vector<stg::Tile> row;
        for (int j = 0; j < 10; j++) {
            stg::Tile tile;
            tile.type = stg::Land;
            tile.sprite.setTexture(texture_land);
            tile.sprite.setPosition(gf::Vector2i(i * 64,j * 64));
            row.push_back(tile);
        }
        board.board.push_back(row);
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

        renderer.display();
    }

    return 0;
}
