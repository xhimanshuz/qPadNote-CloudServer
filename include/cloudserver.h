#ifndef CLOUDSERVER_H
#define CLOUDSERVER_H

#include <boost/asio.hpp>
#include <iostream>

#include "sessionhandler.h"

class CloudServer
{
    std::shared_ptr<boost::asio::io_context> ioc;
    std::vector<std::shared_ptr<SessionHandler>> sessionVector;

public:
    explicit CloudServer(std::shared_ptr<boost::asio::io_context> _ioc);
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    void sessionRefresh();

    void acceptConnection();
};

#endif // CLOUDSERVER_H
