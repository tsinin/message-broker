#ifndef HAVKA_SRC_UTIL_H_
#define HAVKA_SRC_UTIL_H_

#include <sys/stat.h>

#include <iostream>

/// Defines for server monitoring (info about new clients, requests and errors)

#ifdef MONITORING

#define LOG_INFO(msg) std::cerr << "[INFO] " << msg << '\n'
#define LOG_WARNING(msg) std::cerr << "[WARNING] " << msg << '\n'
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << '\n'

#else

#define LOG_INFO(msg) 1
#define LOG_WARNING(msg) 1
#define LOG_ERROR(msg) 1

#endif

#define LOG_FATAL(msg)                      \
    std::cerr << "[FATAL] " << msg << '\n'; \
    exit(1)

/// Function for checking if file with given path exists
inline bool IsFileExisting(const std::string &sPath) {
    struct stat buffer;
    return (stat(sPath.c_str(), &buffer) == 0);
}

/// Namespace with utility functions to measure time. Not recommended to use.
/**
 * Namespace with utility functions to measure time.
 * Not recommended to use.
 */
namespace havka::time_measure {

void start1();
void stopAdd1();
double getTime1();

void start2();
void stopAdd2();
double getTime2();

void start3();
void stopAdd3();
double getTime3();

}  // namespace havka::time_measure

#endif  // HAVKA_SRC_UTIL_H_