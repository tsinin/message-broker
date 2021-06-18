#include <gtest/gtest.h>

#include <set>
#include <thread>

#include "../src/server/storage.h"

namespace {
class StorageTest : public testing::Test {
public:
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
     * Threads simultaneously push elements to storage (multiple writers) and
     * then simultaneously pop elements (multiple readers), then function checks
     * the equality of expected and final sets of elements
     * @param threadsNumber Number of threads to run
     * @param elementsForThread Number of start elements for each thread
     * @param tagsNumber Number of topics in storage to test
     */
    void testThreadSafetySimple(int threadsNumber, int elementsForThread,
                                int tagsNumber) {
        storage = havka::createMessageStorage(StorageType::RAM,
                                              QueueType::MutexQueue);
        std::vector<std::string> tags;
        tags.reserve(tagsNumber);
        for (int i = 0; i < tagsNumber; ++i) {
            tags.push_back(random_string(5));
        }

        std::vector<std::vector<std::pair<std::string, havka::Message>>> to_storage(
            threadsNumber);
        std::vector<std::vector<std::pair<std::string, havka::Message>>> from_storage(
            threadsNumber);
        havka::Message tmpMessage;
        for (int i = 0; i < threadsNumber; ++i) {
            for (int j = 0; j < elementsForThread; ++j) {
                tmpMessage.setData(random_string(10).c_str(), 10, havka::MessageDataType::Text);
                to_storage[i].push_back(
                    {tags[rand() % tagsNumber], tmpMessage});
            }
        }

        auto to_storage_thread = [&](int index) {
            for (const auto& el : to_storage[index]) {
                storage->postMessage(el.second, el.first);
            }
        };

        auto from_storage_thread = [&](int index) {
            std::optional<havka::Message> el;
            for (const auto& tag : tags) {
                while ((el = storage->getMessageNonblocking(tag)) != std::nullopt) {
                    from_storage[index].emplace_back(tag, *el);
                }
            }
        };

        std::vector<std::thread> threads(threadsNumber);
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i] = std::thread(to_storage_thread, i);
        }
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i].join();
        }

        for (int i = 0; i < threadsNumber; ++i) {
            threads[i] = std::thread(from_storage_thread, i);
        }
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i].join();
        }

        std::set<std::pair<std::string, havka::Message>> all_expected;
        std::set<std::pair<std::string, havka::Message>> all_real;
        for (int i = 0; i < threadsNumber; ++i) {
            for (const auto& el : to_storage[i]) {
                all_expected.insert(el);
            }
            for (const auto& el : from_storage[i]) {
                ASSERT_TRUE(all_real.find(el) == all_real.end());
                all_real.insert(el);
            }
        }
        ASSERT_EQ(all_real.size(), threadsNumber * elementsForThread);
        ASSERT_EQ(all_real, all_expected);
    }

    /**
     * Threads simultaneously push and pop elements (multiple writers + multiple
     * readers). Then function checks the equality of expected and final sets of
     * elements
     * @param threadsNumber Number of threads to run
     * @param elementsForThread Number of start elements for each thread
     * @param tagsNumber Number of topics in storage to test
     */
    void testThreadSafetyDifficult(int threadsNumber, int elementsForThread,
                                   int tagsNumber) {
        storage = havka::createMessageStorage(StorageType::RAM, QueueType::MutexQueue);
        std::vector<std::string> tags;
        tags.reserve(tagsNumber);
        for (int i = 0; i < tagsNumber; ++i) {
            tags.push_back(random_string(5));
        }

        std::vector<std::vector<std::pair<std::string, havka::Message>>> to_storage(
            threadsNumber);
        std::vector<std::vector<std::pair<std::string, havka::Message>>> from_storage(
            threadsNumber);
        havka::Message tmpMessage;
        for (int i = 0; i < threadsNumber; ++i) {
            for (int j = 0; j < elementsForThread; ++j) {
                tmpMessage.setData(random_string(10).c_str(), 10, havka::MessageDataType::Text);
                to_storage[i].push_back(
                    {tags[rand() % tagsNumber], tmpMessage});
            }
        }

        auto to_storage_thread = [&](int index) {
            for (const auto& el : to_storage[index]) {
                storage->postMessage(el.second, el.first);

                std::optional<havka::Message> from_storage_el;
                while ((from_storage_el = storage->getMessageNonblocking(el.first)) ==
                       std::nullopt) {
                }
                from_storage[index].emplace_back(el.first, *from_storage_el);
            }
        };

        std::vector<std::thread> threads(threadsNumber);
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i] = std::thread(to_storage_thread, i);
        }
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i].join();
        }

        std::set<std::pair<std::string, havka::Message>> all_expected;
        std::set<std::pair<std::string, havka::Message>> all_real;
        for (int i = 0; i < threadsNumber; ++i) {
            for (const auto& el : to_storage[i]) {
                all_expected.insert(el);
            }
            for (const auto& el : from_storage[i]) {
                ASSERT_TRUE(all_real.find(el) == all_real.end());
                all_real.insert(el);
            }
        }
        ASSERT_EQ(all_real.size(), threadsNumber * elementsForThread);
        ASSERT_EQ(all_real, all_expected);
    }

    std::shared_ptr<havka::IMessageStorage> storage;
};
}  // namespace

