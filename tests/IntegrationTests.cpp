#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <memory>

#include "client/client.h"
#include "server/server.h"

using testing::Eq;

namespace {
class IntegrationTest : public ::testing::Test {
public:
    IntegrationTest() = default;

    /**
     * Joins server thread
     */
    ~IntegrationTest() override { server_thread_->join(); }

    /**
     * Runs server with exact timeout to correctly finish test
     * @param seconds Number of seconds for server to run
     * @param threads Number of threads for server to run
     */
    void runServer(int seconds, int threads = -1) {
        server_thread_ = std::make_shared<std::thread>([threads, seconds] {
            std::make_shared<havka::BrokerServer>(
                net::ip::make_address("127.0.0.1"), 9090, StorageType::RAM,
                QueueType::MutexQueue, threads, seconds)
                ->run();
        });
    }

    /**
     * Generates random readable string with exact length
     * @param length number of elements
     * @return random string with given length
     */
    static std::string random_string(size_t length) {
        auto randchar = []() -> char {
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string str(length, 0);
        std::generate_n(str.begin(), length, randchar);
        return str;
    }

    /**
     * Clients simultaneously send messages to server and then simultaneously
     * get messages from server, then function checks the equality of
     * expected and final sets of elements
     * @param clientsNumber Number of clients to run
     * @param elementsForClient Number of start messages for each thread
     * @param tagsNumber Number of topics to test
     */
    static void testMultiClientSimple(int clientsNumber, int elementsForClient,
                                      int tagsNumber) {
        std::vector<std::string> tags;
        tags.reserve(tagsNumber);
        for (int i = 0; i < tagsNumber; ++i) {
            tags.push_back(random_string(5));
        }

        std::vector<std::vector<std::pair<std::string, havka::Message>>>
            to_server(clientsNumber);
        std::vector<std::vector<std::pair<std::string, havka::Message>>>
            from_server(clientsNumber);
        havka::Message tmpMessage;
        for (int i = 0; i < clientsNumber; ++i) {
            for (int j = 0; j < elementsForClient; ++j) {
                tmpMessage.setData(random_string(10).c_str(), 10,
                                   havka::MessageDataType::Text);
                to_server[i].push_back({tags[rand() % tagsNumber], tmpMessage});
            }
        }

        auto to_storage_thread = [&](int index) {
            std::shared_ptr<havka::BrokerSyncClient> client(
                std::make_shared<havka::BrokerSyncClient>(
                    net::ip::make_address("127.0.0.1"), 9090));
            client->connect();
            for (const auto& el : to_server[index]) {
                client->postMessage(el.second, el.first,
                                    havka::RequestType::PostMessageSafe);
            }
        };

        auto from_storage_thread = [&](int index) {
            std::shared_ptr<havka::BrokerSyncClient> client(
                std::make_shared<havka::BrokerSyncClient>(
                    net::ip::make_address("127.0.0.1"), 9090));
            client->connect();
            std::optional<havka::Message> el;
            for (const auto& tag : tags) {
                while ((el = client->getMessage(
                            tag, havka::RequestType::GetMessageNonblocking)) !=
                       std::nullopt) {
                    from_server[index].emplace_back(tag, *el);
                }
            }
        };

        std::vector<std::thread> threads(clientsNumber);
        for (int i = 0; i < clientsNumber; ++i) {
            threads[i] = std::thread(to_storage_thread, i);
        }
        for (int i = 0; i < clientsNumber; ++i) {
            threads[i].join();
        }

        for (int i = 0; i < clientsNumber; ++i) {
            threads[i] = std::thread(from_storage_thread, i);
        }
        for (int i = 0; i < clientsNumber; ++i) {
            threads[i].join();
        }

        std::set<std::pair<std::string, havka::Message>> all_expected;
        std::set<std::pair<std::string, havka::Message>> all_real;
        for (int i = 0; i < clientsNumber; ++i) {
            for (const auto& el : to_server[i]) {
                all_expected.insert(el);
            }
            for (const auto& el : from_server[i]) {
                ASSERT_TRUE(all_real.find(el) == all_real.end());
                all_real.insert(el);
            }
        }
        ASSERT_EQ(all_real.size(), clientsNumber * elementsForClient);
        ASSERT_EQ(all_real, all_expected);
    }

    /**
     * Clients simultaneously send and get messages from server and then
     * function checks the equality of expected and final sets of elements.
     * Checks GetMessageNonblocking type.
     * @param clientsNumber Number of clients to run
     * @param elementsForClient Number of start messages for each thread
     * @param tagsNumber Number of topics to test
     */
    static void testMultiClientDifficult(int clientsNumber,
                                         int elementsForClient,
                                         int tagsNumber) {
        std::vector<std::string> tags;
        tags.reserve(tagsNumber);
        for (int i = 0; i < tagsNumber; ++i) {
            tags.push_back(random_string(5));
        }

        std::vector<std::vector<std::pair<std::string, havka::Message>>>
            to_server(clientsNumber);
        std::vector<std::vector<std::pair<std::string, havka::Message>>>
            from_server(clientsNumber);
        havka::Message tmpMessage;
        for (int i = 0; i < clientsNumber; ++i) {
            for (int j = 0; j < elementsForClient; ++j) {
                tmpMessage.setData(random_string(10).c_str(), 10,
                                   havka::MessageDataType::Text);
                to_server[i].push_back({tags[rand() % tagsNumber], tmpMessage});
            }
        }

        auto to_storage_thread = [&](int index) {
            std::shared_ptr<havka::BrokerSyncClient> client(
                std::make_shared<havka::BrokerSyncClient>(
                    net::ip::make_address("127.0.0.1"), 9090));
            client->connect();
            for (const auto& el : to_server[index]) {
                client->postMessage(el.second, el.first,
                                    havka::RequestType::PostMessageSafe);

                std::optional<havka::Message> from_storage_el;
                while ((from_storage_el = client->getMessage(
                            el.first,
                            havka::RequestType::GetMessageNonblocking)) ==
                       std::nullopt) {
                }
                from_server[index].emplace_back(el.first, *from_storage_el);
            }
        };

        std::vector<std::thread> threads(clientsNumber);
        for (int i = 0; i < clientsNumber; ++i) {
            threads[i] = std::thread(to_storage_thread, i);
        }
        for (int i = 0; i < clientsNumber; ++i) {
            threads[i].join();
        }

        std::set<std::pair<std::string, havka::Message>> all_expected;
        std::set<std::pair<std::string, havka::Message>> all_real;
        for (int i = 0; i < clientsNumber; ++i) {
            for (const auto& el : to_server[i]) {
                all_expected.insert(el);
            }
            for (const auto& el : from_server[i]) {
                ASSERT_TRUE(all_real.find(el) == all_real.end());
                all_real.insert(el);
            }
        }
        ASSERT_EQ(all_real.size(), clientsNumber * elementsForClient);
        ASSERT_EQ(all_real, all_expected);
    }

    /**
     * Creates multiple read-clients and send-clients, read-clients are getting
     * messages from server with blocking (waiting if queue is empty).
     * Input amount should be equal to output.
     * Then function checks the equality of expected and final sets of elements.
     * @param sendClientsNumber Number of clients which send messages
     * @param readClientsNumber Number of clients which get messages
     * @param elementsForSendClient Number of messages to send for every
     * send-client
     * @param elementsForReadClient Number of messages to get for every
     * get-client
     */
    static void testMultiClientBlocking(int sendClientsNumber,
                                        int readClientsNumber,
                                        int elementsForSendClient,
                                        int elementsForReadClient) {
        ASSERT_EQ(sendClientsNumber * elementsForSendClient,
                  readClientsNumber * elementsForReadClient);

        std::string tag = random_string(5);

        std::vector<std::vector<std::pair<std::string, havka::Message>>>
            to_server(sendClientsNumber);
        std::vector<std::vector<std::pair<std::string, havka::Message>>>
            from_server(readClientsNumber);
        havka::Message tmpMessage;
        for (int i = 0; i < sendClientsNumber; ++i) {
            for (int j = 0; j < elementsForSendClient; ++j) {
                tmpMessage.setData(random_string(1000).c_str(), 1000,
                                   havka::MessageDataType::Text);
                to_server[i].push_back({tag, tmpMessage});
            }
        }

        auto sendingClient = [&](int index) {
            auto client(std::make_shared<havka::BrokerSyncClient>(
                net::ip::make_address("127.0.0.1"), 9090));
            client->connect();
            for (const auto& el : to_server[index]) {
                client->postMessage(el.second, el.first,
                                    havka::RequestType::PostMessageSafe);
            }
        };

        auto readingBlockingClient = [&](int index) {
            auto client(std::make_shared<havka::BrokerSyncClient>(
                net::ip::make_address("127.0.0.1"), 9090));
            client->connect();

            std::optional<havka::Message> from_server_message;
            for (int i = 0; i < elementsForReadClient; ++i) {
                from_server_message = client->getMessage(
                    tag, havka::RequestType::GetMessageBlocking);
                from_server[index].emplace_back(tag, *from_server_message);
            }
        };

        std::vector<std::thread> readThreads(readClientsNumber);
        std::vector<std::thread> sendThreads(sendClientsNumber);
        for (int i = 0; i < readClientsNumber; ++i) {
            readThreads[i] = std::thread(readingBlockingClient, i);
        }
        for (int i = 0; i < sendClientsNumber; ++i) {
            sendThreads[i] = std::thread(sendingClient, i);
        }
        for (auto& thread : readThreads) {
            thread.join();
        }
        for (auto& thread : sendThreads) {
            thread.join();
        }

        std::set<std::pair<std::string, havka::Message>> all_expected;
        std::set<std::pair<std::string, havka::Message>> all_real;

        for (int i = 0; i < readClientsNumber; ++i) {
            for (const auto& el : from_server[i]) {
                ASSERT_TRUE(all_real.find(el) == all_real.end());
                all_real.insert(el);
            }
        }
        for (int i = 0; i < sendClientsNumber; ++i) {
            for (const auto& el : to_server[i]) {
                all_expected.insert(el);
            }
        }
        ASSERT_EQ(all_real.size(), readClientsNumber * elementsForReadClient);
        ASSERT_EQ(all_real, all_expected);
    }

    std::shared_ptr<std::thread> server_thread_;
};
}  // namespace

TEST_F(IntegrationTest, SimpleTest1) {
    runServer(2, 8);
    sleep(1);
    std::shared_ptr<havka::BrokerSyncClient> client(
        std::make_shared<havka::BrokerSyncClient>(
            net::ip::make_address("127.0.0.1"), 9090));
    client->connect();
    havka::Message mes1;
    mes1.setData("111", 3, havka::MessageDataType::Text);
    client->postMessage(mes1, "tag1", havka::RequestType::PostMessageSafe);
    ASSERT_EQ(
        client->getMessage("tag2", havka::RequestType::GetMessageNonblocking),
        std::nullopt);
    ASSERT_EQ(
        *client->getMessage("tag1", havka::RequestType::GetMessageNonblocking),
        mes1);
    ASSERT_EQ(
        client->getMessage("tag1", havka::RequestType::GetMessageNonblocking),
        std::nullopt);
}

TEST_F(IntegrationTest, SimpleTest2) {
    runServer(2, 8);
    sleep(1);
    std::shared_ptr<havka::BrokerSyncClient> client(
        std::make_shared<havka::BrokerSyncClient>(
            net::ip::make_address("127.0.0.1"), 9090));
    client->connect();

    havka::Message mes1, mes2, mes3, mes4, mes5;
    mes1.setData("111", 3, havka::MessageDataType::Text);
    mes2.setData("2222", 4, havka::MessageDataType::Binary);
    mes3.setData("33333", 5, havka::MessageDataType::Text);
    mes4.setData("4", 1, havka::MessageDataType::Binary);
    mes5.setData("55", 2, havka::MessageDataType::Text);

    client->postMessage(mes1, "tag1", havka::RequestType::PostMessageSafe);
    client->postMessage(mes2, "tag2", havka::RequestType::PostMessageSafe);
    client->postMessage(mes3, "tag1", havka::RequestType::PostMessageSafe);
    ASSERT_EQ(
        *client->getMessage("tag1", havka::RequestType::GetMessageNonblocking),
        mes1);
    client->postMessage(mes4, "tag1", havka::RequestType::PostMessageSafe);
    client->postMessage(mes5, "tag2", havka::RequestType::PostMessageSafe);
    ASSERT_EQ(
        *client->getMessage("tag1", havka::RequestType::GetMessageNonblocking),
        mes3);
    ASSERT_EQ(
        *client->getMessage("tag1", havka::RequestType::GetMessageNonblocking),
        mes4);
    ASSERT_EQ(
        client->getMessage("tag1", havka::RequestType::GetMessageNonblocking),
        std::nullopt);
    ASSERT_EQ(
        *client->getMessage("tag2", havka::RequestType::GetMessageNonblocking),
        mes2);
    ASSERT_EQ(
        *client->getMessage("tag2", havka::RequestType::GetMessageNonblocking),
        mes5);
    ASSERT_EQ(
        client->getMessage("tag1", havka::RequestType::GetMessageNonblocking),
        std::nullopt);
}

TEST_F(IntegrationTest, SingleClientStressTest) {
    runServer(6, 11);
    sleep(1);
    std::shared_ptr<havka::BrokerSyncClient> client(
        std::make_shared<havka::BrokerSyncClient>(
            net::ip::make_address("127.0.0.1"), 9090));
    client->connect();

    std::unordered_map<std::string, std::queue<havka::Message>> queue;

    const int TAGS = 100;
    const int START_ITEMS = 1000;
    const int OPERATIONS = 30000;

    std::vector<std::string> tags;
    tags.reserve(TAGS);
    for (int i = 0; i < TAGS; ++i) {
        tags.push_back(random_string(5));
    }

    havka::Message message;
    for (int i = 0; i < START_ITEMS; ++i) {
        message.setData(random_string(10).c_str(), 10,
                        havka::MessageDataType::Text);
        std::string tag = tags[rand() % TAGS];

        queue[tag].push(message);

        client->postMessage(message, tag, havka::RequestType::PostMessageSafe);
    }

    for (int i = 0; i < OPERATIONS; ++i) {
        std::string tag = tags[rand() % TAGS];
        if (rand() % 2) {  // post message
            message.setData(random_string(10).c_str(), 10,
                            havka::MessageDataType::Text);
            queue[tag].push(message);
            client->postMessage(message, tag,
                                havka::RequestType::PostMessageSafe);

        } else {  // get message
            if (queue[tag].empty()) {
                ASSERT_EQ(client->getMessage(
                              tag, havka::RequestType::GetMessageNonblocking),
                          std::nullopt);
            } else {
                ASSERT_EQ(*client->getMessage(
                              tag, havka::RequestType::GetMessageNonblocking),
                          queue[tag].front());
                queue[tag].pop();
            }
        }
    }
}

TEST_F(IntegrationTest, MultiClientSimpleTest1) {
    runServer(3, 10);
    sleep(1);
    testMultiClientSimple(2, 1000, 100);
}

TEST_F(IntegrationTest, MultiClientSimpleTest2) {
    runServer(10, 10);
    sleep(1);
    testMultiClientSimple(2, 20000, 1000);
}

TEST_F(IntegrationTest, MultiClientSimpleTest3) {
    runServer(10, 6);
    sleep(1);
    testMultiClientSimple(6, 20000, 1000);
}

TEST_F(IntegrationTest, MultiClientSimpleTest4) {
    runServer(10, 2);
    sleep(1);
    testMultiClientSimple(10, 10000, 1000);
}

TEST_F(IntegrationTest, MultiClientDifficultTest1) {
    runServer(3, 10);
    sleep(1);
    testMultiClientDifficult(2, 1000, 100);
}

TEST_F(IntegrationTest, MultiClientDifficultTest2) {
    runServer(10, 10);
    sleep(1);
    testMultiClientDifficult(2, 20000, 1000);
}

TEST_F(IntegrationTest, MultiClientDifficultTest3) {
    runServer(10, 6);
    sleep(1);
    testMultiClientDifficult(6, 20000, 1000);
}

TEST_F(IntegrationTest, MultiClientDifficultTest4) {
    runServer(10, 2);
    sleep(1);
    testMultiClientDifficult(10, 10000, 1000);
}

TEST_F(IntegrationTest, MultiClientBlockingTest1) {
    runServer(3, 2);
    sleep(1);
    testMultiClientBlocking(2, 8, 10000, 2500);
}

TEST_F(IntegrationTest, MultiClientBlockingTest2) {
    runServer(11, 2);
    sleep(1);
    testMultiClientBlocking(2, 8, 50000, 12500);
}