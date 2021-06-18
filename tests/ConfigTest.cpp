#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>

#include "client/client_config.h"
#include "server/server_config.h"

namespace net = boost::asio;  // from <boost/asio.hpp>

using testing::Eq;

namespace {
class ConfigTest : public testing::Test {
public:
    std::shared_ptr<havka::ServerConfig> serverConfig;
    std::shared_ptr<havka::ClientConfig> clientConfig;
};
}  // namespace

TEST_F(ConfigTest, ServerConfig_NonExistingFileTest) {
    ASSERT_DEATH(serverConfig = std::make_shared<havka::ServerConfig>(
                     "non-existing-file.ext"),
                 "\\[FATAL\\] ServerConfig file does not exist");
}

TEST_F(ConfigTest, ServerConfig_AbsentAddressTest) {
    std::ofstream file;
    file.open("absent_address_config.yaml", std::ios::trunc);
    file << "endpoint_port: 5432\n";
    file.close();

    ASSERT_DEATH(serverConfig = std::make_shared<havka::ServerConfig>(
                     "absent_address_config.yaml"),
                 "\\[FATAL\\] ServerConfig does not have address or port");

    std::remove("absent_address_config.yaml");
}

TEST_F(ConfigTest, ServerConfig_AbsentPortTest) {
    std::ofstream file("absent_port_config.yaml", std::ios::trunc);
    file << "endpoint_address: 127.0.0.1\n";
    file.close();

    ASSERT_DEATH(serverConfig = std::make_shared<havka::ServerConfig>(
                     "absent_port_config.yaml"),
                 "\\[FATAL\\] ServerConfig does not have address or port");

    std::remove("absent_port_config.yaml");
}

TEST_F(ConfigTest, ServerConfig_CorrectFieldsTest1) {
    std::ofstream file("config_test_1.yaml", std::ios::trunc);
    file << "endpoint_address: 127.0.0.1\n"
            "endpoint_port: 9090\n"
            "storage_type: ram\n"
            "queue_type: mutex\n"
            "threads: 1\n"
            "timeout: 42\n";
    file.close();

    serverConfig = std::make_shared<havka::ServerConfig>("config_test_1.yaml");
    ASSERT_EQ(serverConfig->getAddress(), net::ip::make_address("127.0.0.1"));
    ASSERT_EQ(serverConfig->getPort(), 9090);
    ASSERT_EQ(serverConfig->getStorageType(), StorageType::RAM);
    ASSERT_EQ(serverConfig->getQueueType(), QueueType::MutexQueue);
    ASSERT_EQ(serverConfig->getThreadsNumber(), 1);
    ASSERT_EQ(serverConfig->getTimeout(), 42);

    std::remove("config_test_1.yaml");
}

TEST_F(ConfigTest, ServerConfig_CorrectFieldsTest2) {
    std::ofstream file("config_test_2.yaml", std::ios::trunc);
    file << "endpoint_address: 25.255.0.130\n"
            "endpoint_port: 0546\n"
            "storage_type: ram\n"
            "queue_type: mutex\n"
            "threads: 555\n"
            "timeout: -1\n";
    file.close();

    serverConfig = std::make_shared<havka::ServerConfig>("config_test_2.yaml");
    ASSERT_EQ(serverConfig->getAddress(),
              net::ip::make_address("25.255.0.130"));
    ASSERT_EQ(serverConfig->getPort(), 0546);
    ASSERT_EQ(serverConfig->getStorageType(), StorageType::RAM);
    ASSERT_EQ(serverConfig->getQueueType(), QueueType::MutexQueue);
    ASSERT_EQ(serverConfig->getThreadsNumber(), 555);
    ASSERT_EQ(serverConfig->getTimeout(), -1);

    std::remove("config_test_2.yaml");
}

TEST_F(ConfigTest, ServerConfig_CorrectFieldsTest3) {
    std::ofstream file("config_test_3.yaml", std::ios::trunc);
    file << "endpoint_address: 0.0.0.0\n"
            "endpoint_port: 0\n"
            "storage_type: ram\n"
            "queue_type: mutex\n"
            "threads: 5\n"
            "timeout: 424242\n";
    file.close();

    serverConfig = std::make_shared<havka::ServerConfig>("config_test_3.yaml");
    ASSERT_EQ(serverConfig->getAddress(), net::ip::make_address("0.0.0.0"));
    ASSERT_EQ(serverConfig->getPort(), 0);
    ASSERT_EQ(serverConfig->getStorageType(), StorageType::RAM);
    ASSERT_EQ(serverConfig->getQueueType(), QueueType::MutexQueue);
    ASSERT_EQ(serverConfig->getThreadsNumber(), 5);
    ASSERT_EQ(serverConfig->getTimeout(), 424242);

    std::remove("config_test_3.yaml");
}

TEST_F(ConfigTest, ServerConfig_DefaultFieldsTest1) {
    std::ofstream file("default_test.yaml", std::ios::trunc);
    file << "endpoint_address: 0.0.0.0\n"
            "endpoint_port: 0\n";
    file.close();

    serverConfig = std::make_shared<havka::ServerConfig>("default_test.yaml");
    ASSERT_EQ(serverConfig->getAddress(), net::ip::make_address("0.0.0.0"));
    ASSERT_EQ(serverConfig->getPort(), 0);
    ASSERT_EQ(serverConfig->getStorageType(), StorageType::RAM);
    ASSERT_EQ(serverConfig->getQueueType(), QueueType::MutexQueue);
    ASSERT_EQ(serverConfig->getThreadsNumber(), -1);
    ASSERT_EQ(serverConfig->getTimeout(), -1);

    std::remove("default_test.yaml");
}

TEST_F(ConfigTest, ClientConfig_NonExistingFileTest) {
    ASSERT_DEATH(clientConfig = std::make_shared<havka::ClientConfig>(
                     "non-existing-file.ext"),
                 "\\[FATAL\\] ClientConfig file does not exist");
}

TEST_F(ConfigTest, ClientConfig_AbsentAddressTest) {
    std::ofstream file;
    file.open("absent_address_config.yaml", std::ios::trunc);
    file << "server_port: 5432\n";
    file.close();

    ASSERT_DEATH(clientConfig = std::make_shared<havka::ClientConfig>(
                     "absent_address_config.yaml"),
                 "\\[FATAL\\] ClientConfig does not have address or port");

    std::remove("absent_address_config.yaml");
}

TEST_F(ConfigTest, ClientConfig_AbsentPortTest) {
    std::ofstream file("absent_port_config.yaml", std::ios::trunc);
    file << "server_address: 127.0.0.1\n";
    file.close();

    ASSERT_DEATH(clientConfig = std::make_shared<havka::ClientConfig>(
                     "absent_port_config.yaml"),
                 "\\[FATAL\\] ClientConfig does not have address or port");

    std::remove("absent_port_config.yaml");
}

TEST_F(ConfigTest, ClientConfig_CorrectFieldsTest1) {
    std::ofstream file("config_test_1.yaml", std::ios::trunc);
    file << "server_address: 127.0.0.1\n"
            "server_port: 9090\n";
    file.close();

    clientConfig = std::make_shared<havka::ClientConfig>("config_test_1.yaml");
    ASSERT_EQ(clientConfig->getServerAddress(),
              net::ip::make_address("127.0.0.1"));
    ASSERT_EQ(clientConfig->getServerPort(), 9090);

    std::remove("config_test_1.yaml");
}