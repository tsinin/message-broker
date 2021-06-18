#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <thread>

#include "../src/server/queue.h"
#include "util.h"

namespace {
class QueueTest : public testing::Test {
public:
    std::shared_ptr<havka::MutexQueue<havka::Message>> m_queue =
        std::make_shared<havka::MutexQueue<havka::Message>>();
    std::shared_ptr<havka::MutexQueue<int>> i_queue =
        std::make_shared<havka::MutexQueue<int>>();

    /**
     * Simple template test with 2 elements
     * @tparam T
     * @param first
     * @param second
     */
    template <typename T>
    void simpleTestTwoElements(const T& first, const T& second) {
        std::shared_ptr<havka::MutexQueue<T>> t_queue =
            std::make_shared<havka::MutexQueue<T>>();

        ASSERT_EQ(t_queue->size(), 0);
        ASSERT_EQ(t_queue->pop(), std::nullopt);
        ASSERT_EQ(t_queue->size(), 0);
        t_queue->push(first);
        ASSERT_EQ(t_queue->size(), 1);
        t_queue->push(second);
        ASSERT_EQ(t_queue->size(), 2);
        ASSERT_EQ(t_queue->pop(), first);
        ASSERT_EQ(t_queue->size(), 1);
        ASSERT_EQ(t_queue->pop(), second);
        ASSERT_EQ(t_queue->size(), 0);
        ASSERT_EQ(t_queue->pop(), std::nullopt);
        ASSERT_EQ(t_queue->size(), 0);
    }

    /**
     * Threads simultaneously push elements (multiple writers) and then
     * simultaneously pop elements (multiple readers), then function checks
     * the equality of expected and final sets of elements
     * @param threadsNumber Number of threads to run
     * @param elementsForThread Number of start elements for each thread
     */
    void testThreadSafetySimple(int threadsNumber, int elementsForThread) {
        std::vector<std::vector<int>> to_queue(threadsNumber);
        std::vector<std::vector<int>> from_queue(threadsNumber);
        for (int i = 0; i < threadsNumber; ++i) {
            for (int j = 0; j < elementsForThread; ++j) {
                to_queue[i].push_back(i * elementsForThread + j);
            }
        }

        auto to_queue_thread = [&](int index) {
            for (int el : to_queue[index]) {
                i_queue->push(el);
            }
        };

        auto from_queue_thread = [&](int index) {
            std::optional<int> el;
            while ((el = i_queue->pop()) != std::nullopt) {
                from_queue[index].push_back(*el);
            }
        };

        std::vector<std::thread> threads(threadsNumber);
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i] = std::thread(to_queue_thread, i);
        }
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i].join();
        }

        for (int i = 0; i < threadsNumber; ++i) {
            threads[i] = std::thread(from_queue_thread, i);
        }
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i].join();
        }

        std::set<int> all_expected;
        std::set<int> all_real;
        for (int i = 0; i < threadsNumber; ++i) {
            for (auto el : to_queue[i]) {
                all_expected.insert(el);
            }
            for (auto el : from_queue[i]) {
                ASSERT_TRUE(all_real.find(el) == all_real.end());
                all_real.insert(el);
            }
        }
        ASSERT_EQ(all_real, all_expected);
    }

    /**
     * Threads simultaneously push and pop elements (multiple writers + multiple
     * readers) Then function checks the equality of expected and final sets of
     * elements
     * @param threadsNumber Number of threads to run
     * @param elementsForThread Number of start elements for each thread
     */
    void testThreadSafetyDifficult(int threadsNumber, int elementsForThread) {
        std::vector<std::vector<int>> to_queue(threadsNumber);
        std::vector<std::vector<int>> from_queue(threadsNumber);
        for (int i = 0; i < threadsNumber; ++i) {
            for (int j = 0; j < elementsForThread; ++j) {
                to_queue[i].push_back(i * elementsForThread + j);
            }
        }

        auto to_queue_thread = [&](int index) {
            for (int to_queue_el : to_queue[index]) {
                i_queue->push(to_queue_el);

                std::optional<int> from_queue_el;
                while ((from_queue_el = i_queue->pop()) == std::nullopt) {
                }
                from_queue[index].push_back(*from_queue_el);
            }
        };

        std::vector<std::thread> threads(threadsNumber);
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i] = std::thread(to_queue_thread, i);
        }
        for (int i = 0; i < threadsNumber; ++i) {
            threads[i].join();
        }

        std::set<int> all_expected;
        std::set<int> all_real;
        for (int i = 0; i < threadsNumber; ++i) {
            for (auto el : to_queue[i]) {
                all_expected.insert(el);
            }
            for (auto el : from_queue[i]) {
                ASSERT_TRUE(all_real.find(el) == all_real.end());
                all_real.insert(el);
            }
        }
        ASSERT_EQ(all_real, all_expected);
    }
};
}  // namespace

