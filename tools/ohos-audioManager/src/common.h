#ifndef CLI_COMMON_H
#define CLI_COMMON_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

#define CLI_LOG(fmt, ...) do { \
    fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__); \
} while(0)
#define CLI_ERROR(fmt, ...) do { \
    fprintf(stdout, "[ERROR] " fmt "\n", ##__VA_ARGS__); \
} while(0)

int OutputSuccess(const json& data);
int OutputError(const std::string& code, const std::string& message, const std::string& suggestion = "");

#endif // CLI_COMMON_H