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

#ifndef LOG_TAG
#define LOG_TAG "AudioEndpointInner"
#endif

#include "audio_endpoint_sink_adapter.h"
#include "audio_utils.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {

bool AudioEndpointSinkAdapter::IsOtherEndpointRunning(const uint32_t fastRenderId, const EndpointName &key)
{
    std::lock_guard<std::mutex> lock(checkerOperationMapMutex_);
    auto fastRenderIt = operationMap.find(fastRenderId);
    if (fastRenderIt == operationMap.end()) {
        return false;
    }
    const auto &operationList = fastRenderIt->second;
    for (const auto &pair : operationList) {
        if (pair.first != key && pair.second == AudioEndpoint::EndpointStatus::RUNNING) {
            return true;
        }
    }
    return false;
}

void AudioEndpointSinkAdapter::AddOperation(const uint32_t fastRenderId, const EndpointName &key,
    AudioEndpoint::EndpointStatus status)
{
    std::lock_guard<std::mutex> lock(checkerOperationMapMutex_);
    operationMap[fastRenderId].emplace_back(std::pair<EndpointName, AudioEndpoint::EndpointStatus>(key, status));
}

void AudioEndpointSinkAdapter::RemoveOperation(const uint32_t fastRenderId)
{
    std::lock_guard<std::mutex> lock(checkerOperationMapMutex_);
    auto fastRenderIt = operationMap.find(fastRenderId);
    if (fastRenderIt != operationMap.end()) {
        operationMap.erase(fastRenderIt);
    }
}

void AudioEndpointSinkAdapter::UpdateStatus(const uint32_t fastRenderId, const EndpointName &key,
    const AudioEndpoint::EndpointStatus &newStatus)
{
    std::lock_guard<std::mutex> lock(checkerOperationMapMutex_);
    auto fastRenderIt = operationMap.find(fastRenderId);
    if (fastRenderIt != operationMap.end()) {
        auto &operationList = fastRenderIt->second;
        for (auto &pair : operationList) {
            if (pair.first == key) {
                pair.second = newStatus;
                break;
            }
        }
    }
}
}
}