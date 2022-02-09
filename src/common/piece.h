#ifndef PIECE_H
#define PIECE_H

#include <string>
#include <map>
#include <gf/RenderWindow.h>

#define COORD_MIN 0
#define COORD_MAX 9

namespace stg {
    enum PieceName {
        NONE,
        LAKE,
        DRAPEAU,
        ESPION,
        ECLAIREUR,
        DEMINEUR,
        SERGENT,
        LIEUTENANT,
        CAPITAINE,
        COMMANDANT,
        COLONEL,
        GENERAL,
        MARECHAL,
        BOMBE,
        PION
    };

    enum Color {
        EMPTY,
        RED,
        BLUE
    };

    class Piece {

        public:
            Piece() = default;
            Piece(PieceName _name,Color _col);

            PieceName getPieceName() const;
            Color getColor() const;
            bool isDisplayed() const;
            Piece& setDisplay(bool set_display);

            int battleResult(const Piece& other) const;
            
            static Piece makeBlankPiece();

            PieceName name;
            Color col;
            bool display;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, stg::Piece& p) {
        return ar | p.name | p.col;
    }
}

#endif