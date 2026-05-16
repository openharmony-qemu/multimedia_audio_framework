#include "common.h"
#include <iostream>

int OutputSuccess(const json& data) {
    json response;
    response["type"] = "result";
    response["status"] = "success";
    response["data"] = data;
    std::cout << response.dump() << std::endl;
    return 0;
}

int OutputError(const std::string& code, const std::string& message, const std::string& suggestion) {
    json response;
    response["type"] = "result";
    response["status"] = "failed";
    response["errCode"] = code;
    response["errMsg"] = message;
    response["suggestion"] = suggestion;
    std::cout << response.dump() << std::endl;
    return 1;
}