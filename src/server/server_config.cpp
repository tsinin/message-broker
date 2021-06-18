#include "server/server_config.h"

#include <yaml-cpp/yaml.h>

#include "util.h"

namespace havka {

ServerConfig::ServerConfig(const std::string &config_path) {
    if (!IsFileExisting(config_path)) {
        LOG_FATAL("ServerConfig file does not exist");
    }

    YAML::Node config = YAML::LoadFile(config_path);
    if (!config || !config.IsMap()) {
        LOG_FATAL("ServerConfig file is incorrect");
    }

    if (!config["endpoint_address"] || !config["endpoint_port"]) {
        LOG_FATAL("ServerConfig does not have address or port");
    }

    if (!config["storage_type"]) {
        LOG_INFO(
            "There is no information about storage type "
            "in configuration file, set to default");
        storageType_ = StorageType::RAM;
    } else {
        storageType_ =
            getStorageTypeFromString(config["storage_type"].as<std::string>());
    }

    if (!config["queue_type"]) {
        LOG_INFO(
            "There is no information about queue type "
            "in configuration file, set to default");
        queueType_ = QueueType::MutexQueue;
    } else {
        queueType_ =
            getQueueTypeFromString(config["queue_type"].as<std::string>());
    }

    if (!config["threads"]) {
        LOG_INFO(
            "There is no information about threads "
            "in configuration file, set to default");
        threadsNumber_ = -1;
    } else {
        threadsNumber_ = config["threads"].as<int>();
    }

    if (!config["timeout"]) {
        LOG_INFO(
            "There is no information about timeout "
            "in configuration file, set to default");
        secondsTimeout_ = -1;
    } else {
        secondsTimeout_ = config["timeout"].as<int>();
    }

    address_ =
        net::ip::make_address(config["endpoint_address"].as<std::string>());
    port_ = config["endpoint_port"].as<unsigned short>();
}

net::ip::address ServerConfig::getAddress() const { return address_; }

unsigned short ServerConfig::getPort() const { return port_; }

StorageType ServerConfig::getStorageType() const { return storageType_; }

QueueType ServerConfig::getQueueType() const { return queueType_; }

int ServerConfig::getThreadsNumber() const { return threadsNumber_; }

int ServerConfig::getTimeout() const { return secondsTimeout_; }

}  // namespace havka