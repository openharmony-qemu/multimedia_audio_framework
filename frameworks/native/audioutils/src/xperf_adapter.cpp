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
#define LOG_TAG "XperfAdapter"
#endif

#include "xperf_adapter.h"
#include "xperf_service_client.h"
#include "xperf_service_action_type.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
XperfAdapter& XperfAdapter::GetInstance()
{
    static XperfAdapter instance;
    return instance;
}

bool XperfAdapter::NeedNotifyXperf(StreamUsage usage)
{
    if ((usage == STREAM_USAGE_MUSIC) || (usage == STREAM_USAGE_VOICE_COMMUNICATION) ||
        (usage == STREAM_USAGE_MOVIE)) {
        return true;
    }

    return false;
}

void XperfAdapter::ReportStateChangeEventIfNeed(int32_t eventId, StreamUsage usage, uint32_t sessionId,
    int32_t pid, int32_t uid)
{
    if (!NeedNotifyXperf(usage)) {
        return;
    }
    auto timeNow = std::chrono::system_clock::now();
    auto durationSinceEpochMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow.time_since_epoch());

    const std::string msg = "#UNIQUEID:" + std::to_string(sessionId) +
    "#PID:" + std::to_string(pid) +
    "#BUNDLE_NAME:" + std::to_string(uid) +
    "#HAPPEN_TIME:" + std::to_string(durationSinceEpochMs.count()) +
    "#STATUS:" + std::to_string(eventId);

    OHOS::HiviewDFX::XperfServiceClient::GetInstance().NotifyToXperf(HiviewDFX::DomainId::AUDIO, eventId, msg);
}

void XperfAdapter::ReportFaultEvent(int32_t faultcode, uint32_t uid, uint32_t sessionId)
{
    auto timeNow = std::chrono::system_clock::now();
    auto durationSinceEpochMs = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow.time_since_epoch());

    const std::string msg = "#UNIQUEID:" + std::to_string(sessionId) +
    "#FAULT_ID:" + std::to_string(0) +
    "#FAULT_CODE:" + std::to_string(faultcode) +
    "#HAPPEN_TIME:" + std::to_string(durationSinceEpochMs.count());

    OHOS::HiviewDFX::XperfServiceClient::GetInstance().NotifyToXperf(HiviewDFX::DomainId::AUDIO,
        XPERF_EVENT_FAULT, msg);
}
} // namespace AudioStandard
} // namespace OHOS