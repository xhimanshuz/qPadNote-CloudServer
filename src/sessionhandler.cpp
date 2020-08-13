#include "sessionhandler.h"

int SessionHandler::getSessionId() const
{
    return sessionId;
}

void SessionHandler::makeObsolete(bool value)
{
    obsolete = value;
}

void SessionHandler::hashSync()
{
    std::cout<<"[!] Hash Exchange Requested!\n";
    auto hashVectorUidPair = readHashVector();

    auto hashVector = mongocxx->getBlockHash(hashVectorUidPair.first);

    writeHashVector(hashVectorUidPair.first, hashVector);
}

void SessionHandler::writeHashVector(int16_t uid, std::vector<uint32_t> &hashVector)
{
    Protocol::Request::HashExchange hashExchange(uid, hashVector.size());
    if(writeHeader(Protocol::TYPE::HASH_EXCHANGE, sizeof(hashExchange), 1))
    {
        auto wsize = socket->write_some(boost::asio::buffer(&hashExchange, sizeof(hashExchange)));
        std::cout<< "[>>] HashExchange Request Sent, size: "<< wsize<<std::endl;

        uint32_t hashes[hashVector.size()];
        int i{0};
        for(auto hv: hashVector)
            hashes[i++] = hv;

        wsize = socket->write_some(boost::asio::buffer(&hashes, sizeof(uint32_t)*hashExchange.hashQty));
        std::cout<< "[>>] Hashes Request Sent, size: "<< wsize<< " and Qty: "<<hashVector.size() << std::endl;
    }
}

std::pair<int16_t, std::vector<uint32_t> > SessionHandler::readHashVector()
{
        Protocol::Request::HashExchange hashExchange;
        auto rsize = socket->read_some(boost::asio::buffer(&hashExchange, sizeof(hashExchange)));
        std::cout<< "[>>] HashExchange Received, size: "<< rsize<<" Qty: "<< hashExchange.hashQty<<std::endl;
        auto size = sizeof(uint32_t)*hashExchange.hashQty;

        std::vector<uint32_t> hashVector;
        uint32_t hashes[hashExchange.hashQty];

        rsize = socket->read_some(boost::asio::buffer(&hashes, sizeof(uint32_t)*hashExchange.hashQty));
        std::cout<< "[>>] Hashes Received, size: "<< rsize<< " and Qty: "<<hashVector.size() << std::endl;

        for(auto i=0; i< hashExchange.hashQty; i++)
            hashVector.push_back(hashes[i]);

        for(auto h: hashVector)
            std::cout<<h<<" ";
        std::cout<<std::endl;

        return {hashExchange.uid, hashVector};
}

SessionHandler::SessionHandler(std::unique_ptr<boost::asio::ip::tcp::socket> _socket):sessionId(0), obsolete{false}, socket(std::move(_socket))
{
    sessionId = rand();
    std::cout<<"[!] Session Created with id: "<< sessionId<<std::endl;
    mongocxx = MongocxxInterface::get();

    //    socket->send(boost::asio::buffer("You are connected"));
    //    receiveHeader();
}

void SessionHandler::writeAllBlocks()
{
    auto blockVector = mongocxx->getBlocks();

    Protocol::Block blocks[blockVector.size()];
    for(auto i=0; i<blockVector.size(); i++)
        blocks[i] = blockVector[i];

    if(writeHeader(Protocol::TYPE::BLOCKS, sizeof(blocks), blockVector.size()))
    {
        socket->wait(socket->wait_write);

        auto bsize = socket->write_some(boost::asio::buffer(&blocks, sizeof(blocks)));
        std::cout<<"[!] Blocks Writed Bytes: "<< bsize;
    }
}

