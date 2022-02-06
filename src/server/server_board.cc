#include "server_board.h"
#include "../client/libs/board.h"

#include <iostream>
#include <cmath>

namespace stg {

    ServerBoard::ServerBoard() : board({}) {
        for (int i = 0; i < 100; ++i) {
            board.push_back(stg::Piece::makeBlankPiece());
        }

        setPiece(4,2,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(4,3,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(5,2,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(5,3,{stg::PieceName::LAKE,stg::Color::EMPTY});

        setPiece(4,6,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(4,7,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(5,6,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(5,7,{stg::PieceName::LAKE,stg::Color::EMPTY});

    }

    bool ServerBoard::isMoveAllowed(int from_x,int from_y, int to_x, int to_y, stg::Color color) const {

        //same place
        if (from_x == to_x && from_y == to_y) {
            return false;
        }

        stg::Piece to_move = getPiece(from_x,from_y);

        std::cout << "Piece at (" << from_x <<"," << from_y << ") is a " << to_move.getPieceName() << std::endl;

        if (to_move.getPieceName() == stg::PieceName::NONE 
        || to_move.getPieceName() == stg::PieceName::LAKE 
        || to_move.getPieceName() == stg::PieceName::DRAPEAU 
        || to_move.getPieceName() == stg::PieceName::BOMBE
        || to_move.getColor() != color) {
            return false;
        }

        stg::Piece target = getPiece(to_x,to_y);
        if (target.getColor() == color || target.getPieceName() == stg::PieceName::LAKE) {
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

    bool ServerBoard::movePiece(int from_x, int from_y, int to_x, int to_y, stg::Color color) {
        if (!isMoveAllowed(from_x,from_y,to_x,to_y,color)) {
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

    bool ServerBoard::stillHasFlag(stg::Color color) const {
        for (auto piece : board) {
            if (piece.getPieceName() == stg::PieceName::DRAPEAU && piece.getColor() == color) {
                return true;
            }
        }
        return false;
    }

    stg::Piece ServerBoard::getPiece(int x, int y) const {
        return board[x*10+y];
    }

    void ServerBoard::setPiece(int x, int y, const stg::Piece& piece) {
        board[x*10+y] = piece;
    }

    void ServerBoard::importSubmittedBoard(stg::Color color, const std::vector<stg::Piece>& submitted) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 10; ++j) {

                if (color == stg::Color::RED) {
                    setPiece(3-i,9-j,submitted[i*10+j]);
                } else {
                    setPiece(6+i,j,submitted[i*10+j]);
                }
            }
        }

        debugPrint();
    }


    void ServerBoard::debugPrint() const {
        for (int i = COORD_MIN; i <= COORD_MAX; ++i) {
            for (int j = COORD_MIN; j <= COORD_MAX; ++j) {
                std::cout << getPiece(i,j).getPieceName() << " "; 
            }

            std::cout << std::endl;
        }
    }

}