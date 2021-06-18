#ifndef HAVKA_SRC_SERVER_QUEUE_H_
#define HAVKA_SRC_SERVER_QUEUE_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

#include "message.hpp"
#include "net.h"
#include "types.hpp"

namespace havka {

// Forward declaration, definition is in file server/net.h
class Connection;

/// Queue interface.
/**
 * Class IQueue is a queue interface of objects with type T
 * @tparam T Type of elements in queue
 */
template <typename T>
class IQueue {
public:
    IQueue() = default;
    IQueue(const IQueue<T>&) = delete;
    virtual IQueue& operator=(const IQueue<T>&) = delete;

    virtual ~IQueue() = default;

    /**
     * Gets current number of elements in queue
     * @return Number of elements
     */
    virtual unsigned long size() const = 0;

    /**
     * Gets first element in the queue and removes it from the queue if
     * queue is not empty, else returns std::nullopt
     * @return First element or std::nullopt if queue is empty
     */
    virtual std::optional<T> pop() = 0;

    /**
     * Pushes new element to the queue.
     * @param item Element to push to the queue
     */
    virtual void push(const T& item) = 0;
};

/// Implementation of queue interface with mutex. Thread-safe.
/**
 * Class MutexQueue implements IQueue interface with thread-safety using mutex.
 * Thread-safe.
 * @tparam T Type of elements in queue
 */
template <typename T>
class MutexQueue : public IQueue<T> {
public:
    MutexQueue() = default;
    MutexQueue(const MutexQueue<T>&) = delete;
    MutexQueue& operator=(const MutexQueue<T>&) = delete;

    virtual ~MutexQueue() = default;

    /**
     * Gets current number of elements in queue
     * @return Number of elements
     */
    unsigned long size() const override;

    /**
     * Gets first element in the queue and removes it from the queue if
     * queue is not empty, else returns std::nullopt
     * @return First element or std::nullopt if queue is empty
     */
    std::optional<T> pop() override;

    /**
     * Pushes new element to the queue.
     * @param item Element to push to the queue
     */
    void push(const T& item) override;

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
};

/**
 * Creates new message queue with given type
 * @param queueType queue type to create with
 * @return shared pointer to created queue
 */
std::shared_ptr<IQueue<Message>> createMessageQueue(QueueType queueType);

/**
 * Creates new connection queue of shared pointers to connections
 * with given type
 * @param queueType queue type to create with
 * @return shared pointer to created queue
 */
std::shared_ptr<IQueue<std::shared_ptr<Connection>>> createConnectionQueue(
    QueueType queueType);

}  // namespace havka

#endif  // HAVKA_SRC_SERVER_QUEUE_H_