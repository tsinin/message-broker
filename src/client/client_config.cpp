#include "client/client_config.h"

#include <yaml-cpp/yaml.h>

namespace havka {

ClientConfig::ClientConfig(const std::string& config_path) {
    if (!IsFileExisting(config_path)) {
        LOG_FATAL("ClientConfig file does not exist");
    }

    YAML::Node config = YAML::LoadFile(config_path);
    if (!config || !config.IsMap()) {
        LOG_FATAL("ClientConfig file is incorrect");
    }

    if (!config["server_address"] || !config["server_port"]) {
        LOG_FATAL("ClientConfig does not have address or port");
    }
    serverAddress_ =
        net::ip::make_address(config["server_address"].as<std::string>());
    serverPort_ = config["server_port"].as<unsigned short>();
}

net::ip::address ClientConfig::getServerAddress() const {
    return serverAddress_;
}

unsigned short ClientConfig::getServerPort() const { return serverPort_; }

}  // namespace havka