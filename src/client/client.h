#ifndef HAVKA_SRC_CLIENT_CLIENT_H_
#define HAVKA_SRC_CLIENT_CLIENT_H_

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <chrono>
#include <iostream>

#include "client/client_config.h"
#include "message.hpp"
#include "types.hpp"
#include "util.h"

namespace havka {

/// from <boost/asio.hpp>
namespace net = boost::asio;

/// from <boost/asio/ip/tcp.hpp>
using tcp = boost::asio::ip::tcp;

/// Client interface for sending requests to the message broker server.
/**
 * Code example for BrokerSyncClient:
 * @code
 * BrokerSyncClient client(...);
 * if (!client.connect()) {
 *     ... error while connecting ...
 * }
 * ... connection is successful ...
 * Message message1;
 * ... fill message ...
 * if (!client.postMessage(message1, "some tag", RequestType::PostMessageSafe))
 * {
 *      ... error while posting ...
 * }
 * ... posting is successful ...
 * auto message2 = client.getMessage("some other tag",
 *                                   Request::GetMessageNonblocking);
 * @endcode
 */
class BrokerClient {
public:
    BrokerClient() = delete;

    /**
     * Constructs client for message broker.
     * @param serverAddress server's IP address
     * @param serverPort server's port
     * @param maxBufferSize max size for buffer (aka max message size) in bytes
     */
    explicit BrokerClient(const net::ip::address& serverAddress,
                          unsigned short serverPort,
                          std::size_t maxBufferSize = 65536){};

    /**
     * Establishes connection between client and server.
     * Should be used before all requests (postMessage / getMessage).
     * If request goes before this function, nothing successful will be returned
     * @return Returns true on success, false on failure.
     */
    virtual bool connect() = 0;

    /**
     * Sends message to the message broker. Message will be read at most once.
     * @param message Message to be sent
     * @param tag Topic with which message should be sent
     * @param requestType Type of postMessage request
     * @return Returns true on success, false on failure
     */
    virtual bool postMessage(const Message& message, const std::string& tag,
                             RequestType requestType) = 0;

    /**
     * Sends the request to the message broker to get message with exact tag.
     * On success, server will delete the message
     * @param tag Message topic
     * @param getType Type of getMessage request
     * @return returns message on success, std::nullopt on failure
     */
    virtual std::optional<Message> getMessage(const std::string& tag,
                                              RequestType getType) = 0;
};

/// Implementation of interface for message broker client.
/**
 * Code example: @code
 * BrokerSyncClient client(...);
 * if (!client.connect()) {
 *     ... error while connecting ...
 * }
 * ... connection is successful ...
 * Message message1;
 * ... fill message ...
 * if (!client.postMessage(message1, "some tag", RequestType::PostMessageSafe))
 * {
 *      ... error while posting ...
 * }
 * ... posting is successful ...
 * auto message2 = client.getMessage("some other tag",
 *                                   Request::GetMessageNonblocking);
 * @endcode
 */
class BrokerSyncClient : public BrokerClient {
public:
    BrokerSyncClient() = delete;

    /**
     * Constructs client for message broker.
     * @param serverAddress server's IP address
     * @param serverPort server's port
     * @param maxBufferSize max size for buffer
     * (aka approximately max message size) in bytes
     */
    explicit BrokerSyncClient(const net::ip::address& serverAddress,
                              unsigned short serverPort,
                              std::size_t maxBufferSize = 65536);

    /**
     * Destructor for client. Frees buffer.
     */
    ~BrokerSyncClient();

    /**
     * Establishes connection between client and server.
     * Should be used before all requests (postMessage / getMessage).
     * If request goes before this function, nothing successful will be returned
     * @return Returns true on success, false on failure.
     */
    bool connect() override;

    /**
     * Sends message to the message broker. Message will be read at most once.
     * Blocking.
     * @param message Message to be sent
     * @param tag Topic with which message should be sent
     * @param postType Type of postMessage request
     * @return Returns true on success, false on failure
     */
    bool postMessage(const Message& message, const std::string& tag,
                     RequestType postType) override;

    /**
     * Sends the request to the message broker to get message with exact tag.
     * On success, server will delete the message.
     * Blocking.
     * @param tag Message topic
     * @param getType Type of getMessage request
     * @return returns message on success, std::nullopt on failure
     */
    std::optional<Message> getMessage(const std::string& tag,
                                      RequestType getType) override;

private:
    std::shared_ptr<net::io_context> ioc_;

    tcp::endpoint endpoint_;
    tcp::resolver resolver_;
    tcp::socket socket_;
    const boost::asio::ip::basic_resolver_results<tcp> results_;
    boost::system::error_code ec_;

    char* buffer_;
    std::size_t bufSize_;
    std::size_t maxBufferSize_;
    Request request_;
    Response response_;

    bool isConnected_;

    void serializeRequest_();

    void deserializeResponse_();
};

}  // namespace havka

#endif  // HAVKA_SRC_CLIENT_CLIENT_H_