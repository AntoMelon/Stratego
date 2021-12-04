#include "board.h"

#include <cmath>

namespace stg {
    Board::Board() {

    }

    bool Board::addPiece(int x, int y, stg::Piece& piece) {
        if (board.count({x,y}) > 0) {
            return false;
        }

        board.insert({{x,y},piece});
        return true;
    }

    bool Board::removePiece(int x, int y) {
        if (board.count({x,y}) == 0) {
            return false;
        }

        return board.erase({x,y});
    }

    bool Board::isMoveAllowed(int from_x,int from_y, int to_x, int to_y) const {
        if (from_x == to_y && from_y == to_y) {
            return false;
        }
        
        stg::Piece to_move = board.find({from_x,from_y})->second;

        if (to_move.getPieceName() == stg::PieceName::NONE || to_move.getPieceName() == stg::PieceName::LAKE || to_move.getPieceName() == stg::PieceName::DRAPEAU || to_move.getPieceName() == stg::PieceName::BOMBE) {
            return false;
        }

        stg::Piece target = board.find({to_x,to_y})->second;
        if (target.getColor() == to_move.getColor() || target.getPieceName() == stg::PieceName::LAKE) {
            return false;
        }

        if (to_move.getPieceName() == stg::PieceName::ECLAIREUR) {
            if (from_x == to_x) {

                for (int i = std::min(from_y,to_y)+1; i < std::max(from_y,to_y); ++i) {
                    if (board.find({from_x,i})->second.getPieceName() != stg::PieceName::NONE) {
                        return false;
                    }
                }

                return true;

            } if (from_y == to_y) {
                
                for (int i = std::min(from_x,to_x)+1; i < std::max(from_x,to_x); ++i) {
                    if (board.find({i,from_y})->second.getPieceName() != stg::PieceName::NONE) {
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

        stg::Piece to_move = board.find({from_x,from_y})->second;
        stg::Piece target = board.find({to_x,to_y})->second;

        int result = to_move.battleResult(target);

        board.erase({from_x,from_y});
        board.insert({{from_x,from_y},stg::Piece::makeBlankPiece()});

        if (result >= 0) {
            board.erase({to_x,to_y});

            if (result == 1) {
                board.insert({{to_x,to_y},to_move});
            }
        }
        

        return true;
    }
}