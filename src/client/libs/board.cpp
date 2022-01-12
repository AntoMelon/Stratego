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
        this->board = std::vector<std::vector<std::pair<Square,Piece>>>();
        for(int i = 0; i < 10; i++) {
            std::vector<std::pair<Square,Piece>> row;
            for(int j = 0; j < 10; j++) {
                if (j==4 || j==5) {
                    if(i==2 || i==3 || i==6 || i==7) {
                        row.push_back(std::pair(Square(TileType::River, false), Piece::makeBlankPiece()));
                        continue;
                    }
                }
                row.push_back(std::pair(Square(TileType::Land, true), Piece::makeBlankPiece()));
            }
            this->board.push_back(row);
        }

        manager.addSearchDir(gf::Path("src/client/resources"));

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

    Square Board::getSquare(int x, int y) {
        return board[x][y].first;
    }

    void Board::setPiece(int x, int y, Piece piece) {
        board[x][y].second = piece;
    }

    Piece Board::getPiece(int x, int y) {
        return board[x][y].second;
    }

    void Board::render(gf::RenderWindow &renderer, gf::AdaptativeView *view) {
        renderer.setView(*view);
        for(int i = 0; i < 10; i++) {
            for(int j = 0; j < 10; j++) {
                gf::Sprite sprite;
                sprite.setPosition(gf::Vector2f(i*64, j*64));
                if(board[i][j].first.getType() == TileType::Land) {
                    if(j==0) {
                        if(i==0) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_top_left_edge"))));
                        } else if(i==9) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_top_right_edge"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_top_side"))));
                        }
                    } else if(j==9) {
                        if(i==0) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_bot_left_edge"))));
                        } else if(i==9) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_bot_right_edge"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_bot_side"))));
                        }
                    } else if(i==0) {
                        sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_left_side"))));
                    } else if(i==9) {
                        sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land_right_side"))));
                    } else {
                        sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("land"))));
                    }
                } else if(board[i][j].first.getType() == TileType::River) {
                    if(board[i-1][j].first.getType()==Land) {
                        if(board[i][j+1].first.getType()==Land) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_bottom_left"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_top_left"))));
                        }
                    } else if (board[i+1][j].first.getType()==Land) {
                        if (board[i][j + 1].first.getType() == Land) {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_bottom_right"))));
                        } else {
                            sprite.setTexture(manager.getTexture(gf::Path(tileTextures.at("lake_top_right"))));
                        }
                    }
                }
                renderer.draw(sprite);
                if(board[i][j].second.getPieceName()!=PieceName::NONE) {
                    gf::Sprite sprite;
                    sprite.setPosition(gf::Vector2f(i*64, j*64));
                    sprite.setTexture(manager.getTexture(gf::Path(pieceTextures.at(std::pair(board[i][j].second.getPieceName(), board[i][j].second.getColor())))));
                    renderer.draw(sprite);
                }
            }
        }
    }

    std::pair<std::string, bool> Board::movePiece(gf::Vector2i from, gf::Vector2i to) {
        if(board[from.x][from.y].second.getPieceName()==PieceName::NONE) {
            return std::pair("Case vide", false);
        }
        if(board[to.x][to.y].second.getPieceName()!=PieceName::NONE) {
            return std::pair("Case occupée", false);
        }

        // Everything OK, move the piece
        board[to.x][to.y].second = board[from.x][from.y].second;
        board[from.x][from.y].second = Piece::makeBlankPiece();
        return std::pair("", true);
    }

    gf::Vector2i Board::convert_mouse_coord_to_case(int mouse_x, int mouse_y, int window_x, int window_y) {

        gf::Vector2i result(0, 0);
        int size_considered = window_x;

        //adapt coord in function of the window size
        if (window_x < window_y) {

            mouse_y = mouse_y - ((window_y - window_x) / 2);

        } else if (window_x > window_y) {

            size_considered = window_y;
            mouse_x = mouse_x - ((window_x - window_y) / 2);

        }

        //calcul the right coord on the board
        //for x
        if (mouse_x  < size_considered / 10) {
            result.x = 0;
        } else if (mouse_x  < size_considered / 10 * 2) {
            result.x = 1;
        } else if (mouse_x  < size_considered / 10 * 3) {
            result.x = 2;
        } else if (mouse_x  < size_considered / 10 * 4) {
            result.x = 3;
        } else if (mouse_x  < size_considered / 10 * 5) {
            result.x = 4;
        } else if (mouse_x  < size_considered / 10 * 6) {
            result.x = 5;
        } else if (mouse_x  < size_considered / 10 * 7) {
            result.x = 6;
        } else if (mouse_x  < size_considered / 10 * 8) {
            result.x = 7;
        } else if (mouse_x  < size_considered / 10 * 9) {
            result.x = 8;
        } else {
            result.x = 9;
        }
        //for y
        if (mouse_y  < size_considered / 10) {
            result.y = 0;
        } else if (mouse_y  < size_considered / 10 * 2) {
            result.y = 1;
        } else if (mouse_y  < size_considered / 10 * 3) {
            result.y = 2;
        } else if (mouse_y  < size_considered / 10 * 4) {
            result.y = 3;
        } else if (mouse_y  < size_considered / 10 * 5) {
            result.y = 4;
        } else if (mouse_y  < size_considered / 10 * 6) {
            result.y = 5;
        } else if (mouse_y  < size_considered / 10 * 7) {
            result.y = 6;
        } else if (mouse_y  < size_considered / 10 * 8) {
            result.y = 7;
        } else if (mouse_y  < size_considered / 10 * 9) {
            result.y = 8;
        } else {
            result.y = 9;
        }

        return result;

    }

}