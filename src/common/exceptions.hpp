#pragma once

#include <stdexcept>
#include <string>

class ClusterSizeException : public std::runtime_error {
public:
    explicit ClusterSizeException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class StaleDataException : public std::runtime_error {
public:
    explicit StaleDataException(const std::string& msg)
        : std:: runtime_error(msg) {}
};