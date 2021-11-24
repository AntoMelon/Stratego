#include "piece.h"

namespace stg {
    Piece::Piece(int str) : strength(str) {

    }

    int Piece::getStrength() {
        return strength;
    }
}