#ifndef PIECE_H
#define PIECE_H

#include <string>
#include <map>


namespace stg {
    enum PieceName {
        NONE,
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
        BOMBE
    };

    enum Color {
        RED,
        BLUE
    };

    class Piece {

        public:
            Piece(PieceName _name,Color _col);

            PieceName getPieceName();
            Color getColor();

        private:
        PieceName name;
        Color col;

    };

    template<typename Archive>
    Archive operator|(Archive& ar, stg::Piece& p) {
        return ar | p.name | p.col;
    }
}

#endif