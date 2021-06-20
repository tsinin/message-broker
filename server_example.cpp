#include "server/server.h"

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    /// Example of usage of havka::ServerConfig and havka::BrokerServer
    std::string config_path{argv[1]};

    /// Initializing ServerConfig object. Takes path to file.
    havka::ServerConfig config(config_path);

    /// Initializing BrokerServer. Takes address, port, storage type,
    /// queue type, number of threads (default is -1, maximum)
    /// and timeout in seconds (default is -1, without timeout)
    auto broker = std::make_unique<havka::BrokerServer>(
        config.getAddress(), config.getPort(), config.getStorageType(),
        config.getQueueType(), config.getThreadsNumber(), config.getTimeout());

    try {
        /// Running server. Blocking call.
        broker->run();
    } catch (std::exception& e) {
        std::cerr << e.what();
    }

    return 0;
}