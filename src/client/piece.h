#ifndef PIECE_H
#define PIECE_H

#include <string>
#include <map>


namespace stg {
    const std::map<int,std::string> pieceName {
            {0, "Drapeau"},
            {1, "Espion"},
            {2, "Eclaireur"},
            {3, "Demineur"},
            {4, "Sergent"},
            {5, "Lieutenant"},
            {6, "Capitaine"},
            {7, "Commandant"},
            {8, "Colonel"},
            {9, "Sergent"},
            {10, "Marechal"},
            {99, "Bombe"}
        };
    
    std::string getPieceName(int strength) {
        return pieceName.find(strength)->second;
    }

    class Piece {

        public:
            Piece(int str);

            int getStrength();

        private:
        int strength;

    };

    template<typename Archive>
    Archive operator|(Archive& ar, stg::Piece& p) {
        return ar | p.strength;
    }
}

#endif