TEST_F(StorageTest, RamMutexStorage_SimpleSingleThreadTest1) {
    storage = havka::createMessageStorage(StorageType::RAM, QueueType::MutexQueue);

    ASSERT_EQ(storage->getMessageNonblocking("tag1"), std::nullopt);
    ASSERT_EQ(storage->getMessageNonblocking("tag2"), std::nullopt);
    havka::Message mes1;
    havka::Message mes2;
    havka::Message mes3;
    mes1.setData("111", 3, havka::MessageDataType::Text);
    mes2.setData("2222", 4, havka::MessageDataType::Binary);
    mes3.setData("33333", 5, havka::MessageDataType::Text);
    storage->postMessage(mes1, "tag1");
    storage->postMessage(mes2, "tag2");
    storage->postMessage(mes3, "tag1");
    ASSERT_EQ(storage->getMessageNonblocking("tag1"), mes1);
    ASSERT_EQ(storage->getMessageNonblocking("tag1"), mes3);
    ASSERT_EQ(storage->getMessageNonblocking("tag2"), mes2);
    ASSERT_EQ(storage->getMessageNonblocking("tag1"), std::nullopt);
    ASSERT_EQ(storage->getMessageNonblocking("tag2"), std::nullopt);
}

TEST_F(StorageTest, RamMutexStorage_SimpleSingleThreadTest2) {
    storage = havka::createMessageStorage(StorageType::RAM, QueueType::MutexQueue);

    ASSERT_EQ(storage->getMessageNonblocking("tag1"), std::nullopt);
    ASSERT_EQ(storage->getMessageNonblocking("tag2"), std::nullopt);
    ASSERT_EQ(storage->getMessageNonblocking("tag3"), std::nullopt);
    havka::Message mes1, mes2, mes3, mes4, mes5, mes6;
    mes1.setData("111", 3, havka::MessageDataType::Text);
    mes2.setData("2222", 4, havka::MessageDataType::Binary);
    mes3.setData("33333", 5, havka::MessageDataType::Text);
    mes4.setData("4", 1, havka::MessageDataType::Binary);
    mes5.setData("55", 2, havka::MessageDataType::Text);
    mes5.setData("666666", 6, havka::MessageDataType::Text);
    storage->postMessage(mes1, "tag1");
    storage->postMessage(mes2, "tag2");
    storage->postMessage(mes3, "tag3");
    storage->postMessage(mes4, "tag1");
    storage->postMessage(mes5, "tag2");
    storage->postMessage(mes6, "tag3");
    ASSERT_EQ(storage->getMessageNonblocking("tag1"), mes1);
    ASSERT_EQ(storage->getMessageNonblocking("tag1"), mes4);
    ASSERT_EQ(storage->getMessageNonblocking("tag2"), mes2);
    ASSERT_EQ(storage->getMessageNonblocking("tag2"), mes5);
    ASSERT_EQ(storage->getMessageNonblocking("tag3"), mes3);
    ASSERT_EQ(storage->getMessageNonblocking("tag3"), mes6);
    ASSERT_EQ(storage->getMessageNonblocking("tag1"), std::nullopt);
    ASSERT_EQ(storage->getMessageNonblocking("tag2"), std::nullopt);
    ASSERT_EQ(storage->getMessageNonblocking("tag3"), std::nullopt);
}

TEST_F(StorageTest, RamMutexStorage_LargeSingleThreadTest) {
    storage = havka::createMessageStorage(StorageType::RAM, QueueType::MutexQueue);

    std::unordered_map<std::string, std::queue<havka::Message>> queue;

    const int TAGS = 100;
    const int START_ITEMS = 1000;
    const int OPERATIONS = 3000000;

    std::vector<std::string> tags;
    tags.reserve(TAGS);
    for (int i = 0; i < TAGS; ++i) {
        tags.push_back(random_string(5));
    }

    for (int i = 0; i < START_ITEMS; ++i) {
        havka::Message message;
        message.setData(random_string(10).c_str(), 10, havka::MessageDataType::Text);
        std::string tag = tags[rand() % TAGS];

        queue[tag].push(message);

        storage->postMessage(message, tag);
    }

    for (int i = 0; i < OPERATIONS; ++i) {
        std::string tag = tags[rand() % TAGS];
        if (rand() % 2) {  // post message
            havka::Message message;
            message.setData(random_string(10).c_str(), 10, havka::MessageDataType::Text);
            queue[tag].push(message);
            storage->postMessage(message, tag);
        } else {  // get message
            if (queue[tag].empty()) {
                ASSERT_EQ(storage->getMessageNonblocking(tag), std::nullopt);
            } else {
                ASSERT_EQ(storage->getMessageNonblocking(tag), queue[tag].front());
                queue[tag].pop();
            }
        }
    }
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedSimpleTest1) {
    testThreadSafetySimple(2, 1000, 100);
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedSimpleTest2) {
    testThreadSafetySimple(2, 600000, 1000);
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedSimpleTest3) {
    testThreadSafetySimple(12, 1000, 100);
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedSimpleTest4) {
    testThreadSafetySimple(12, 100000, 1000);
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedDifficultTest1) {
    testThreadSafetyDifficult(2, 1000, 100);
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedDifficultTest2) {
    testThreadSafetyDifficult(2, 600000, 1000);
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedDifficultTest3) {
    testThreadSafetyDifficult(12, 1000, 100);
}

TEST_F(StorageTest, RamMutexStorage_MultiThreadedDifficultTest4) {
    testThreadSafetyDifficult(12, 100000, 1000);
}