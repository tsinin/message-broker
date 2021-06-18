#ifndef HAVKA_MESSAGE_HPP
#define HAVKA_MESSAGE_HPP

#include <cereal/archives/binary.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <chrono>
#include <optional>
#include <string>

/// Namespace with all classes of havka library
namespace havka {

/// Type of data in the message.
enum MessageDataType { Text, Binary };

/// structure for message interface, stores data and its type
struct Message {
    /// Type of data in the message.
    MessageDataType dataType{Binary};

    /// Data of message. Can be readable or not.
    std::string data{0};

    /**
     * Function to set data  to the message from char*
     * @param data_ data
     * @param size length of data in bytes
     * @param dataType_ data type
     */
    void setData(const char* data_, std::size_t size,
                 MessageDataType dataType_) {
        dataType = dataType_;
        data = std::string(data_, size);
    }

    /**
     * Technical operator to using set<Message>
     * @param rhs second message
     * @return
     */
    bool operator<(const Message& rhs) const {
        if (dataType < rhs.dataType) return true;
        if (rhs.dataType < dataType) return false;
        return data < rhs.data;
    }

    /**
     * Technical operator for using unordered_set<Message>
     * @param rhs second message
     * @return
     */
    bool operator==(const Message& rhs) const {
        return dataType == rhs.dataType && data == rhs.data;
    }
    /**
     * Technical operator for using unordered_set<Message>
     * @param rhs second message
     * @return
     */
    bool operator!=(const Message& rhs) const { return !(rhs == *this); }

    /**
     * Technical function to use cereal library for packing Message into archive
     * @tparam Archive archive type
     * @param ar archive
     */
    template <class Archive>
    void serialize(Archive& ar) {
        ar(dataType, data);
    }
};

/// Enum for request type
enum RequestType {
    /// Awaiting the confirmation from broker that
    /// message is posted without errors
    PostMessageSafe,

    /// If queue is empty, wait for message
    GetMessageBlocking,

    /// If queue is empty, return
    GetMessageNonblocking,

    /// Being sent to server after
    /// every response with Message
    DeliveryConfirmation
};

/**
 * Get string name of request type from RequestType object
 * @param requestType
 * @return string name of type
 */
inline std::string getStringFromRequestType(RequestType requestType) {
    switch (requestType) {
        case PostMessageSafe:
            return "RequestType::PostMessageSafe";
        case GetMessageBlocking:
            return "RequestType::GetMessageBlocking";
        case GetMessageNonblocking:
            return "RequestType::GetMessageNonblocking";
        case DeliveryConfirmation:
            return "RequestType::DeliveryConfirmation";
        default:
            return "Unknown type";
    }
}

/// Enum for response type
enum ResponseType {
    /// Post request was successfully processed
    PostSuccess,

    /// Being sent only if posting was safe
    /// (RequestType::PostMessageSafe) and error occured
    ErrorWhilePosting,

    /// Get request was successfully processed
    GetSuccess,

    /// If get-request was blocking, connection is alive and
    /// waiting for a message.
    /// If get-request was non-blocking, std::nullopt is returned
    EmptyTopic,

    /// Unknown error
    Error
};

/// Structure of client request
/**
 * Consists of all information about request in format corresponding to
 * messaging protocol.
 */
struct Request {
    /// Message in request. Can be empty (for 'GetMessage*' requests) so it is
    /// optional.
    std::optional<Message> message;

    /// Topic. Not empty for PostMessage* and GetMessage* requests
    std::string topic;

    /// Request type corresponding to messaging protocol.
    RequestType type;

    /**
     * Technical function to use cereal library for packing Message into archive
     * @tparam Archive archive type
     * @param ar archive
     */
    template <class Archive>
    void serialize(Archive& ar) {
        ar(message, topic, type);
    }
};

/// Structure of server response
/**
 * Consists of all information about response in format corresponding to
 * messaging protocol.
 */
struct Response {
    /// Message in response. Can be empty (for POST-responses) so it is
    /// optional.
    std::optional<Message> message;

    /// Response main message corresponding to messaging protocol.
    ResponseType type{ResponseType::Error};

    /**
     * Technical function to use cereal library for packing Message into archive
     * @tparam Archive archive type
     * @param ar archive
     */
    template <class Archive>
    void serialize(Archive& ar) {
        ar(message, type);
    }
};

}  // namespace havka

#endif  // HAVKA_MESSAGE_HPP
