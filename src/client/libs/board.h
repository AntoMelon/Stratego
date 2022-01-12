#ifndef STGCLIENT_BOARD_H
#define STGCLIENT_BOARD_H

#include <vector>
#include "../../common/piece.h"
#include <gf/RenderWindow.h>
#include <gf/Vector.h>
#include <gf/Sprite.h>
#include <gf/Texture.h>
#include <gf/Path.h>
#include <gf/ResourceManager.h>
#include <gf/ViewContainer.h>
#include <gf/Views.h>

namespace stg {

    enum TileType{
        Land,
        River
    };

    class Square {
    private:
        TileType type;
        bool walkable;
    public:
        Square(TileType type, bool walkable);
        TileType getType();
        bool isWalkable();
    };

    class Board {
    public:

        // Default Board Generator
        Board();

        // Get the square at the given coordinates
        Square getSquare(int x, int y);

        // Get the piece at the given coordinates
        Piece getPiece(int x, int y);

        // Set the piece at the given coordinates
        void setPiece(int x, int y, Piece piece);

        //Render the board
        void render(gf::RenderWindow &renderer, gf::AdaptativeView* view);

        gf::Vector2i convert_mouse_coord_to_case(int mouse_x, int mouse_y, int window_x, int window_y);

        std::pair<std::string, bool> movePiece(gf::Vector2i from, gf::Vector2i to);

    private:
        std::vector<std::vector<std::pair<Square,stg::Piece>>> board;
        std::map<std::pair<stg::PieceName, stg::Color>, std::string> pieceTextures;
        std::map<std::string, std::string> tileTextures;
        gf::ResourceManager manager;
    };

}

#endif //STGCLIENT_BOARD_H
