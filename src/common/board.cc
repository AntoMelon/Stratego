#include "board.h"
#include "../client/libs/board.h"


#include <cmath>

namespace stg {
    Board::Board() {

    }

    const std::vector<std::vector<std::pair<Square, stg::Piece>>> &Board::getBoard() const {
        return board;
    }

    bool Board::isMoveAllowed(int from_x,int from_y, int to_x, int to_y) const {
        if (from_x == to_y && from_y == to_y) {
            return false;
        }
        
        stg::Piece to_move = getPiece(from_x,from_y);

        if (to_move.getPieceName() == stg::PieceName::NONE || to_move.getPieceName() == stg::PieceName::LAKE || to_move.getPieceName() == stg::PieceName::DRAPEAU || to_move.getPieceName() == stg::PieceName::BOMBE) {
            return false;
        }

        stg::Piece target = getPiece(to_x,to_y);
        if (target.getColor() == to_move.getColor() || target.getPieceName() == stg::PieceName::LAKE) {
            return false;
        }

        if (to_move.getPieceName() == stg::PieceName::ECLAIREUR) {
            if (from_x == to_x) {

                for (int i = std::min(from_y,to_y)+1; i < std::max(from_y,to_y); ++i) {
                    if (getPiece(from_x,i).getPieceName() != stg::PieceName::NONE) {
                        return false;
                    }
                }

                return true;

            } if (from_y == to_y) {
                
                for (int i = std::min(from_x,to_x)+1; i < std::max(from_x,to_x); ++i) {
                    if (getPiece(i,from_y).getPieceName() != stg::PieceName::NONE) {
                        return false;
                    }
                }

                return true;

            } 

            return false;
        }

        return (std::abs(from_x-to_x) == 1 && std::abs(from_y-to_y) == 0) || (std::abs(from_x-to_x) == 0 && std::abs(from_y-to_y) == 1);
    }

    bool Board::movePiece(int from_x, int from_y, int to_x, int to_y) {
        if (!isMoveAllowed(from_x,from_y,to_x,to_y)) {
            return false;
        }

        stg::Piece to_move = getPiece(from_x,from_y);
        stg::Piece target = getPiece(to_x,to_y);

        int result = to_move.battleResult(target);

        setPiece(from_x,from_y,stg::Piece::makeBlankPiece());

        if (result >= 0) {
            setPiece(to_x,to_y,stg::Piece::makeBlankPiece());

            if (result == 1) {
                setPiece(to_x,to_y,to_move);
            }
        }
        

        return true;
    }

    stg::Piece Board::getPiece(int x, int y) const {
        return board[x*10+y];
    }

    void Board::setPiece(int x, int y, const stg::Piece& piece) {
        board[x*10+y] = piece;
    }

}