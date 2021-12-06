#ifndef BOARD_H
#define BOARD_H

#include "./piece.h"

#include <vector>

namespace stg {

    class Board {
        public:
        Board();
        bool addPiece(int x, int y, stg::Piece& piece);
        bool removePiece(int x, int y);

        bool isMoveAllowed(int from_x,int from_y, int to_x, int to_y) const;
        bool movePiece(int from_x, int from_y, int to_x, int to_y);


        stg::Piece getPiece(int x, int y) const;
        void setPiece(int x, int y, const stg::Piece& piece);


        private:
        std::vector<stg::Piece> board;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, stg::Board& board) {
        return ar;
    }
}

#endif