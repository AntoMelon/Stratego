#ifndef STGSERVER_BOARD_H
#define STGSERVER_BOARD_H

#include "../common/piece.h"

#include <vector>

namespace stg {

    class ServerBoard {
        public:
        ServerBoard();
        bool addPiece(int x, int y, stg::Piece& piece);
        bool removePiece(int x, int y);

        bool isMoveAllowed(int from_x,int from_y, int to_x, int to_y) const;
        bool movePiece(int from_x, int from_y, int to_x, int to_y);


        stg::Piece getPiece(int x, int y) const;
        void setPiece(int x, int y, const stg::Piece& piece);

        void importSubmittedBoard(stg::Color color, const std::vector<stg::Piece>& submitted);


        private:
        std::vector<stg::Piece> board;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, stg::ServerBoard& board) {
        return ar;
    }
}

#endif