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
#include "nlohmann/json.hpp"

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
    std::string ret{};
    nlohmann::json json;
    json["sampleRate"] = data.samplingRate;
    json["duration"] = data.duration;
    json["underrunCnt"] = data.underrunCnt;
    json["originalFlag"] = data.originalFlag;
    json["zeroDataPercent"] = data.zeroDataPercent;

    ret = json.dump();
    return ret;
}

std::string DfxUtils::SerializeToJSONString(const CapturerStats &data)
{
    std::string ret{};
    nlohmann::json json;
    json["sampleRate"] = data.samplingRate;
    json["duration"] = data.duration;
    ret = json.dump();

    return ret;
}

std::string DfxUtils::SerializeToJSONString(const std::vector<InterruptEffect> &data)
{
    std::string ret{};
    nlohmann::json jsonArray;
    for (auto &item : data) {
        nlohmann::json json;
        json["bundleName"] = item.bundleName;
        json["streamUsage"] = item.streamUsage;
        json["appState"] = item.appState;
        json["interruptEvent"] = item.interruptEvent;
        jsonArray.push_back(json);
    }

    ret = jsonArray.dump();
    return ret;
}

template<class T>
std::string DfxUtils::SerializeToJSONString(const std::vector<T> &data)
{
    std::string ret{};
    nlohmann::json jsonArray;
    for (auto &item : data) {
        nlohmann::json json;
        json["value"] = item;
        jsonArray.push_back(json);
    }

    ret = jsonArray.dump();
    return ret;
}

template std::string DfxUtils::SerializeToJSONString<uint8_t>(const std::vector<uint8_t> &data);
template std::string DfxUtils::SerializeToJSONString<uint32_t>(const std::vector<uint32_t> &data);
template std::string DfxUtils::SerializeToJSONString<uint64_t>(const std::vector<uint64_t> &data);
template std::string DfxUtils::SerializeToJSONString<std::string>(const std::vector<std::string> &data);
} // namespace AudioStandard
} // namespace OHOS