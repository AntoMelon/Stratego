#include "piece.h"

namespace stg {
    Piece::Piece(PieceName _name, Color _col) : name(_name), col(_col) {

    }

    PieceName Piece::getPieceName() const {
        return name;
    }

    Color Piece::getColor() const {
        return col;
    }

    int Piece::battleResult(const Piece& other) const {
        if (name == stg::PieceName::ESPION && other.name == stg::PieceName::MARECHAL) {
            return 1;
        }

        if (name == stg::PieceName::DEMINEUR && other.name == stg::PieceName::BOMBE) {
            return 1;
        }

        if (name > other.name) {
            return 1;
        }

        if (name < other.name) {
            return -1;
        }

        return 0;
    }

    Piece Piece::makeBlankPiece() {
        return Piece(stg::PieceName::NONE,stg::Color::EMPTY);
    }
}