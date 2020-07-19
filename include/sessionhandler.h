#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <thread>
#include <boost/asio.hpp>
#include <iostream>

#include "mongocxxinterface.h"
#include "Protocol.h"

class SessionHandler/*: public std::enable_shared_from_this<SessionHandler>*/
{
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;

    std::shared_ptr<MongocxxInterface> mongocxx;
public:
    SessionHandler(std::unique_ptr<boost::asio::ip::tcp::socket> _socket);
    void sendData();
    void sendHeader();
    void receiveHeader();
    void receiveBlocks(uint16_t size, uint8_t quantity);
    void removeBlock();
    void removeBlocks();
    void requestHandle();
    void BlocksTabRequest(int16_t uid, std::string tid);
    void BlocksAllRequest(int16_t uid);

    void sendBlocks(std::vector<Protocol::Block> blockVector);
    bool sendHeader(Protocol::TYPE _type, Protocol::TYPE _body, uint16_t _bodySize, uint8_t _quantity);

    ~SessionHandler();
//    std::shared_ptr<SessionHandler> getThis();
};

#endif // SESSIONHANDLER_H
