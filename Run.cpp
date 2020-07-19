#include <boost/asio.hpp>
#include "Block.h"
#include "cloudserver.h"
#include "mongocxxinterface.h"

int main(int argc, char **argv)
{
    std::shared_ptr<boost::asio::io_context> ioc = std::make_shared<boost::asio::io_context>();

    CloudServer cloudServer(ioc);
//    auto mi = MongocxxInterface::get();
//    auto v = mi->getBlocks("tid", "Tab B");
//    auto v = mi->getBlocks();
//    std:: cout<< sizeof(Block);
//    std::cout<< block.toJson() << std::endl;

    ioc->run();

    return 0;
}
