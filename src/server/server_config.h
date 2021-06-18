#ifndef HAVKA_SRC_SERVER_SERVER_CONFIG_H_
#define HAVKA_SRC_SERVER_SERVER_CONFIG_H_

#include <boost/asio.hpp>
#include <string>

#include "types.hpp"

namespace net = boost::asio;  // from <boost/asio.hpp>

namespace havka {

/// Class for reading server config from file.
/**
 * Class is responsible for handling server configuration file and
 * transforming data in code.
 */
class ServerConfig {
public:
    ServerConfig() = delete;

    /**
     * Reads data from configuration file and
     * says if something essential is absent.
     * @param config_path path to configuration file
     */
    explicit ServerConfig(const std::string& config_path);

    /**
     * Returns server IP address (boost::asio::ip::address)
     * @return server IP address
     */
    net::ip::address getAddress() const;

    /**
     * Returns server port (unsigned short)
     * @return server port
     */
    unsigned short getPort() const;

    /**
     * Returns storage type
     * @return storage type
     */
    StorageType getStorageType() const;

    /**
     * Returns queue type
     * @return queue type
     */
    QueueType getQueueType() const;

    /**
     * Returns number of threads
     * @return number of threads
     */
    int getThreadsNumber() const;

    /**
     * Returns server timeout in seconds
     * @return server timeout in seconds
     */
    int getTimeout() const;

private:
    net::ip::address address_;
    unsigned short port_;
    StorageType storageType_;
    QueueType queueType_;
    int threadsNumber_;
    int secondsTimeout_;

    // ...
};
}  // namespace havka

#endif  // HAVKA_SRC_SERVER_SERVER_CONFIG_H_