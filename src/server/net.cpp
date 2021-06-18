//
// Created by tsinin on 5/30/21.
//

#include "server/net.h"

#include <utility>

namespace havka {

Connection::Connection(tcp::socket socket,
                       std::shared_ptr<IMessageStorage> storage,
                       std::size_t maxBufferSize)
    : socket_(std::move(socket)),
      storage_(std::move(storage)),
      buffer_(new char[maxBufferSize]),
      bufSize_(0),
      maxBufSize_(maxBufferSize),
      waitingAccept_(false),
      getBlock_(false) {}

Connection::~Connection() {
    if (waitingAccept_) {
        LOG_INFO("Accept was not received\n");
        storage_->postMessage(*response_.message, request_.topic);
    }
    delete[] buffer_;
}

void Connection::start() {
    waitingAccept_ = getBlock_ = false;
    readRequest_();
}

void Connection::sendEmergedMessage(const Message &message) {
    assert(request_.type == RequestType::GetMessageBlocking);
    response_.message = message;
    response_.type = ResponseType::GetSuccess;
    serializeResponse_();

    auto self = shared_from_this();

    socket_.async_write_some(boost::asio::buffer(buffer_, bufSize_),
                             [self](boost::system::error_code ec, std::size_t len) {
                                 if (!ec) {
                                     self->waitAccept_();
                                 }
                             });
}

void Connection::deserializeRequest_() {
//    for (int i = 0; i < bufSize_; ++i) {
//        std::cout << std::hex << +buffer_[i] << " ";
//    }
//    std::cout << '\n';
//    std::cout << bufSize_ << '\n';

    std::string buffer_str(buffer_, bufSize_);
    std::stringstream iss(buffer_str);
    cereal::BinaryInputArchive iarchive(iss);
    iarchive(request_);
}

void Connection::serializeResponse_() {
    std::stringstream oss;
    {
        cereal::BinaryOutputArchive oarchive(oss);
        oarchive(response_);
    }
    std::string toBuf = oss.str();
    memcpy(buffer_, toBuf.c_str(), toBuf.length());
    bufSize_ = toBuf.length();
}

void Connection::readRequest_() {
    auto self = shared_from_this();

    socket_.async_read_some(
        boost::asio::buffer(buffer_, maxBufSize_),
        [self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                self->bufSize_ = length;
                self->processRequest_();
            }
        });
}

void Connection::processRequest_() {
    /// deserialize request from buffer_ to request_
    deserializeRequest_();

    LOG_INFO("New request:\n"
             << "...... type: " << getStringFromRequestType(request_.type));

    if (request_.type == RequestType::PostMessageSafe) {
        createPostResponse_();
    } else if (request_.type == RequestType::GetMessageNonblocking ||
               request_.type == RequestType::GetMessageBlocking) {
        createGetResponse_();
        if (getBlock_) {
            LOG_INFO("Connection is blocked");
            return;
        }
    } else {
        createFailureResponse_();
    }
    /// serialize response from response_ to buffer_
    serializeResponse_();

    writeResponse_();
}

void Connection::createGetResponse_() {
    if (request_.type == RequestType::GetMessageNonblocking) {
        auto message = storage_->getMessageNonblocking(request_.topic);
        if (message == std::nullopt) {
            response_.type = ResponseType::EmptyTopic;
        } else {
            response_.type = ResponseType::GetSuccess;
        }
        response_.message = message;
    } else {  /// request_.type == RequestType::GetMessageBlocking
        auto message =
            storage_->getMessageBlocking(request_.topic, shared_from_this());
        if (message == std::nullopt) {
            /// block
            getBlock_ = true;
        } else {
            response_.message = message;
            response_.type = ResponseType::GetSuccess;
        }
    }
}

void Connection::createPostResponse_() {
    if (request_.message == std::nullopt) {
        /// problem
        LOG_INFO("Message in request is empty, shutdown connection??");
        return;
    }
    storage_->postMessage(*request_.message, request_.topic);

    response_.type = ResponseType::PostSuccess;
    response_.message = std::nullopt;
}

void Connection::createFailureResponse_() {
    response_.type = ResponseType::Error;
}

void Connection::writeResponse_() {
    auto self = shared_from_this();

    auto loop_handler = [self](boost::system::error_code ec, std::size_t) {
        if (!ec) {
            self->start();
        }
    };

    auto accept_handler = [self](boost::system::error_code ec, std::size_t) {
        if (!ec) {
            self->waitAccept_();
        }
    };

    if (request_.type == RequestType::PostMessageSafe) {

        socket_.async_write_some(boost::asio::buffer(buffer_, bufSize_),
                                 loop_handler);

    } else if (response_.type == ResponseType::GetSuccess) {

        socket_.async_write_some(boost::asio::buffer(buffer_, bufSize_),
                                 accept_handler);

    } else if (request_.type == RequestType::GetMessageNonblocking) {
        /// and response_.type == ResponseType::EmptyTopic

        socket_.async_write_some(boost::asio::buffer(buffer_, bufSize_),
                                 loop_handler);

    } else {  /// response type is "EmptyTopic" and
              /// request type is "GetMessageBlocking"
        /// waiting message in queue with this topic
    }
}

void Connection::waitAccept_() {
    auto self = shared_from_this();

    waitingAccept_ = true;
    socket_.async_read_some(
        boost::asio::buffer(buffer_, maxBufSize_),
        [self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                self->waitingAccept_ = false;
                self->bufSize_ = length;
                self->deserializeRequest_();
                LOG_INFO("Accept:\n"
                         << "...... "
                         << getStringFromRequestType(self->request_.type) << '\n');

                self->socket_.async_write_some(boost::asio::buffer(self->buffer_, 1),
                                         [self](boost::system::error_code ec, std::size_t length) {
                                           self->start();
                                         });

            }
        });
}

}  // namespace havka