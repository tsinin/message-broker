#include "client/client.h"

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    /// Example of usage BrokerSyncClient and ClientConfig
    std::string config_path{argv[1]};

    /// Initializing config from file path
    havka::ClientConfig config(config_path);

    /// Initializing client. Server address and port are in config object
    auto havka = std::make_unique<havka::BrokerSyncClient>(
        config.getServerAddress(), config.getServerPort());

    /// Connecting to server. Function returns false on failure
    if (!havka->connect()) {
        return 1;
    }

    try {
        /// Initializing message to send and setting data to it
        havka::Message in;
        in.setData("asdf", 4, havka::MessageDataType::Text);

        for (int i = 0; i < 100; ++i) {
            /// Posting message to server. Posting type is required
            havka->postMessage(in, "1234", havka::RequestType::PostMessageSafe);
        }

        std::optional<havka::Message> out;
        for (int i = 0; i < 100; ++i) {
            /// Getting message with exact tag from server.
            /// Call is blocking anyway, blocking in type is about protocol:
            /// GetMessageBlocking - if queue is empty, client waits for message
            /// GetMessageNonblocking - if queue is empty,
            ///                         client returns std::nullopt
            out = havka->getMessage("1234", havka::RequestType::GetMessageBlocking);

            /// out->data is an std::string object with message.
            /// Can be interpreted just as bytes sequence.
        }

    } catch (std::exception& e) {
        std::cerr << e.what();
    }

    return 0;
}