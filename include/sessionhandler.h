#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <thread>
#include <boost/asio.hpp>
#include <iostream>

#include "mongocxxinterface.h"
#include "Protocol.h"

class SessionHandler/*: public std::enable_shared_from_this<SessionHandler>*/
{
    std::shared_ptr<MongocxxInterface> mongocxx;
    int sessionId;
    bool obsolete;
public:
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;
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
    bool isObsolete();
    bool start();
    void deleteTab(std::string tid = "");
    void renameTab(std::string tid, std::string new_tid);

    void sendBlocks(std::vector<Protocol::Block> blockVector);
    bool sendHeader(Protocol::TYPE _type, Protocol::TYPE _body, uint16_t _bodySize, uint8_t _quantity);

    ~SessionHandler();
//    std::shared_ptr<SessionHandler> getThis();
    int getSessionId() const;
    void makeObsolete(bool value = true);
};

#endif // SESSIONHANDLER_H
