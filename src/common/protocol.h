//
// Created by ellipsialp on 10/11/2021.
//

#include <string>
#include <gf/Packet.h>

#include <gf/Id.h>
#include <gf/SerializationOps.h>

using namespace gf::literals;

namespace stg {

    enum RequestType {
        ERROR,
        CONNECTION,
        WAITING,
        BOARD,
        MOVE,
        END,
    };

    struct coords {
        int x;
        int y;
    };

    struct ClientHello {
        static constexpr gf::Id type = "ClientHello"_id;
        std::string name;
    };

    template<typename Archive>
    Archive operator|(Archive& ar, ClientHello& data) {
        return ar | data.name;
    }

    class Request {
    public :
        stg::RequestType type;

        gf::Packet serialize();
    };

    class ErrorRequest : public Request {
    public:
        stg::RequestType type = ERROR;
        struct {
            std::string message;
        } content;
    };

    /*class ConnectionRequest : public Request {
        stg::RequestType type = CONNECTION;
        struct {
            enum {
                OK,
                KO
            } status;
        } content;
    };

    class WaitingRequest : public Request {
        stg::RequestType type = WAITING;
        struct {
            std::string message;
        } content;
    };

    class BoardRequest : public Request {
        stg::RequestType type = BOARD;
        struct {
            //stg::Board board;
            enum {
                RED,
                BLUE
            } color;
        } content;
    };

    class MoveRequest : public Request {
        stg::RequestType type = MOVE;
        struct {
            std::vector<struct {
                stg::coords from;
                stg::coords to;
            }> moves;
        } content;
    };

    class EndRequest : public Request {
        stg::RequestType type = END;
        struct {
            enum {
                RED,
                BLUE
            } winner;
        } content;
    };*/

}

#ifndef STG_PROTOCOL_H
#define STG_PROTOCOL_H

#endif //STG_PROTOCOL_H
