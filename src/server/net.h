#ifndef HAVKA_SRC_SERVER_NET_H_
#define HAVKA_SRC_SERVER_NET_H_

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <memory>
#include <sstream>

#include "message.hpp"
#include "server/server_config.h"
#include "server/storage.h"

namespace havka {

/// from <boost/asio.hpp>
namespace net = boost::asio;

/// from <boost/asio/ip/tcp.hpp>
using tcp = boost::asio::ip::tcp;

// Forward declaration. Defined in file src/server/storage.h
class IMessageStorage;

/// Class is responsible for handling requests from one client.
/**
 * Class Connection is responsible for handling new client connection:
 * reading request, processing request, forming and sending response
 * to client.
 */
class Connection : public std::enable_shared_from_this<Connection> {
public:
    /**
     * Constructs new connection, gets socket and storage pointer from server
     * @param socket Connection socket
     * @param storage Pointer to message storage
     * @param maxBufferSize maximum of bytes to be sent and received at once
     */
    explicit Connection(tcp::socket socket,
                        std::shared_ptr<IMessageStorage> storage,
                        std::size_t maxBufferSize = 65536);

    /**
     * Destructs connection. If there was GET-request and client
     * didn't confirm the delivery of the message, message returns to the queue
     */
    ~Connection();

    /**
     * Starts connection processing. Creates callbacks on reading request and
     * on timeout deadline.
     */
    void start();

    /**
     * Function used from Storage, when there are waiting clients and
     * somebody posts message
     */
    void sendEmergedMessage(const Message& message);


private:
    std::shared_ptr<IMessageStorage> storage_;
    tcp::socket socket_;
    char* buffer_;
    std::size_t bufSize_;
    std::size_t maxBufSize_;
    Request request_;
    Response response_;
    bool waitingAccept_;
    bool getBlock_;

    void deserializeRequest_();

    void serializeResponse_();

    /**
     * Creates callback on reading request
     */
    void readRequest_();

    /**
     * Processes request after reading
     */
    void processRequest_();

    /**
     * Creates response on GET-request (which gets message
     * from message storage with exact tag)
     */
    void createGetResponse_();

    /**
     * Creates response on POST-request (which posts message
     * to message storage with exact tag)
     */
    void createPostResponse_();


    /**
     * Creates response on unknown request
     */
    void createFailureResponse_();

    /**
     * Creates callback on sending response to the client.
     * Callback closes socket.
     */
    void writeResponse_();


    void waitAccept_();
};

}

#endif  // HAVKA_SRC_SERVER_NET_H_