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
#include "audio_stream_info.h"
#include "audio_source_type.h"
#include <cinttypes>
#include <cstdlib>

namespace {
constexpr uint32_t SESSION_TIMEOUT_MS = 2 * 60 * 60 * 1000;
constexpr uint32_t INTERPHONE_CAPTURE_TIMEOUT_MS = 10 * 60 * 1000;
constexpr uint32_t INTERPHONE_RENDER_TIMEOUT_MS = 6 * 60 * 60 * 1000;
constexpr int32_t UID_KEY_LEN = 4;             // "uid:" length
constexpr int32_t ERROR_CODE_KEY_LEN = 10;      // "errorCode:" length
constexpr int32_t ERROR_REASON_KEY_LEN = 12;    // "errorReason:" length
constexpr int32_t TIMESTAMP_KEY_LEN = 10;       // "timestamp:" length
constexpr int32_t DECIMAL_BASE = 10;               // strtol/strtoull base parameter
}

namespace OHOS {
namespace AudioStandard {
StreamDfxManager& StreamDfxManager::GetInstance()
{
    static StreamDfxManager instance;
    return instance;
}

bool StreamDfxManager::IsInterphoneStream(const AudioProcessConfig &processConfig)
{
    if (processConfig.audioMode == AUDIO_MODE_PLAYBACK) {
        return processConfig.rendererInfo.streamUsage == STREAM_USAGE_INTERPHONE;
    } else {
        return processConfig.capturerInfo.sourceType == SOURCE_TYPE_INTERPHONE;
    }
}

void StreamDfxManager::CheckStreamOccupancy(uint32_t sessionId, const AudioProcessConfig &processConfig, bool isStart)
{
    int64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    AUDIO_DEBUG_LOG("check stream occupancy, sessionId %{public}u, isStart %{public}d, currentTime %{public}" PRId64,
        sessionId, isStart, currentTime);
    std::lock_guard<std::mutex> lock(streamMutex_);
    for (auto item = streamRecordMap_.begin(); item != streamRecordMap_.end(); ++item) {
        AudioProcessConfig &config = item->second.processConfig;
        bool isInterphone = IsInterphoneStream(config);
        int64_t startTime = item->second.startTime;
        int64_t lastUploadTime = item->second.lastUploadTime;
        bool isPlayback = (config.audioMode == AUDIO_MODE_PLAYBACK);
        
        uint32_t timeout = SESSION_TIMEOUT_MS;
        if (isInterphone) {
            timeout = isPlayback ? INTERPHONE_RENDER_TIMEOUT_MS : INTERPHONE_CAPTURE_TIMEOUT_MS;
        }
        if ((currentTime - startTime) > timeout && (currentTime - lastUploadTime) > timeout) {
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
    bool isInterphone = IsInterphoneStream(processConfig);
    bool isPlayback = (processConfig.audioMode == AUDIO_MODE_PLAYBACK);
    AUDIO_INFO_LOG("sessionId %{public}u timeout, start time %{public}" PRId64 ", "\
        "current time %{public}" PRId64 ", streamUsage %{public}d, sourcetype %{public}d, uid %{private}d, "\
        "isInterphone %{public}d", sessionId, startTime, currentTime, processConfig.rendererInfo.streamUsage,
        processConfig.capturerInfo.sourceType, processConfig.appInfo.appUid, isInterphone);

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::STREAM_OCCUPANCY,
        Media::MediaMonitor::EventType::DURATION_AGGREGATION_EVENT);
    CHECK_AND_RETURN_LOG(bean != nullptr, "bean is nullptr");

    bean->Add("IS_PLAYBACK", isPlayback);
    bean->Add("SESSIONID", static_cast<int32_t>(sessionId));
    bean->Add("UID", processConfig.appInfo.appUid);
    int32_t streamTypeInt = isPlayback ?
        static_cast<int32_t>(processConfig.rendererInfo.streamUsage) :
        static_cast<int32_t>(processConfig.capturerInfo.sourceType);
    bean->Add("STREAM_OR_SOURCE_TYPE", streamTypeInt);
    bean->Add("START_TIME", static_cast<uint64_t>(startTime));
    bean->Add("UPLOAD_TIME", static_cast<uint64_t>(currentTime));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void StreamDfxManager::SendAudioErrorEvent(int32_t uid, int32_t errorCode, const std::string &erroDesc,
    bool isClient)
{
    AUDIO_INFO_LOG("SendAudioErrorEvent: errorCode=0x%{public}08X, erroDesc=%{public}s",
        errorCode, erroDesc.c_str());
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO,
        Media::MediaMonitor::EventId::UNIFIED_FAULT_CODE,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("ERROR_UID", uid);
    bean->Add("ERROR_CODE", errorCode);
    bean->Add("ERROR_REASON", erroDesc);
    if (isClient) {
        AUDIO_DEBUG_LOG("check send event isClient");
    }
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void StreamDfxManager::GetErrorInfosFromMedia(std::vector<AudioFaultRecord> &errorInfos)
{
    std::vector<std::string> faultRecordStrs;
    // Media::MediaMonitor::MediaMonitorManager::GetInstance().GetUnifiedFaultCodeRecords(faultRecordStrs);
    for (const auto &recordStr : faultRecordStrs) {
        AudioFaultRecord record;
        size_t uidPos = recordStr.find("uid:");
        size_t errorCodePos = recordStr.find("errorCode:");
        size_t errorReasonPos = recordStr.find("errorReason:");
        size_t timestampPos = recordStr.find("timestamp:");
        if (uidPos != std::string::npos) {
            auto uidStr = recordStr.substr(uidPos + UID_KEY_LEN,
                recordStr.find(',', uidPos) - uidPos - UID_KEY_LEN);
            record.uid = static_cast<int32_t>(strtol(uidStr.c_str(), nullptr, DECIMAL_BASE));
        }
        if (errorCodePos != std::string::npos) {
            auto errorCodeStr = recordStr.substr(errorCodePos + ERROR_CODE_KEY_LEN,
                recordStr.find(',', errorCodePos) - errorCodePos - ERROR_CODE_KEY_LEN);
            record.errorCode = static_cast<int32_t>(strtol(errorCodeStr.c_str(), nullptr, DECIMAL_BASE));
        }
        if (errorReasonPos != std::string::npos) {
            record.errorReason = recordStr.substr(errorReasonPos + ERROR_REASON_KEY_LEN,
                recordStr.find(',', errorReasonPos) - errorReasonPos - ERROR_REASON_KEY_LEN);
        }
        if (timestampPos != std::string::npos) {
            record.timestamp = static_cast<uint64_t>(strtoull(
                recordStr.substr(timestampPos + TIMESTAMP_KEY_LEN).c_str(), nullptr, DECIMAL_BASE));
        }
        errorInfos.push_back(record);
    }
}
} // namespace AudioStandard
} // namespace OHOS