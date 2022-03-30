#include <iostream>
#include "board.h"

namespace stg {

    Square::Square(TileType type, bool isWalkable) {
        this->type = type;
        this->walkable = isWalkable;
    }

    TileType Square::getType() {
        return this->type;
    }

    bool Square::isWalkable() {
        return this->walkable;
    }

    Board::Board() {

        this->board = gf::Array2D<std::pair<Square,Piece>>(gf::Vector2i(10, 10), std::pair<Square,Piece>(Square(TileType::Land, true), Piece(PieceName::NONE, Color::EMPTY)));
        for(int i = 0; i < 10; i++) {
            for(int j = 0; j < 10; j++) {
                if(j == 4 || j == 5) {
                    if (i == 2 || i == 3 || i == 6 || i == 7) {
                        board(gf::Vector2i(i,j)) = std::pair<Square,Piece>(Square(TileType::River, false), Piece(PieceName::NONE, Color::EMPTY));
                    }
                }
            }
        }


        manager.addSearchDir("resources");

        tileTextures.insert(std::pair("land", "field.png"));
        tileTextures.insert(std::pair("land_top_side", "field_top.png"));
        tileTextures.insert(std::pair("land_bot_side", "field_bottom.png"));
        tileTextures.insert(std::pair("land_right_side", "field_right.png"));
        tileTextures.insert(std::pair("land_left_side", "field_left.png"));
        tileTextures.insert(std::pair("land_top_left_edge", "field_top_left.png"));
        tileTextures.insert(std::pair("land_top_right_edge", "field_top_right.png"));
        tileTextures.insert(std::pair("land_bot_left_edge", "field_bottom_left.png"));
        tileTextures.insert(std::pair("land_bot_right_edge", "field_bottom_right.png"));

        tileTextures.insert(std::pair("lake_bottom_right", "lac_bottom_right.png"));
        tileTextures.insert(std::pair("lake_bottom_left", "lac_bottom_left.png"));
        tileTextures.insert(std::pair("lake_top_right", "lac_top_right.png"));
        tileTextures.insert(std::pair("lake_top_left", "lac_top_left.png"));

        pieceTextures.insert(std::pair(std::pair(PieceName::PION, Color::BLUE), "blue_piece.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::DRAPEAU, Color::BLUE), "blue_flag.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::BOMBE, Color::BLUE), "blue_bomb.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::ESPION, Color::BLUE), "blue_spy.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::ECLAIREUR, Color::BLUE), "blue_scout.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::DEMINEUR, Color::BLUE), "blue_deminer.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::SERGENT, Color::BLUE), "blue_sergent.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::LIEUTENANT, Color::BLUE), "blue_lieutenant.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::COMMANDANT, Color::BLUE), "blue_commandant.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::CAPITAINE, Color::BLUE), "blue_capitain.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::COLONEL, Color::BLUE), "blue_colonel.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::GENERAL, Color::BLUE), "blue_general.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::MARECHAL, Color::BLUE), "blue_marechal.png"));

        pieceTextures.insert(std::pair(std::pair(PieceName::PION, Color::RED), "red_piece.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::DRAPEAU, Color::RED), "red_flag.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::BOMBE, Color::RED), "red_bomb.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::ESPION, Color::RED), "red_spy.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::ECLAIREUR, Color::RED), "red_scout.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::DEMINEUR, Color::RED), "red_deminer.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::SERGENT, Color::RED), "red_sergent.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::LIEUTENANT, Color::RED), "red_lieutenant.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::COMMANDANT, Color::RED), "red_commandant.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::CAPITAINE, Color::RED), "red_capitain.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::COLONEL, Color::RED), "red_colonel.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::GENERAL, Color::RED), "red_general.png"));
        pieceTextures.insert(std::pair(std::pair(PieceName::MARECHAL, Color::RED), "red_marechal.png"));
    }

    Square& Board::getSquare(int x, int y) {
        return board(gf::Vector2i(x,y)).first;
    }

    void Board::setPiece(int x, int y, Piece piece) {
        board(gf::Vector2i(x,y)).second = piece;
    }

    void Board::unsetPiece(gf::Vector2i coords) {
        board(coords).second = Piece(stg::PieceName::NONE, stg::Color::EMPTY);
    }

    Piece& Board::getPiece(int x, int y) {
        return board(gf::Vector2i(x,y)).second;
    }

    std::vector<stg::Piece> Board::getAllPiece() {

        std::vector<stg::Piece> to_send;

        for (int y = 6; y <= COORD_MAX; ++y) {

            for (int x = COORD_MIN; x <= COORD_MAX; ++x) {

                to_send.push_back(board(gf::Vector2i(x,y)).second);

            }

        }

        return to_send;

    }

    std::string Board::getTexture(stg::PieceName name, stg::Color color) {

        return pieceTextures[std::pair<stg::PieceName, stg::Color>(name, color)];

    }



    void Board::setPieceFromColor(stg::Color color) {

        int x = COORD_MIN, y = 6;
        while (x < 8) {
            setPiece(x,y,stg::Piece(stg::PieceName::ECLAIREUR,color));
            ++x;
        }
        while (x < 10) {
            setPiece(x,y,stg::Piece(stg::PieceName::COLONEL,color));
            ++x;
        }
        x = 0;
        ++y;
        while (x < 6) {
            setPiece(x,y,stg::Piece(stg::PieceName::BOMBE,color));
            ++x;
        }
        while (x < 10) {
            setPiece(x,y,stg::Piece(stg::PieceName::SERGENT,color));
            ++x;
        }
        x = 0;
        ++y;
        while (x < 5) {
            setPiece(x,y,stg::Piece(stg::PieceName::DEMINEUR,color));
            ++x;
        }
        while (x < 9) {
            setPiece(x,y,stg::Piece(stg::PieceName::LIEUTENANT,color));
            ++x;
        }
        setPiece(x,y,stg::Piece(stg::PieceName::MARECHAL,color));
        x = 0;
        ++y;
        while (x < 4) {
            setPiece(x,y,stg::Piece(stg::PieceName::CAPITAINE,color));
            ++x;
        }
        while (x < 7) {
            setPiece(x,y,stg::Piece(stg::PieceName::COMMANDANT,color));
            ++x;
        }
        setPiece(x,y,stg::Piece(stg::PieceName::GENERAL,color));
        ++x;
        setPiece(x,y,stg::Piece(stg::PieceName::ESPION,color));
        ++x;
        setPiece(x,y,stg::Piece(stg::PieceName::DRAPEAU,color));

    }

    void Board::setOtherSide(stg::Color color) {

        for (int y = COORD_MIN; y < 4; ++y) {
            for (int x = COORD_MIN; x <= COORD_MAX; ++x) {
                setPiece(x, y, stg::Piece(stg::PieceName::PION, color));
            }
        }

    }

    void Board::render(gf::RenderWindow &renderer, gf::AdaptativeView *view) {
        renderer.setView(*view);
        for(int i = COORD_MIN; i <= COORD_MAX; i++) {
            for(int j = COORD_MIN; j <= COORD_MAX; j++) {
                gf::Sprite sprite;
                sprite.setPosition(gf::Vector2f(i*SPRITE_SIZE, j*SPRITE_SIZE));
                if(board(gf::Vector2i(i,j)).first.getType() == TileType::Land) {
                    if(j==COORD_MIN) {
                        if(i==COORD_MIN) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_top_left_edge"))));
                        } else if(i==COORD_MAX) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_top_right_edge"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_top_side"))));
                        }
                    } else if(j==COORD_MAX) {
                        if(i==COORD_MIN) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_bot_left_edge"))));
                        } else if(i==COORD_MAX) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_bot_right_edge"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_bot_side"))));
                        }
                    } else if(i==COORD_MIN) {
                        sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_left_side"))));
                    } else if(i==COORD_MAX) {
                        sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_right_side"))));
                    } else {
                        sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land"))));
                    }
                } else if(board(gf::Vector2i(i,j)).first.getType() == TileType::River) {
                    if(board(gf::Vector2i(i-1,j)).first.getType()==Land) {
                        if(board(gf::Vector2i(i,j+1)).first.getType()==Land) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_bottom_left"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_top_left"))));
                        }
                    } else if (board(gf::Vector2i(i+1,j)).first.getType()==Land) {
                        if (board(gf::Vector2i(i,j+1)).first.getType() == Land) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_bottom_right"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_top_right"))));
                        }
                    }
                }
                renderer.draw(sprite);
                if(board(gf::Vector2i(i,j)).second.getPieceName()!=PieceName::NONE && board(gf::Vector2i(i,j)).second.getPieceName() != stg::PieceName::LAKE && board(gf::Vector2i(i,j)).second.display) {
                    gf::Sprite sprite;
                    sprite.setPosition(gf::Vector2f(i*SPRITE_SIZE, j*SPRITE_SIZE));
                    sprite.setTexture(manager.getTexture(gf::Path(pieceTextures.at(std::pair(board(gf::Vector2i(i,j)).second.getPieceName(), board(gf::Vector2i(i,j)).second.getColor())))));
                    renderer.draw(sprite);
                }
            }
        }
    }

    void Board::movePiece(gf::Vector2i from, gf::Vector2i to) {
        if ((from == gf::Vector2i(-1,-1)) || (to == gf::Vector2i(-1,-1))) return;
        if (board(to).second.getPieceName() != stg::PieceName::NONE) return;
        setPiece(to.x, to.y, stg::Piece(board(from).second.getPieceName(), board(from).second.getColor()));
        board(from).second = Piece(stg::PieceName::NONE, stg::Color::EMPTY);
        return;
    }

    void Board::toString(){
        for (auto y : board) {
            for (auto x = 0; x<10; x++) {
                std::cout << y.second.getPieceName() << " ";
            }
            std::cout << std::endl;
        }
    }

    void Board::swapPiece(gf::Vector2i first, gf::Vector2i other) {
        if ((first == gf::Vector2i(-1,-1)) || (other == gf::Vector2i(-1,-1))) return;
        if (board(first).second.getPieceName() == stg::PieceName::NONE) return;
        if (board(other).second.getPieceName() == stg::PieceName::NONE) return;
        stg::Piece firstPiece = stg::Piece(board(first).second.getPieceName(), board(first).second.getColor());
        stg::Piece otherPiece = stg::Piece(board(other).second.getPieceName(), board(other).second.getColor());
        setPiece(other.x, other.y, firstPiece);
        setPiece(first.x, first.y, otherPiece);
        return;
    }

}