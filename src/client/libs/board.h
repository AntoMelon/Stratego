#ifndef STGCLIENT_BOARD_H
#define STGCLIENT_BOARD_H

#include <string>
#include "../../common/piece.h"
#include <gf/RenderWindow.h>
#include <gf/Vector.h>
#include <gf/Sprite.h>
#include <gf/Texture.h>
#include <gf/Path.h>
#include <gf/ResourceManager.h>
#include <gf/ViewContainer.h>
#include <gf/Views.h>
#include <gf/Array2D.h>

#define SPRITE_SIZE 64

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
        Square& getSquare(int x, int y);

        // Get the piece at the given coordinates
        Piece& getPiece(int x, int y);

        std::vector<stg::Piece> getAllPiece();

        std::string getTexture(stg::PieceName name, stg::Color color);

        // Set the piece at the given coordinates
        void setPiece(int x, int y, Piece piece);

        void unsetPiece(gf::Vector2i coords);

        //Set the piece at the initial coordinates from a given color
        void setPieceFromColor(stg::Color color);

        //Finish to set the starting board after board has been send
        void setOtherSide(stg::Color color);

        //Render the board
        void render(gf::RenderWindow &renderer, gf::AdaptativeView* view);
        void movePiece(gf::Vector2i from, gf::Vector2i to);

        void toString();

        void swapPiece(gf::Vector2i first, gf::Vector2i other);

        gf::ResourceManager manager;
    private:

        gf::Array2D<std::pair<Square,stg::Piece>> board;
        std::map<std::pair<stg::PieceName, stg::Color>, std::string> pieceTextures;
        std::map<std::string, std::string> tileTextures;
    };

}

#endif //STGCLIENT_BOARD_H
