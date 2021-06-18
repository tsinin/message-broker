#ifndef HAVKA_SRC_SERVER_HAVKA_H_
#define HAVKA_SRC_SERVER_HAVKA_H_

#include <string>

#include "server/net.h"
#include "server/server_config.h"

namespace havka {

/// Class is responsible for handling new clients and creating new Connections.
/**
 * Usage example:
 * @code
 *  auto broker = std::make_unique<havka::BrokerServer>(
 *      "127.0.0.1", 9090, havka::StorageType::RAM,
 *      havka::QueueType::MutexQueue, 12, 360);
 *
 *  try {
 *      /// Running server. Blocking call.
 *      broker->run();
 *  } catch (std::exception& e) {
 *      std::cerr << e.what();
 *  }
 * @endcode
 */
class BrokerServer {
public:
    BrokerServer() = delete;

    /**
     * Constructor of BrokerServer instance.
     * Initializes storage and context.
     * @param address Server address
     * @param port Server port
     * @param storageType Type of storage for messages
     * @param queueType Type of queues used in storage
     * @param threads Number of threads (-1 to set it to maximum)
     * @param secondsTimeout Server timeout in seconds (-1 to set it to maximum)
     */
    explicit BrokerServer(const net::ip::address& address, unsigned short port,
                          StorageType storageType, QueueType queueType,
                          int threads = -1, int secondsTimeout = -1);

    /**
     * Destructor of BrokerServer.
     * Joins threads.
     */
    ~BrokerServer();

    /**
     * Creates callbacks on deadline or signal to stop server,
     * creates callback on new client connection and runs threads.
     * Blocks until server will stop.
     * Blocking
     */
    void run();

private:
    std::shared_ptr<IMessageStorage> storage_;

    unsigned int threadsNum_;
    std::vector<std::thread> threads_;

    std::shared_ptr<net::io_context> ioc_;

    net::signal_set signals_;
    tcp::endpoint endpoint_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    net::steady_timer deadline_;
    bool hasTimeout_;

    /**
     * Creates callback on new connection which processes it and
     * creates new callback on new client connection.
     * Non-blocking
     */
    void acceptLoop_();

    /**
     * Creates callback on signals SIGINT and SIGTERM to correct
     * termination of server.
     * Non-blocking
     */
    void waitSignal_();

    /**
     * Creates callback on deadline timeout if it exists to
     * correct termination of server.
     * Non-blocking
     */
    void waitDeadline_();
};

}  // namespace havka

#endif  // HAVKA_SRC_SERVER_HAVKA_H_