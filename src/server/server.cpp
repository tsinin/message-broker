//
// Created by tsinin on 5/30/21.
//

#include "server/server.h"

#include "server/storage.h"
#include "types.hpp"
#include "util.h"

namespace havka {
BrokerServer::BrokerServer(const net::ip::address& address, unsigned short port,
                           StorageType storageType, QueueType queueType,
                           int threads, int secondsTimeout)
    : storage_(createMessageStorage(storageType, queueType)),
      threadsNum_(threads > 0 ? threads : std::thread::hardware_concurrency()),
      ioc_(std::make_shared<net::io_context>(threadsNum_)),
      signals_(*ioc_),
      endpoint_(address, port),
      acceptor_(*ioc_, endpoint_),
      socket_(*ioc_),
      deadline_(socket_.get_executor(), std::chrono::seconds(secondsTimeout)),
      hasTimeout_(secondsTimeout > 0) {
    LOG_INFO("Endpoint address: " << address);
    LOG_INFO("Endpoint port: " << port);
    LOG_INFO("Storage type: " << getStringFromStorageType(storageType));
    LOG_INFO("Queue type: " << getStringFromQueueType(queueType));
    LOG_INFO("Threads: " << threadsNum_);
    if (hasTimeout_) {
        LOG_INFO("Timeout: " << secondsTimeout << " seconds");
    } else {
        LOG_INFO("Timeout: disabled");
    }
}

BrokerServer::~BrokerServer() {
    for (auto& thread : threads_) {
        thread.join();
    }
}

void BrokerServer::run() {
    if (hasTimeout_) {
        waitDeadline_();
    }
    waitSignal_();
    acceptLoop_();

    threads_.reserve(threadsNum_ - 1);
    for (int i = 0; i < threadsNum_ - 1; ++i) {
        threads_.emplace_back([this] { ioc_->run(); });
    }
    LOG_INFO("Server is working...\n");
    ioc_->run();
}

void BrokerServer::acceptLoop_() {
    acceptor_.async_accept(socket_, [&](boost::system::error_code ec) {
        if (!ec) {
            std::make_shared<Connection>(std::move(socket_), storage_)->start();
        }
        acceptLoop_();
    });
}

void BrokerServer::waitSignal_() {
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.async_wait(
        [this](boost::system::error_code /* ec */, int /* signo */) {
            LOG_INFO("Stop-signal has been caught");
            LOG_INFO("Stopping server...");
            ioc_->stop();
        });
}

void BrokerServer::waitDeadline_() {
    deadline_.async_wait([this](boost::system::error_code /* ec */) {
        LOG_INFO("Deadline has been expired");
        LOG_INFO("Stopping server...");
        ioc_->stop();
    });
}

}  // namespace havka