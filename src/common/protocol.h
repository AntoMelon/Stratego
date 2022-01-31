//
// Created by ellipsialp on 10/11/2021.
//

#include <string>
#include <vector>

#include <gf/Packet.h>

#include <gf/Id.h>
#include <gf/SerializationOps.h>

#include "piece.h"

#define HOST "localhost"
#define PORT "4269"

using namespace gf::literals;

namespace stg {

    enum PLAYING_STATE {CONNEXION, PLACEMENT, IN_GAME, END};

    enum ResponseCode {
        WAITING,
        STARTING,
        BOARD_OK,
        BOARD_ERR,
        MOVE_ERR
    };


    /*Client -> Server*/

    struct ClientHello {
        static constexpr gf::Id type = "ClientHello"_id;
        std::string name;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, ClientHello& data) {
        return ar | data.name;
    }

    struct ClientMoveRequest {
        static constexpr gf::Id type = "ClientMoveRequest"_id;
        int from_x;
        int from_y;
        int to_x;
        int to_y;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, ClientMoveRequest& move) {
        return ar | move.from_x | move.from_y | move.to_x | move.to_y;
    }

    struct ClientBoardSubmit {
        static constexpr gf::Id type = "ClientBoardSubmit"_id;

        stg::Color color;
        std::vector<stg::Piece> board;
    };

    template<typename Archive> 
    Archive operator|(Archive& ar, ClientBoardSubmit& submit) {
        return ar | submit.color | submit.board;
    }

    /*Server -> Client*/

    struct ServerMessage {
        static constexpr gf::Id type = "ServerMessage"_id;
        stg::ResponseCode code;
        std::string message;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, ServerMessage& response) {
        return ar | response.code | response.message;
    }

    struct ServerAssignColor {
        static constexpr gf::Id type = "ServerAssignColor"_id;
        stg::Color color;
        bool starting;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, ServerAssignColor& assign) {
        return ar | assign.color | assign.starting;
    }

    struct ServerMoveNotif {
        static constexpr gf::Id type = "ServerMoveNotif"_id;
        int from_x;
        int from_y;
        int to_x;
        int to_y;

        int str_atk;
        int str_def;

        bool atk_alive;
        bool def_alive;

        bool win;
        bool lose;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, ServerMoveNotif& move) {
        return ar | move.from_x | move.from_y | move.to_x | move.to_y | move.str_atk | move.str_def | move.atk_alive | move.def_alive | move.win | move.lose;
    }
}

#ifndef STG_PROTOCOL_H
#define STG_PROTOCOL_H

#endif //STG_PROTOCOL_H
