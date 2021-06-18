#ifndef HAVKA_SRC_TYPES_H_
#define HAVKA_SRC_TYPES_H_

#include "util.h"

/**
 * Enum for type of queue being used.
 */
enum class QueueType {
    MutexQueue,
};

inline QueueType getQueueTypeFromString(const std::string& name) {
    if (name == "mutex") {
        return QueueType::MutexQueue;
    } else {
        LOG_ERROR("Returning QueueType::MutexQueue from string '" << name
                                                                  << "'");
        return QueueType::MutexQueue;
    }
}

inline std::string getStringFromQueueType(QueueType queueType) {
    switch (queueType) {
        case QueueType::MutexQueue:
            return "QueueType::MutexQueue";
        default:
            return "Unknown QueueType";
    }
}

/**
 * Enum for type of storage being used
 */
enum class StorageType {
    RAM,
};

inline StorageType getStorageTypeFromString(const std::string& name) {
    if (name == "ram") {
        return StorageType::RAM;
    } else {
        LOG_ERROR("Returning StorageType::RAM from string '" << name << "'");
        return StorageType::RAM;
    }
}

inline std::string getStringFromStorageType(StorageType storageType) {
    switch (storageType) {
        case StorageType::RAM:
            return "StorageType::RAM";
        default:
            return "Unknown StorageType";
    }
}

#endif  // HAVKA_SRC_TYPES_H_