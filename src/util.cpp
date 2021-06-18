#include "util.h"

#include <chrono>

namespace havka::time_measure {

std::chrono::time_point<std::chrono::high_resolution_clock> begin1;
std::chrono::time_point<std::chrono::high_resolution_clock> begin2;
std::chrono::time_point<std::chrono::high_resolution_clock> begin3;
std::chrono::time_point<std::chrono::high_resolution_clock> end1;
std::chrono::time_point<std::chrono::high_resolution_clock> end2;
std::chrono::time_point<std::chrono::high_resolution_clock> end3;

double time1;
double time2;
double time3;

void start1() {
    begin1 = std::chrono::high_resolution_clock::now();
}

void start2() {
    begin2 = std::chrono::high_resolution_clock::now();
}

void start3() {
    begin3 = std::chrono::high_resolution_clock::now();
}


void stopAdd1() {
    end1 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1);
    time1 += (double)duration.count() / 1000000;
}

void stopAdd2() {
    end2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end2 - begin2);
    time2 += (double)duration.count() / 1000000;
}

void stopAdd3() {
    end3 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end3 - begin3);
    time3 += (double)duration.count() / 1000000;
}


double getTime1() {
    return time1;
}

double getTime2() {
    return time2;
}

double getTime3() {
    return time3;
}

}   // namespace havka::time_measure