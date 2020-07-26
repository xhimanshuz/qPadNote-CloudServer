#include "cloudserver.h"

CloudServer::CloudServer(std::shared_ptr<boost::asio::io_context> _ioc): ioc(std::move(_ioc))
{
    std::cout<<"[!] Server Started!"<<std::endl;
    acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(*ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8000));
    acceptConnection();
}

void CloudServer::sessionRefresh()
{
    std::vector<std::vector<std::shared_ptr<SessionHandler>>::iterator> sessionToDelete;

    for(auto i=sessionVector.begin(); i!=sessionVector.end(); i++)
    {
        if(i->get()->isObsolete())
        {
            std::cout<<"[!] Session is obsolute now, Removing Session: "<< i->get()->socket->remote_endpoint();
            sessionToDelete.push_back(i);
            i->reset();
        }
    }

    for(auto i: sessionToDelete)
        sessionVector.erase(i);
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
        auto session = std::make_shared<SessionHandler>(std::make_unique<boost::asio::ip::tcp::socket>(std::move(socket)));
        sessionVector.push_back(session);
        try
        {
            session->start();
        }
        catch (boost::system::system_error se)
        {
            std::cout<<"[E] Exception in session: "<< session->getSessionId()<<" ERROR: " << se.code().value()<<" : "<< se.what()<<" -> " <<std::endl;
            session->makeObsolete();
            sessionRefresh();
        }

        sessionRefresh();
        acceptConnection();
    });
}
