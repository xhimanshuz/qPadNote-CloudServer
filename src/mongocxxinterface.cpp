#include "mongocxxinterface.h"

std::shared_ptr<MongocxxInterface> MongocxxInterface::instance;

MongocxxInterface::MongocxxInterface(std::string uri)
{
    client = std::make_shared<mongocxx::client>(mongocxx::uri(("mongodb://localhost:27017")));
    database = (*client)["qPadNote"];
    blockCollection = database["appdata.blocks"];
}

std::shared_ptr<MongocxxInterface> MongocxxInterface::get()
{
    if(!instance)
        instance = std::make_shared<MongocxxInterface>();
    return instance;
}

Protocol::Block MongocxxInterface::getBlock(uint32_t uid, uint64_t _id)
{
    auto value = blockCollection.find_one({});
//    auto value = blockCollection.find_one(bsoncxx::builder::stream::document{} << "_id"<< 10000 << "uid"<< 15432432453242 << bsoncxx::builder::stream::finalize);
    if(!value)
        return Protocol::Block();
    Protocol::Block block(value->view());
    std::cout<< block.toJson();
    return block;
}


std::vector<Protocol::Block> MongocxxInterface::getBlocks()
{
    std::vector<Protocol::Block> vectorBlock;
    auto cursor = blockCollection.find({});
    for(auto doc: cursor)
    {
        Protocol::Block b(doc);
        vectorBlock.push_back(b);
    }
        return vectorBlock;
}

bool MongocxxInterface::putBlock(int64_t _id, const std::string tid, int32_t uid, bool isDone, const std::string title, const std::string substring)
{
//    auto blockValue = bsoncxx::builder::stream::document{}
//            << "_id" << _id
//            << "tid" << tid
//            << "uid" << uid
//            << "isDone" << isDone
//            << "title" << title
//            << "substring" << substring
//            << bsoncxx::builder::stream::finalize;

    Protocol::Block block(_id, tid, uid, isDone, title, substring);
    return putBlock(block);
}



bool MongocxxInterface::putBlock(Protocol::Block &block)
{
    auto blockValue = block.toDocumentValue();

    auto result = blockCollection.find_one(bsoncxx::builder::stream::document() << "_id"<< block._id << bsoncxx::builder::stream::finalize);

    if(result)
    {
        std::cout<<"[!] _ID: "<< block._id<< " Found, Now Updating existing data to Mongo"<<std::endl;
        try
        {
            blockCollection.update_one(bsoncxx::builder::stream::document() <<"_id"<< block._id<< bsoncxx::builder::stream::finalize,
                                        bsoncxx::builder::stream::document()<< "$set"
                                        << bsoncxx::builder::stream::open_document
                                        << "tid"<< block.tid << "uid"<< block.uid<< "isDone"<< block.isDone << "title" << block.title<< "substring"<< block.substring<<
                                        bsoncxx::builder::stream::close_document<<
                                        bsoncxx::builder::stream::finalize);
        }
        catch(mongocxx::exception e)
        {
            std::cout<< "[E] Exception Occured! for _id: "<< block._id << e.code() <<" - "<< e.what()<<std::endl;
            return false;
        }
            std::cout<<"[!] Block Updated: "<< bsoncxx::to_json(blockValue) <<std::endl;
        return true;
    }

    try
    {
        auto result = blockCollection.insert_one(blockValue.view());
    }
    catch(mongocxx::exception e)
    {
        std::cout<< "[E] Exception Occured! for _id: "<< block._id << e.code() <<" - "<< e.what()<<std::endl;
        return false;
    }

    std::cout<<"[!] Block Putted: "<< bsoncxx::to_json(blockValue) <<std::endl;
    return true;
}

bool MongocxxInterface::removeBlock(int64_t _id)
{
    std::cout<<"[!] Removing Block with _id:"<< _id<< std::endl;

    auto result = blockCollection.find_one(bsoncxx::builder::stream::document() << "_id"<< _id << bsoncxx::builder::stream::finalize);
    if(result)
    {
        try
        {
            blockCollection.delete_one(bsoncxx::builder::stream::document() << "_id"<< _id << bsoncxx::builder::stream::finalize);
            std::cout<<"[!] Successfully Removed block with _id: "<< _id<<std::endl;
        }
        catch (mongocxx::exception e)
        {
            std::cout<<"[E] Exception Occured! Error in deleting block with _id: "<< _id<<std::endl;
            return false;
        }
    }
    else
        std::cout<<"[E] Block With _id: "<<_id<< " not found."<<std::endl;
    return true;
}

bool MongocxxInterface::removeBlocks(int32_t uid, std::string tid)
{
    std::cout<<"[!] Removing Block with uid: "<< uid <<" tid: "<< tid<<std::endl;

    auto result = blockCollection.find_one(bsoncxx::builder::stream::document() << "uid"<< uid << "tid" <<tid << bsoncxx::builder::stream::finalize);
    if(result)
    {
        try
        {
            blockCollection.delete_one(bsoncxx::builder::stream::document() << "uid"<< uid << "tid"<< tid << bsoncxx::builder::stream::finalize);
            std::cout<<"[!] Successfully Removed block with uid: "<< uid<<" tid: "<< tid<<std::endl;
        }
        catch (mongocxx::exception e)
        {
            std::cout<<"[E] Exception Occured! Error in deleting block with uid: "<< uid<< "tid"<< tid <<std::endl;
            return false;
        }
    }
    else
        std::cout<<"[E] Block With uid: "<<uid<<" tid: "<< tid<<" not found."<<std::endl;
    return true;
}


std::vector<Protocol::Block> MongocxxInterface::getBlocks(std::string key, int value)
{
    std::vector<Protocol::Block> vectorBlock;
    if(key == "")
    {
        auto cursor = blockCollection.find({});
        for(auto doc: cursor)
        {
            Protocol::Block b(doc);
            vectorBlock.push_back(b);
        }
    }
    else
    {
        auto cursor = blockCollection.find( bsoncxx::builder::stream::document() << key<< value << bsoncxx::builder::stream::finalize);
        for(auto doc: cursor)
        {
            Protocol::Block b(doc);
            vectorBlock.push_back(b);
        }
    }

    return vectorBlock;
}

std::vector<Protocol::Block> MongocxxInterface::getBlocks(int32_t uid, std::string tid)
{

    std::vector<Protocol::Block> vectorBlock;
    auto cursor = blockCollection.find(bsoncxx::builder::stream::document{} << "tid"<< tid << "uid"<< uid << bsoncxx::builder::stream::finalize);

    for(auto doc: cursor)
    {
        Protocol::Block b(doc);
        vectorBlock.push_back(b);
    }

    return vectorBlock;
}

std::vector<Protocol::Block> MongocxxInterface::getBlocks(std::string key, std::string value)
{
    std::vector<Protocol::Block> vectorBlock;
    if(key == "")
    {
        auto cursor = blockCollection.find({});
        for(auto doc: cursor)
        {
            Protocol::Block b(doc);
            vectorBlock.push_back(b);
        }
    }
    else
    {
        auto cursor = blockCollection.find( bsoncxx::builder::stream::document() << key<< value << bsoncxx::builder::stream::finalize);
        for(auto doc: cursor)
        {
            Protocol::Block b(doc);
            vectorBlock.push_back(b);
        }
    }

    return vectorBlock;
}
