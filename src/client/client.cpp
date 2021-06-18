#include "client/client.h"

namespace havka {

BrokerSyncClient::BrokerSyncClient(const net::ip::address &serverAddress,
                                 unsigned short serverPort,
                                 std::size_t maxBufferSize)
    : BrokerClient(serverAddress, serverPort),
      endpoint_(serverAddress, serverPort),
      ioc_(std::make_shared<net::io_context>()),
      resolver_(*ioc_),
      socket_(*ioc_),
      results_(resolver_.resolve(endpoint_)),
      buffer_(new char[maxBufferSize]),
      bufSize_(0),
      maxBufferSize_(maxBufferSize),
      isConnected_(false) {}

BrokerSyncClient::~BrokerSyncClient() {
    delete[] buffer_;
    socket_.shutdown(tcp::socket::shutdown_both);
    socket_.close();
}

bool BrokerSyncClient::connect() {
    net::connect(socket_, results_, ec_);
    if (!ec_) {
        isConnected_ = true;
        return true;
    } else {
        return false;
    }
}

bool BrokerSyncClient::postMessage(const Message &message,
                                  const std::string &tag,
                                  RequestType postType) {
    if (!isConnected_) {
        return false;
    }

    if (postType != RequestType::PostMessageSafe) {
        request_.type = RequestType::PostMessageSafe;
    } else {
        request_.type = postType;
    }
    request_.message = message;
    request_.topic = tag;

    serializeRequest_();
    net::write(socket_, boost::asio::buffer(buffer_, bufSize_), ec_);
    if (ec_) {
        return false;
    }

    bufSize_ =
        socket_.read_some(boost::asio::buffer(buffer_, maxBufferSize_), ec_);
    if (ec_) {
        return false;
    }
    deserializeResponse_();

    return response_.type == ResponseType::PostSuccess;
}

std::optional<Message> BrokerSyncClient::getMessage(const std::string &tag,
                                                   RequestType getType) {
    if (!isConnected_) {
        return std::nullopt;
    }

    if (getType != RequestType::GetMessageNonblocking &&
        getType != RequestType::GetMessageBlocking) {
        request_.type = RequestType::GetMessageNonblocking;
    } else {
        request_.type = getType;
    }
    request_.message = std::nullopt;
    request_.topic = tag;

    serializeRequest_();
    net::write(socket_, boost::asio::buffer(buffer_, bufSize_), ec_);
    if (ec_) {
        return std::nullopt;
    }

    bufSize_ =
        socket_.read_some(boost::asio::buffer(buffer_, maxBufferSize_), ec_);
    if (ec_) {
        return std::nullopt;
    }
    deserializeResponse_();

    request_.message = std::nullopt;
    if (response_.type == ResponseType::GetSuccess &&
        response_.message != std::nullopt) {
        request_.type = RequestType::DeliveryConfirmation;

        serializeRequest_();
        net::write(socket_, boost::asio::buffer(buffer_, bufSize_), ec_);
        if (ec_) {
            return std::nullopt;
        }

        bufSize_ = net::read(socket_, boost::asio::buffer(buffer_, 1), ec_);
        if (ec_) {
            return std::nullopt;
        }

        return response_.message;
    } else {  /// else send NOTHING and server will push task back to queue
        return std::nullopt;
    }
}

void BrokerSyncClient::serializeRequest_() {
    std::stringstream oss;
    {
        cereal::BinaryOutputArchive oarchive(oss);
        oarchive(request_);
    }
    std::string toBuf = oss.str();
    memcpy(buffer_, toBuf.c_str(), toBuf.length());
    bufSize_ = toBuf.length();

    //    std::cout << bufSize_ << '\n';
    //    for (int i = 0; i < bufSize_; ++i) {
    //        std::cout << std::hex << +buffer_[i] << " ";
    //    }
    //    std::cout << '\n';
}

void BrokerSyncClient::deserializeResponse_() {
    std::string buffer_str(buffer_, bufSize_);
    std::stringstream iss(buffer_str);
    cereal::BinaryInputArchive iarchive(iss);
    iarchive(response_);
}

}  // namespace havka