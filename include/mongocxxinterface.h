#ifndef MONGOCXXINTERFACE_H
#define MONGOCXXINTERFACE_H

#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/exception/exception.hpp>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include "Block.h"

class MongocxxInterface
{
    mongocxx::instance mongoInstance{};
    std::shared_ptr<mongocxx::client> client;
    mongocxx::database database;
    mongocxx::collection blockCollection;

public:
    explicit MongocxxInterface(const std::string uri="mongodb://localhost:27017");

    static std::shared_ptr<MongocxxInterface> get();
    static std::shared_ptr<MongocxxInterface> instance;

    Protocol::Block getBlock(uint32_t uid=0, uint64_t _id=0);
    std::vector<Protocol::Block> getBlocks(int32_t uid, std::string tid);

    std::vector<Protocol::Block> getBlocks(std::string key, int value);
    std::vector<Protocol::Block> getBlocks(std::string key, std::string value);

    std::vector<Protocol::Block> getBlocks();

    bool putBlock(int64_t _id, const std::string tid, int32_t uid, bool isDone, const std::string title, const std::string substring);
    bool putBlock(Protocol::Block &block);

    bool removeBlock(int64_t _id);
    bool removeBlocks(int32_t uid, std::string tid);
    bool removeTab(std::string tid);
};

#endif // MONGOCXXINTERFACE_H
