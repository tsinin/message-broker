//
// Created by tsinin on 5/30/21.
//

#include "server/queue.h"
#include "server/net.h"

namespace havka {

template <typename T>
unsigned long MutexQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

template <typename T>
std::optional<T> MutexQueue<T>::pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return std::nullopt;
    }
    T tmp = queue_.front();
    queue_.pop();
    return tmp;
}

template <typename T>
void MutexQueue<T>::push(const T& item) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(item);
}

std::shared_ptr<IQueue<Message>> createMessageQueue(QueueType queueType) {
    switch (queueType) {
        case QueueType::MutexQueue: {
            return std::make_shared<MutexQueue<Message>>();
        }
    }
}

std::shared_ptr<IQueue<std::shared_ptr<Connection>>> createConnectionQueue(QueueType queueType) {
    switch (queueType) {
        case QueueType::MutexQueue: {
            return std::make_shared<MutexQueue<std::shared_ptr<Connection>>>();
        }
    }
}


/**
 * Templates for ability of using MutexQueue with different
 */

template unsigned long MutexQueue<std::string>::size() const;
template std::optional<std::string> MutexQueue<std::string>::pop();
template void MutexQueue<std::string>::push(const std::string&);

template unsigned long MutexQueue<int>::size() const;
template std::optional<int> MutexQueue<int>::pop();
template void MutexQueue<int>::push(const int&);

template unsigned long MutexQueue<double>::size() const;
template std::optional<double> MutexQueue<double>::pop();
template void MutexQueue<double>::push(const double&);

template unsigned long MutexQueue<Message>::size() const;
template std::optional<Message> MutexQueue<Message>::pop();
template void MutexQueue<Message>::push(const Message&);

template unsigned long MutexQueue<std::shared_ptr<Connection>>::size() const;
template std::optional<std::shared_ptr<Connection>>
MutexQueue<std::shared_ptr<Connection>>::pop();
template void MutexQueue<std::shared_ptr<Connection>>::push(
    const std::shared_ptr<Connection>&);

}  // namespace havka