/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dfx_utils.h"
#include "audio_log.h"
#include "cJSON.h"

namespace OHOS {
namespace AudioStandard {

uint32_t DfxUtils::SerializeToUint32(const DfxStatInt32 &data)
{
    uint32_t result = 0;
    const uint32_t BYTE_3_OFFSET = 8;
    const uint32_t BYTE_2_OFFSET = 16;
    const uint32_t BYTE_1_OFFSET = 24;

    result |= ((uint32_t)data.fourthByte) & 0xFF;
    result |= (((uint32_t)data.thirdByte) & 0xFF) << BYTE_3_OFFSET;
    result |= (((uint32_t)data.secondByte) & 0xFF) << BYTE_2_OFFSET;
    result |= (((uint32_t)data.firstByte) & 0xFF) << BYTE_1_OFFSET;

    return result;
}

std::string DfxUtils::SerializeToJSONString(const RendererStats &data)
{
    cJSON *json = cJSON_CreateObject();
    CHECK_AND_RETURN_RET(json != nullptr, std::string{});
    cJSON_AddNumberToObject(json, "sampleRate", static_cast<double>(data.samplingRate));
    cJSON_AddNumberToObject(json, "duration", static_cast<double>(data.duration));
    cJSON_AddNumberToObject(json, "underrunCnt", static_cast<double>(data.underrunCnt));
    cJSON_AddNumberToObject(json, "originalFlag", static_cast<double>(data.originalFlag));
    cJSON_AddNumberToObject(json, "zeroDataPercent", static_cast<double>(data.zeroDataPercent));
    cJSON_AddNumberToObject(json, "frameWritten", static_cast<double>(data.frameWritten));
    char *pChar = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    std::string ret = (pChar != nullptr) ? std::string(pChar) : std::string{};
    cJSON_free(pChar);
    return ret;
}

std::string DfxUtils::SerializeToJSONString(const CapturerStats &data)
{
    cJSON *json = cJSON_CreateObject();
    CHECK_AND_RETURN_RET(json != nullptr, std::string{});
    cJSON_AddNumberToObject(json, "sampleRate", static_cast<double>(data.samplingRate));
    cJSON_AddNumberToObject(json, "duration", static_cast<double>(data.duration));
    char *pChar = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    std::string ret = (pChar != nullptr) ? std::string(pChar) : std::string{};
    cJSON_free(pChar);
    return ret;
}

std::string DfxUtils::SerializeToJSONString(const std::vector<InterruptEffect> &data)
{
    cJSON *jsonArray = cJSON_CreateArray();
    CHECK_AND_RETURN_RET(jsonArray != nullptr, std::string{});
    for (auto &item : data) {
        cJSON *json = cJSON_CreateObject();
        CHECK_AND_CONTINUE(json != nullptr);
        cJSON_AddStringToObject(json, "bundleName", item.bundleName.c_str());
        cJSON_AddNumberToObject(json, "streamUsage", static_cast<double>(item.streamUsage));
        cJSON_AddNumberToObject(json, "appState", static_cast<double>(item.appState));
        cJSON_AddNumberToObject(json, "interruptEvent", static_cast<double>(item.interruptEvent));
        cJSON_AddItemToArray(jsonArray, json);
    }
    char *pChar = cJSON_PrintUnformatted(jsonArray);
    cJSON_Delete(jsonArray);
    std::string ret = (pChar != nullptr) ? std::string(pChar) : std::string{};
    cJSON_free(pChar);
    return ret;
}

std::string DfxUtils::SerializeToJSONString(const std::vector<uint8_t> &data)
{
    cJSON *jsonArray = cJSON_CreateArray();
    CHECK_AND_RETURN_RET(jsonArray != nullptr, std::string{});
    for (auto &item : data) {
        cJSON *json = cJSON_CreateObject();
        CHECK_AND_CONTINUE(json != nullptr);
        cJSON_AddNumberToObject(json, "value", static_cast<double>(item));
        cJSON_AddItemToArray(jsonArray, json);
    }
    char *pChar = cJSON_PrintUnformatted(jsonArray);
    cJSON_Delete(jsonArray);
    std::string ret = (pChar != nullptr) ? std::string(pChar) : std::string{};
    cJSON_free(pChar);
    return ret;
}

std::string DfxUtils::SerializeToJSONString(const std::vector<uint32_t> &data)
{
    cJSON *jsonArray = cJSON_CreateArray();
    CHECK_AND_RETURN_RET(jsonArray != nullptr, std::string{});
    for (auto &item : data) {
        cJSON *json = cJSON_CreateObject();
        CHECK_AND_CONTINUE(json != nullptr);
        cJSON_AddNumberToObject(json, "value", static_cast<double>(item));
        cJSON_AddItemToArray(jsonArray, json);
    }
    char *pChar = cJSON_PrintUnformatted(jsonArray);
    cJSON_Delete(jsonArray);
    std::string ret = (pChar != nullptr) ? std::string(pChar) : std::string{};
    cJSON_free(pChar);
    return ret;
}

std::string DfxUtils::SerializeToJSONString(const std::vector<uint64_t> &data)
{
    cJSON *jsonArray = cJSON_CreateArray();
    CHECK_AND_RETURN_RET(jsonArray != nullptr, std::string{});
    for (auto &item : data) {
        cJSON *json = cJSON_CreateObject();
        CHECK_AND_CONTINUE(json != nullptr);
        cJSON_AddNumberToObject(json, "value", static_cast<double>(item));
        cJSON_AddItemToArray(jsonArray, json);
    }
    char *pChar = cJSON_PrintUnformatted(jsonArray);
    cJSON_Delete(jsonArray);
    std::string ret = (pChar != nullptr) ? std::string(pChar) : std::string{};
    cJSON_free(pChar);
    return ret;
}

std::string DfxUtils::SerializeToJSONString(const std::vector<std::string> &data)
{
    cJSON *jsonArray = cJSON_CreateArray();
    CHECK_AND_RETURN_RET(jsonArray != nullptr, std::string{});
    for (auto &item : data) {
        cJSON *json = cJSON_CreateObject();
        CHECK_AND_CONTINUE(json != nullptr);
        cJSON_AddStringToObject(json, "value", item.c_str());
        cJSON_AddItemToArray(jsonArray, json);
    }
    char *pChar = cJSON_PrintUnformatted(jsonArray);
    cJSON_Delete(jsonArray);
    std::string ret = (pChar != nullptr) ? std::string(pChar) : std::string{};
    cJSON_free(pChar);
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
