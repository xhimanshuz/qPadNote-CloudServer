#include "Block.h"

using namespace Protocol;


Block::Block(int64_t _id, std::string _tid, const int32_t _uid, bool _isDone, const std::string _title, const std::string _substring, uint32_t _hash): _id(_id),
    isDone(_isDone), uid(_uid), hash{_hash}
{
    std::memset(tid, 0, sizeof(tid));
    std::memset(title, 0, sizeof(title));
    std::memset(substring, 0, sizeof(substring));

    std::strcpy(tid, _tid.c_str());
    std::strcpy(title, _title.c_str());
    std::strcpy(substring, _substring.c_str());
}

Block::Block(): _id{0}, isDone{false}, uid{0}, hash{0}
{
    memset(&tid, '\0', 10);
    memset(&title, '\0', 12);
    memset(&substring, '\0', 64);
}

Block::~Block()
{

}

#ifndef SERVER
Block::Block(TodoBlock &todoBlock): _id{todoBlock.id}, isDone{todoBlock.isToDone}, uid{todoBlock.uid}, hash{todoBlock.hash}
{
    std::memset(tid, 0, sizeof(tid));
    std::memset(title, 0, sizeof(title));
    std::memset(substring, 0, sizeof(substring));

    std::strcpy(tid, todoBlock.tid.c_str());
    std::strcpy(title, todoBlock.title.c_str());
    std::strcpy(substring, todoBlock.subString.c_str());
}

Block &Block::operator=(TodoBlock *todoBlock)
{
    std::memset(tid, 0, sizeof(tid));
    std::memset(title, 0, sizeof(title));
    std::memset(substring, 0, sizeof(substring));

    _id = todoBlock->id;
    uid = todoBlock->uid;

    std::strcpy(tid, todoBlock->tid.c_str());
    std::strcpy(title, todoBlock->title.c_str());
    std::strcpy(substring, todoBlock->subString.c_str());
    hash = todoBlock->hash;
    isDone = todoBlock->isToDone;

    return *this;
}

Block &Block::operator=(TodoBlock &todoBlock)
{
    std::memset(tid, 0, sizeof(tid));
    std::memset(title, 0, sizeof(title));
    std::memset(substring, 0, sizeof(substring));

    _id = todoBlock.id;
    uid = todoBlock.uid;

    std::strcpy(tid, todoBlock.tid.c_str());
    std::strcpy(title, todoBlock.title.c_str());
    std::strcpy(substring, todoBlock.subString.c_str());
    hash = todoBlock.hash;
    isDone = todoBlock.isToDone;

    return *this;
}

#endif

#ifdef SERVER
Block::Block(bsoncxx::document::view block)
{
    fillData(block);
}

bsoncxx::document::value Block::toDocumentValue()
{
    auto blockValue = bsoncxx::builder::stream::document{}
            << "_id" << _id
            << "tid" << tid
            << "uid" << uid
            << "isDone" << isDone
            << "title" << title
            << "substring" << substring
            << "hash" <<(int64_t) hash
            << bsoncxx::builder::stream::finalize;
    return blockValue;
}

bool Block::fillData(bsoncxx::document::view block)
{
//        try
//        {
        // FILL _id
        _id = block["_id"].get_value().get_int64();

        isDone = block["isDone"].get_value().get_bool();

        auto tempStr = block["tid"].get_utf8().value.to_string();
        strcpy(tid, tempStr.c_str());

        tempStr = block["title"].get_utf8().value.to_string();
        strcpy(title, tempStr.c_str());

        tempStr = block["substring"].get_utf8().value.to_string();
        strcpy(substring, tempStr.c_str());

        uid = block["uid"].get_value().get_int32().value;

        hash = block["hash"].get_int64();
//
//        catch(...)
//        {
//            std::cout<<"[E] Error in Filling Block Data.\n";
//            return false;
//        }
        return true;
}

#endif
const std::string Block::toString()
{
    std::stringstream ss;
    ss << "<<==================>>\n"
       << "_id: " << _id << "\n"
       << "uid" << uid << "\n"
       << "isDone: "<< isDone <<"\n"
       << "tid: " << tid << "\n"
       << "title: "<< title << "\n"
       << "substring: "<< substring << "\n"
       << "Hash: "<< hash<<"\n";
    return ss.str();
}

const std::string Block::toJson()
{
    boost::property_tree::ptree json;
    json.put("_id", _id);
    json.put("uid", uid);
    json.put("isDone", isDone);
    json.put("tid", tid);
    json.put("title", title);
    json.put("substring", substring);
    json.put("hash", hash);

    std::stringstream ss;
    boost::property_tree::write_json(ss, json);

    return ss.str();
}