void SessionHandler::receiveHeader()
{
    Protocol::Header header = readHeader();

    if(header.type == Protocol::TYPE::HEADER)
    {
        std::cout<<"[!] HEADER Recived size: "<< sizeof(header)<<" "<< header.toJson()<< std::endl;
        switch(header.body)
        {
        case Protocol::TYPE::BLOCK:
        {
            readBlocks(header.size, header.quantity);
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
        case Protocol::TYPE::HASH_EXCHANGE:
        {
            hashSync();
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

void SessionHandler::readBlocks(uint16_t size, uint8_t quantity)
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
    Protocol::Request::RequestBlock request;
    auto receive_size = socket->read_some(boost::asio::buffer(&request, sizeof(request)));
    std::cout<<"[!] Request Received Size: "<< receive_size<< " Type: "<< request.type<<std::endl;

    switch(request.type)
    {
    case Protocol::Request::TYPE::BLOCKS_ALL_REQ:
        BlocksAllRequest(request.uid);
        break;
    case Protocol::Request::TYPE::BLOCKS_TAB_REQ:
    {
        BlocksTabRequest(request.uid, std::string(request.tid));
        break;
    }
    case Protocol::Request::TYPE::DELETE_TAB_REQ:
    {
        deleteTab(request.tid);
        break;
    }
    case Protocol::Request::TYPE::RENAME_TAB_REQ:
    {
        renameTabRequest();
        break;
    }
    default:
    {
        std::cout<<"[E] Invalid Request!\n";
        break;
    }
    }
}

void SessionHandler::BlocksTabRequest(int16_t uid, std::string tid)
{
    std::cout<< "Block Tab Requested! for uid: "<< uid <<" tid: "<< tid<<std::endl;
    auto blockVector = mongocxx->getBlocks(uid, tid);

    writeBlocks(blockVector);
}

void SessionHandler::BlocksAllRequest(int16_t uid)
{
    std::cout<< "Block All Requested! for uid: "<< uid<< std::endl;
    auto blockVector = mongocxx->getBlocks("uid", uid);

    writeBlocks(blockVector);
}

bool SessionHandler::isObsolete()
{
    return obsolete;
}

void SessionHandler::start()
{
    receiveHeader();
}

void SessionHandler::deleteTab(std::string tid)
{
    std::cout<<"[!] Removing Tab Requested for tid: "<< tid<<std::endl;

    if(!mongocxx->removeTab(tid))
    {
        std::cout<<"[E] Error in removing tab with tid: "<< tid <<std::endl;
        return;
    }
    std::cout<< "[!] Successfully remove tab with tid: "<< tid<< std::endl;

}

void SessionHandler::renameTabRequest()
{
    Protocol::Request::RenameTabRequest rtr;
    auto rsize = socket->read_some(boost::asio::buffer(&rtr, sizeof(rtr)));
    std::cout<<"[!] Rename Tab Reqested with old tid: "<< rtr.xtid << " new tid: "<< rtr.tid<<std::endl;

    mongocxx->renameTab(rtr.xtid, rtr.tid);
}

void SessionHandler::writeBlocks(std::vector<Protocol::Block> blocksVector)
{
    Protocol::Block blocks[blocksVector.size()];

    for(auto i=0; i< blocksVector.size(); i++)
        blocks[i] = blocksVector[i];

    if(writeHeader(Protocol::TYPE::BLOCKS, sizeof(blocks), blocksVector.size()))
    {
        auto bsize = socket->write_some(boost::asio::buffer(&blocks, sizeof(blocks)));
        std::cout<<"[!] Blocks Data Sent! "<< bsize << " with Quanity: "<< blocksVector.size()<<std::endl;
    }

}

bool SessionHandler::writeHeader(Protocol::TYPE _body, uint16_t _bodySize, uint8_t _quantity)
{
    try
    {
        Protocol::Header header(Protocol::TYPE::HEADER, _body, _bodySize, _quantity);
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

Protocol::Header SessionHandler::readHeader()
{
    Protocol::Header header;
    auto hsize = socket->read_some(boost::asio::buffer(&header, sizeof(header)));
    std::cout<<"[!] Header Received size: "<<hsize<< " Type: " << (int)header.body << std::endl;

    return header;
}


SessionHandler::~SessionHandler()
{
    std::cout<<"[!] Session for socket: "<< socket->remote_endpoint()<<" is disconnected!\n";
    socket->close();
}

