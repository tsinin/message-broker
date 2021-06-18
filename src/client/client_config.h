#ifndef HAVKA_SRC_CLIENT_CLIENT_CONFIG_H_
#define HAVKA_SRC_CLIENT_CLIENT_CONFIG_H_

#include <boost/asio.hpp>
#include <string>

#include "util.h"

namespace net = boost::asio;  // from <boost/asio.hpp>

namespace havka {

/// Class for reading client config from file.
/**
 * Class is responsible for handling client configuration file and
 * transforming data in code.
 */
class ClientConfig {
public:
    ClientConfig() = delete;

    /**
     * Reads data from configuration file and
     * says if something essential is absent.
     * @param config_path path to configuration file
     */
    explicit ClientConfig(const std::string& config_path);

    /**
     * Returns server IP address (boost::asio::ip::address)
     * @return server IP address
     */
    net::ip::address getServerAddress() const;

    /**
     * Returns server port (unsigned short)
     * @return server port
     */
    unsigned short getServerPort() const;

private:
    net::ip::address serverAddress_;
    unsigned short serverPort_;

    // ...
};

}  // namespace havka

#endif  // HAVKA_SRC_CLIENT_CLIENT_CONFIG_H_