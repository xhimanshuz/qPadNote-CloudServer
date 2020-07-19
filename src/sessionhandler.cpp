#include "sessionhandler.h"

SessionHandler::SessionHandler(std::unique_ptr<boost::asio::ip::tcp::socket> _socket): socket(std::move(_socket))
{
    mongocxx = MongocxxInterface::get();

    //    socket->send(boost::asio::buffer("You are connected"));
    receiveHeader();
}

void SessionHandler::sendData()
{
    auto blockVector = mongocxx->getBlocks();

    Protocol::Block blocks[blockVector.size()];
    for(auto i=0; i<blockVector.size(); i++)
        blocks[i] = blockVector[i];

    Protocol::Header header(Protocol::TYPE::HEADER, Protocol::TYPE::BLOCK, sizeof(blocks), blockVector.size());
    auto hsize = socket->write_some(boost::asio::buffer(&header, sizeof(header)));
    std::cout<<"[!] Header Writed Bytes: "<< hsize << " SENT Size:"<< header.size<<" Quantity: "<< (int)header.quantity<< " BlockSize: "<< header.size <<std::endl;

    socket->wait(socket->wait_write);

    auto bsize = socket->write_some(boost::asio::buffer(&blocks, sizeof(blocks)));
    std::cout<<"[!] Blocks Writed Bytes: "<< bsize;
}

void SessionHandler::sendHeader()
{

}

void SessionHandler::receiveHeader()
{
    Protocol::Header header;
    auto readSize = socket->read_some(boost::asio::buffer(&header, sizeof(header)));

    if(header.type == Protocol::TYPE::HEADER)
    {
        std::cout<<"[!] HEADER Recived size: "<< readSize<<std::endl;
        switch(header.body)
        {
        case Protocol::TYPE::BLOCK:
        {
            receiveBlocks(header.size, header.quantity);
            break;
        }
        case Protocol::TYPE::DELETE_BLOCK:
        {
            removeBlock();
            break;
        }
        case Protocol::TYPE::REQUEST:
        {
            requestHandle();
            break;
        }
        default:
        {
            std::cout<<"[E] Invalid Header Type!\n";
        }
        }
    }
    receiveHeader();
}

void SessionHandler::receiveBlocks(uint16_t size, uint8_t quantity)
{
    std::cout<<"[!] Blocks Received\n";
    Protocol::Block blocks[quantity];

    auto received_size = socket->read_some(boost::asio::buffer(&blocks, sizeof(blocks)));
    std::cout<<"[!] Received_size from client: "<< received_size<< ", To be Receiving: "<< size<<std::endl;

    std::cout<< "Writing to MongoDB\n";
    for(auto i=0; i< quantity; i++)
    {
        mongocxx->putBlock(blocks[i]);
    }

}

void SessionHandler::removeBlock()
{
    std::cout<<"[!] Remove Block Request Received\n";
    int64_t _id;

    auto received_size = socket->read_some(boost::asio::buffer(&_id, sizeof(_id)));
    std::cout<<"[!] Received_size from client: "<< received_size<< ", To be Receiving: "<< received_size <<std::endl;

    mongocxx->removeBlock(_id);
}

void SessionHandler::removeBlocks()
{
    std::cout<<"[!] Removing Blocks";
}

void SessionHandler::requestHandle()
{
    std::cout<< "[!] Request Received\n";
    Protocol::Request::BlockRequest request;
    auto receive_size = socket->read_some(boost::asio::buffer(&request, sizeof(request)));
    std::cout<<"Request Received Size: "<< receive_size<< " Type: "<< request.type<<std::endl;

    switch(request.type)
    {
    case Protocol::Request::TYPE::BLOCKS_ALL_REQ:
        BlocksAllRequest(request.uid);
        break;
    case Protocol::Request::TYPE::BLOCKS_TAB_REQ:
        BlocksTabRequest(request.uid, std::string(request.tid));
        break;
    default:
        std::cout<<"[E] Invalid Request!\n";
        break;
    }
}

void SessionHandler::BlocksTabRequest(int16_t uid, std::string tid)
{
    std::cout<< "Block Tab Requested! for uid: "<< uid <<" tid: "<< tid<<std::endl;
    auto blockVector = mongocxx->getBlocks(uid, tid);

    sendBlocks(blockVector);
}

void SessionHandler::BlocksAllRequest(int16_t uid)
{
    std::cout<< "Block All Requested! for uid: "<< uid<< std::endl;
    auto blockVector = mongocxx->getBlocks("uid", uid);

    sendBlocks(blockVector);
}

void SessionHandler::sendBlocks(std::vector<Protocol::Block> blocksVector)
{
    Protocol::Block blocks[blocksVector.size()];

    for(auto i=0; i< blocksVector.size(); i++)
        blocks[i] = blocksVector[i];

    if(sendHeader(Protocol::TYPE::HEADER, Protocol::TYPE::BLOCKS, sizeof(blocks), blocksVector.size()))
    {
        auto bsize = socket->write_some(boost::asio::buffer(&blocks, sizeof(blocks)));
        std::cout<<"[!] Blocks Data Sent! "<< bsize << " with Quanity: "<< blocksVector.size()<<std::endl;
    }

}

bool SessionHandler::sendHeader(Protocol::TYPE _type, Protocol::TYPE _body, uint16_t _bodySize, uint8_t _quantity)
{
    try
    {
        Protocol::Header header(_type, _body, _bodySize, _quantity);
        auto rhsize = socket->write_some(boost::asio::buffer(&header, sizeof(header)));
        std::cout<<"[!] Header Writed with Size: "<< rhsize<<std::endl;
    }
    catch (boost::system::system_error se)
    {
        std::cout<<"[E] Exception Occured! For Header: "<< se.what()<<std::endl;
        return false;
    }
    return true;
}


SessionHandler::~SessionHandler()
{
    std::cout<<"[!] Session for socket: "<< socket->remote_endpoint()<<" is disconnected!\n";
    //    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_type::shutdown_both);
}

