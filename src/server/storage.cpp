//
// Created by tsinin on 5/30/21.
//

#include "server/storage.h"

#include "server/queue.h"

namespace havka {

RamStorage::RamStorage(QueueType queueType) : queueType_(queueType) {}

void RamStorage::postMessage(const Message &message, const std::string &tag) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (clients_.find(tag) != clients_.end() && clients_[tag]->size() > 0) {
        /// there is a waiting client, send message immediately
        auto client = *clients_[tag]->pop();
        client->sendEmergedMessage(message);
    } else {
        /// push message to the queue
        if (queues_.find(tag) == queues_.end()) {
            queues_.emplace(tag, createMessageQueue(queueType_));
        }
        queues_[tag]->push(message);
    }
}

std::optional<Message> RamStorage::getMessageNonblocking(
    const std::string &tag) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queues_.find(tag) == queues_.end()) {
        LOG_WARNING("There is no such queue with tag '" << tag << "'");
        return std::nullopt;
    }

    auto el = queues_[tag]->pop();

    if (el == std::nullopt) {
        LOG_WARNING("IQueue with tag '" << tag << "' is empty");
    }
    return el;
}

std::optional<Message> RamStorage::getMessageBlocking(
    const std::string &tag, std::shared_ptr<Connection> connection) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (queues_.find(tag) == queues_.end()) {
        LOG_WARNING("There is no such queue with tag '"
                    << tag << "'\n"
                    << "...... Adding client in a queue...");
        if (clients_.find(tag) == clients_.end()) {
            clients_.emplace(tag, createConnectionQueue(queueType_));
        }
        clients_[tag]->push(connection);
        return std::nullopt;
    }

    auto el = queues_[tag]->pop();
    if (el == std::nullopt) {
        LOG_WARNING("IQueue with tag '"
                    << tag << "' is empty\n"
                    << "...... Adding client in a queue...");
        if (clients_.find(tag) == clients_.end()) {
            clients_.emplace(tag, createConnectionQueue(queueType_));
        }
        clients_[tag]->push(connection);
        return std::nullopt;
    } else {
        return el;
    }
}

std::shared_ptr<IMessageStorage> createMessageStorage(StorageType storageType,
                                                      QueueType queueType) {
    switch (storageType) {
        case StorageType::RAM:
            return std::make_shared<RamStorage>(queueType);
    }
}

}  // namespace havka