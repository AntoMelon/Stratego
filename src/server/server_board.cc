#include "server_board.h"
#include "../client/libs/board.h"

#include <iostream>
#include <cmath>

namespace stg {

    ServerBoard::ServerBoard() : board({}) {
        
        for (int i = COORD_MIN; i <= COORD_MAX; ++i) {
            std::vector<stg::Piece> col;
            for (int j = COORD_MIN; j <= COORD_MAX; ++j) {
                col.push_back(stg::Piece::makeBlankPiece());
            }
            board.push_back(col);
        }

        setPiece(2,4,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(3,4,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(2,5,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(3,5,{stg::PieceName::LAKE,stg::Color::EMPTY});

        setPiece(6,4,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(7,4,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(6,5,{stg::PieceName::LAKE,stg::Color::EMPTY});
        setPiece(7,5,{stg::PieceName::LAKE,stg::Color::EMPTY});

    }

    bool ServerBoard::isMoveAllowed(int from_x,int from_y, int to_x, int to_y, stg::Color color) const {
        if (from_x < COORD_MIN || from_x > COORD_MAX || from_y < COORD_MIN || from_y > COORD_MAX || to_x < COORD_MIN || to_x > COORD_MAX || to_y < COORD_MIN || to_y > COORD_MAX) {
            return false;
        }

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
            std::cout << "pas de pièce" << std::endl;
            return false;
        }

        stg::Piece target = getPiece(to_x,to_y);
        if (target.getColor() == color || target.getPieceName() == stg::PieceName::LAKE) {
            std::cout << "case indisponible" << std::endl;
            return false;
        }

        if (to_move.getPieceName() == stg::PieceName::ECLAIREUR) {
            if (from_x == to_x) {

                for (int i = std::min(from_y,to_y)+1; i < std::max(from_y,to_y); ++i) {
                    if (getPiece(from_x,i).getPieceName() != stg::PieceName::NONE) {
                        std::cout << "pas aligné" << std::endl;
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
        for (auto col : board) {
            for (auto piece : col) {
                if (piece.getPieceName() == stg::PieceName::DRAPEAU && piece.getColor() == color) {
                    return true;
                }
            }
        }
        return false;
    }

    stg::Piece ServerBoard::getPiece(int x, int y) const {
        return board[x][y];
    }

    void ServerBoard::setPiece(int x, int y, const stg::Piece& piece) {
        board[x][y] = piece;
    }

    void ServerBoard::importSubmittedBoard(stg::Color color, const std::vector<stg::Piece>& submitted) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 10; ++j) {

                if (color == stg::Color::RED) {
                    setPiece(9-j,3-i,submitted[i*10+j]);
                } else {
                    setPiece(j,6+i,submitted[i*10+j]);
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