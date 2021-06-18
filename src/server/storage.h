#ifndef HAVKA_SRC_SERVER_STORAGE_H_
#define HAVKA_SRC_SERVER_STORAGE_H_

#include <memory>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

#include "message.hpp"
#include "server/net.h"
#include "server/queue.h"
#include "server/server_config.h"
#include "types.hpp"

namespace havka {

// Forward declaration, definition is in file server/net.h
class Connection;

// Forward declaration, definition is in file server/queue.h
template <typename T>
class IQueue;

/// Storage interface for server.
class IMessageStorage {
public:
    /**
     * Posts message to the storage. If topic is empty and
     * there are waiting clients, sends message to one of them
     * @param message message to post
     * @param tag message topic
     */
    virtual void postMessage(const Message& message,
                             const std::string& tag) = 0;

    /**
     * Gets message from the storage. If topic is empty, returns std::nullopt
     * Nonblocking.
     * @param tag message topic
     * @return message or std::nullopt if topic is empty
     */
    virtual std::optional<Message> getMessageNonblocking(
        const std::string& tag) = 0;

    /**
     * Gets message from the storage.
     * Call is nonblocking.
     * If queue is empty, pushes client to the waiting queue.
     * @param tag message topic
     * @param connection clients connection (which is being pushed to waiting
     * queue if needed)
     * @return message or std::nullopt if topic is empty
     */
    virtual std::optional<Message> getMessageBlocking(
        const std::string& tag, std::shared_ptr<Connection> connection) = 0;
};

/// Implementation of storage interface, uses RAM. Thread-safe.
/**
 * Implementation of IMessageStorage interface
 * with RAM unordered maps and mutex.
 * Thread-safe because of mutex.
 */
class RamStorage : public IMessageStorage {
public:
    RamStorage() = delete;

    /**
     * Initializes storage with exact type of queue used.
     * @param queueType queue type to use.
     */
    explicit RamStorage(QueueType queueType);

    /**
     * Posts message to the storage. If topic is empty and
     * there are waiting clients, sends message to one of them
     * @param message message to post
     * @param tag message topic
     */
    void postMessage(const Message& message, const std::string& tag) override;

    /**
     * Gets message from the storage. If topic is empty, returns std::nullopt
     * Nonblocking.
     * @param tag message topic
     * @return message or std::nullopt if topic is empty
     */
    std::optional<Message> getMessageNonblocking(
        const std::string& tag) override;

    /**
     * Gets message from the storage.
     * Call is nonblocking.
     * If queue is empty, pushes client to the waiting queue.
     * @param tag message topic
     * @param connection clients connection (which is being pushed to waiting
     * queue if needed)
     * @return message or std::nullopt if topic is empty
     */
    std::optional<Message> getMessageBlocking(
        const std::string& tag,
        std::shared_ptr<Connection> connection) override;

private:
    std::unordered_map<std::string, std::shared_ptr<IQueue<Message>>> queues_;
    std::unordered_map<std::string,
                       std::shared_ptr<IQueue<std::shared_ptr<Connection>>>>
        clients_;
    QueueType queueType_;
    std::mutex mutex_;
};

/**
 * Creates new message storage of given type with queues of given type
 * @param storageType type of storage to create
 * @param queueType type of queue to use
 * @return shared pointer on new storage
 */
std::shared_ptr<IMessageStorage> createMessageStorage(StorageType storageType,
                                                      QueueType queueType);

}  // namespace havka

#endif  // HAVKA_SRC_SERVER_STORAGE_H_