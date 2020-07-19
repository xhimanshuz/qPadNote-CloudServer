#include "cloudserver.h"

CloudServer::CloudServer(std::shared_ptr<boost::asio::io_context> _ioc): ioc(std::move(_ioc))
{
    std::cout<<"[!] Server Started!"<<std::endl;
    acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(*ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8000));
    acceptConnection();
}

void CloudServer::acceptConnection()
{
    std::cout<<"[!] Waiting to Accept Connections"<<std::endl;

    acceptor->async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
    {
        if(ec)
        {
            std::cout<<"[E] Error in Connection Acceptor!: "<< ec.message()<< std::endl;
            return;
        }

        std::cout<< "[!] Incoming connection, Socket Created: "<< socket.remote_endpoint()<<std::endl;
        sessionVector.push_back(std::make_shared<SessionHandler>(std::make_unique<boost::asio::ip::tcp::socket>(std::move(socket) )) );

        acceptConnection();
    });
}
