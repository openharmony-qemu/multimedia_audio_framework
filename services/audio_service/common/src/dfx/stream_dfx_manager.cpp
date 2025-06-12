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
#define LOG_TAG "StreamDfxManager"
#endif

#include "audio_common_log.h"
#include "stream_dfx_manager.h"
#include "media_monitor_manager.h"
#include "event_bean.h"
#include <cinttypes>

namespace {
constexpr uint32_t SESSION_TIMEOUT = 2 * 60 * 60 * 1000; // 2 hours
}

namespace OHOS {
namespace AudioStandard {
StreamDfxManager& StreamDfxManager::GetInstance()
{
    static StreamDfxManager instance;
    return instance;
}

void StreamDfxManager::CheckStreamOccupancy(uint32_t sessionId, const AudioProcessConfig &processConfig, bool isStart)
{
    int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    AUDIO_DEBUG_LOG("check stream occupancy, sessionId %{public}u, isStart %{public}d, currentTime %{public}" PRId64,
        sessionId, isStart, currentTime);
    std::lock_guard<std::mutex> lock(streamMutex_);
    for (auto item = streamRecordMap_.begin(); item != streamRecordMap_.end(); ++item) {
        int64_t startTime = item->second.startTime;
        int64_t lastUploadTime = item->second.lastUploadTime;
        if ((currentTime - startTime) > SESSION_TIMEOUT && (currentTime - lastUploadTime) > SESSION_TIMEOUT) {
            ReportStreamOccupancyTimeout(item->first, startTime, currentTime);
            item->second.lastUploadTime = currentTime;
        }
    }

    if (isStart) {
        StreamRecord streamInfo = {processConfig, currentTime, currentTime};
        streamRecordMap_[sessionId] = streamInfo;
    } else {
        streamRecordMap_.erase(sessionId);
    }
}

void StreamDfxManager::ReportStreamOccupancyTimeout(uint32_t sessionId, int64_t startTime, int64_t currentTime)
{
    auto streamInfo = streamRecordMap_.find(sessionId);
    CHECK_AND_RETURN_LOG(streamInfo != streamRecordMap_.end(), "sessionId not found");
    AudioProcessConfig &processConfig = streamInfo->second.processConfig;
    AUDIO_INFO_LOG("sessionId %{public}u timeout, start time %{public}" PRId64 ", "\
        "current time %{public}" PRId64 ", streamUsage %{public}d, sourcetype %{public}d, uid %{private}d",
        sessionId, startTime, currentTime, processConfig.rendererInfo.streamUsage,
        processConfig.capturerInfo.sourceType, processConfig.appInfo.appUid);

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::STREAM_OCCUPANCY,
        Media::MediaMonitor::EventType::DURATION_AGGREGATION_EVENT);
    CHECK_AND_RETURN_LOG(bean != nullptr, "bean is nullptr");

    bean->Add("IS_PLAYBACK", processConfig.audioMode == AUDIO_MODE_PLAYBACK);
    bean->Add("SESSIONID", static_cast<int32_t>(sessionId));
    bean->Add("UID", processConfig.appInfo.appUid);
    int32_t streamTypeInt = (processConfig.audioMode == AUDIO_MODE_PLAYBACK) ?
        static_cast<int32_t>(processConfig.rendererInfo.streamUsage) :
        static_cast<int32_t>(processConfig.capturerInfo.sourceType);
    bean->Add("STREAM_OR_SOURCE_TYPE", streamTypeInt);
    bean->Add("START_TIME", static_cast<uint64_t>(startTime));
    bean->Add("UPLOAD_TIME", static_cast<uint64_t>(currentTime));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}
} // namespace AudioStandard
} // namespace OHOS