#include "piece.h"

namespace stg {
    Piece::Piece(PieceName _name, Color _col) : name(_name), col(_col) {

    }

    PieceName Piece::getPieceName() {
        return name;
    }

    Color Piece::getColor() {
        return col;
    }
}