TEST_F(QueueTest, MutexQueue_SimpleSingleThreadTemplateTest) {
    simpleTestTwoElements(std::string("abc"), std::string("def"));
    simpleTestTwoElements(123, 456);
    simpleTestTwoElements(1.23, 4.56);
}

TEST_F(QueueTest, MutexQueue_SimpleSingleThreadTest) {
    ASSERT_EQ(m_queue->size(), 0);
    ASSERT_EQ(m_queue->pop(), std::nullopt);
    ASSERT_EQ(m_queue->size(), 0);
    havka::Message mes1, mes2, mes3, mes4, mes5;
    mes1.setData("111", 3, havka::MessageDataType::Text);
    mes2.setData("2222", 4, havka::MessageDataType::Binary);
    mes3.setData("33333", 5, havka::MessageDataType::Text);
    mes4.setData("4", 1, havka::MessageDataType::Binary);
    mes5.setData("55", 2, havka::MessageDataType::Text);
    m_queue->push(mes1);
    ASSERT_EQ(m_queue->size(), 1);
    ASSERT_EQ(m_queue->pop(), mes1);
    ASSERT_EQ(m_queue->size(), 0);
    ASSERT_EQ(m_queue->pop(), std::nullopt);
    ASSERT_EQ(m_queue->size(), 0);
    m_queue->push(mes2);
    ASSERT_EQ(m_queue->size(), 1);
    ASSERT_EQ(m_queue->pop(), mes2);
    ASSERT_EQ(m_queue->size(), 0);
    ASSERT_EQ(m_queue->pop(), std::nullopt);
    ASSERT_EQ(m_queue->size(), 0);
    m_queue->push(mes3);
    ASSERT_EQ(m_queue->size(), 1);
    m_queue->push(mes4);
    ASSERT_EQ(m_queue->size(), 2);
    m_queue->push(mes5);
    ASSERT_EQ(m_queue->size(), 3);
    ASSERT_EQ(m_queue->pop(), mes3);
    ASSERT_EQ(m_queue->size(), 2);
    ASSERT_EQ(m_queue->pop(), mes4);
    ASSERT_EQ(m_queue->size(), 1);
    ASSERT_EQ(m_queue->pop(), mes5);
    ASSERT_EQ(m_queue->size(), 0);
    ASSERT_EQ(m_queue->pop(), std::nullopt);
    ASSERT_EQ(m_queue->size(), 0);
}

TEST_F(QueueTest, MutexQueue_LargeSingleThreadTest) {
    std::queue<int> std_queue;

    for (int i = 0; i < 1000000; ++i) {
        int el = rand();
        std_queue.push(el);
        i_queue->push(el);
    }

    for (int i = 0; i < 10000000; ++i) {
        if (rand() % 2) {  // insert random element
            int el = rand();
            std_queue.push(el);
            i_queue->push(el);
            ASSERT_EQ(std_queue.size(), i_queue->size());
        } else {  // pop element
            if (!std_queue.empty()) {
                ASSERT_EQ(std_queue.front(), i_queue->pop());
                std_queue.pop();
            } else {
                ASSERT_EQ(i_queue->pop(), std::nullopt);
            }
            ASSERT_EQ(std_queue.size(), i_queue->size());
        }
    }
}

TEST_F(QueueTest, MutexQueue_MultiThreadedSimpleTest1) {
    testThreadSafetySimple(2, 1000);
}

TEST_F(QueueTest, MutexQueue_MultiThreadedSimpleTest2) {
    testThreadSafetySimple(2, 600000);
}

TEST_F(QueueTest, MutexQueue_MultiThreadedSimpleTest3) {
    testThreadSafetySimple(12, 1000);
}

TEST_F(QueueTest, MutexQueue_MultiThreadedSimpleTest4) {
    testThreadSafetySimple(12, 100000);
}

TEST_F(QueueTest, MutexQueue_MultiThreadedDifficultTest1) {
    testThreadSafetyDifficult(2, 100);
}

TEST_F(QueueTest, MutexQueue_MultiThreadedDifficultTest2) {
    testThreadSafetyDifficult(2, 600000);
}

TEST_F(QueueTest, MutexQueue_MultiThreadedDifficultTest3) {
    testThreadSafetyDifficult(12, 1000);
}

TEST_F(QueueTest, MutexQueue_MultiThreadedDifficultTest4) {
    testThreadSafetyDifficult(12, 100000);
}