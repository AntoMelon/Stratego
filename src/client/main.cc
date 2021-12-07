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

    gf::Texture land = gf::Texture(gf::Path("./resources/field.png"));
    gf::Texture land_top_side = gf::Texture(gf::Path("./resources/field_top.png"));
    gf::Texture land_bot_side = gf::Texture(gf::Path("./resources/field_bottom.png"));   //TODO : Poser la question d'un moyen plus efficace
    gf::Texture land_right_side = gf::Texture(gf::Path("./resources/field_right.png"));
    gf::Texture land_left_side = gf::Texture(gf::Path("./resources/field_left.png"));
    gf::Texture land_top_left_edge = gf::Texture(gf::Path("./resources/field_top_left.png"));
    gf::Texture land_top_right_edge = gf::Texture(gf::Path("./resources/field_top_right.png"));
    gf::Texture land_bot_left_edge = gf::Texture(gf::Path("./resources/field_bottom_left.png"));
    gf::Texture land_bot_right_edge = gf::Texture(gf::Path("./resources/field_bottom_right.png"));

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
            if(i==0) {
                top = true;
            }
            if (j==0) {
                left = true;
            }
            if (j==9) {
                right = true;
            }
            if (i==9) {
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
