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

#undef LOG_TAG
#define LOG_TAG "AudioStreamConcurrencyDetector"

#include "audio_stream_concurrency_detector.h"

#include "hisysevent.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace AudioStandard {

void AudioStreamConcurrencyDetector::ReportHisysEvent(struct AudioStreamConcurrDetectorReportInfo &info)
{
    AUDIO_WARNING_LOG("ReportHisysEvent, uid is %{public}d, appname is %{public}s, streamUsage is %{public}u",
        info.uid, info.appName.c_str(), info.usage);

    for (const auto streamId : info.streamIds) {
        info.startTimes.push_back(streamConcurrInfoMap_[info.uid][info.usage][streamId].startTime);
        info.updateTimes.push_back(streamConcurrInfoMap_[info.uid][info.usage][streamId].updateTime);
    }

    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "STREAM_CONCURRENCY_DETECTOR",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", static_cast<uint32_t>(info.uid),
        "APPNAME", info.appName,
        "STREAMUSAGE", static_cast<uint32_t>(info.usage),
        "STREAMIDS", info.streamIds,
        "STARTTIME", info.startTimes,
        "UPDATETIME", info.updateTimes);

    CHECK_AND_RETURN_LOG(!ret, "write event fail: AUDIO_STREAM_CONCURRENCY_DETECTOR, ret = %{public}d", ret);
}

void AudioStreamConcurrencyDetector::CheckIsOtherStreamRunning(const AudioProcessConfig &config,
    const uint32_t streamId)
{
    struct AudioStreamConcurrDetectorReportInfo info {
        .uid = config.appInfo.appUid,
        .usage = config.rendererInfo.streamUsage,
    };

    for (auto p : streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage]) {
        if ((p.second.updateTime - p.second.startTime >= threshold) && (p.first != streamId)) {
            info.streamIds.push_back(p.first);
        }

        CHECK_AND_BREAK_LOG(info.streamIds.size() < maxStreamNums, "over max stream nums, break");
    }

    CHECK_AND_RETURN_LOG(info.streamIds.size() >= 1, "no other running stream");

    info.appName = GetBundleNameByToken(config.appInfo.appTokenId);
    info.streamIds.push_back(streamId);

    ReportHisysEvent(info);
}

void AudioStreamConcurrencyDetector::UpdateRecord(const AudioProcessConfig &config, const uint32_t streamId)
{
    streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage][streamId].updateTime =
        static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count());

    if (streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage][streamId].startTime == 0) {
        streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage][streamId].startTime =
            streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage][streamId].updateTime;
    }
}

void AudioStreamConcurrencyDetector::UpdateWriteTime(const AudioProcessConfig &config, const uint32_t streamId)
{
    CHECK_AND_RETURN((config.audioMode != AUDIO_MODE_RECORD) && isEnabled_);

    std::lock_guard<std::mutex> lock(concurrencyInfoMapLock_);

    UpdateRecord(config, streamId);

    CHECK_AND_RETURN_LOG(
        (streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage][streamId].updateTime -
        streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage][streamId].startTime >= threshold),
        "the stream is legal, no need to check");

    CheckIsOtherStreamRunning(config, streamId);
}

void AudioStreamConcurrencyDetector::RemoveStream(const AudioProcessConfig &config, const uint32_t streamId)
{
    CHECK_AND_RETURN((config.audioMode != AUDIO_MODE_RECORD) && isEnabled_);

    std::lock_guard<std::mutex> lock(concurrencyInfoMapLock_);

    auto uidIt = streamConcurrInfoMap_.find(config.appInfo.appUid);
    CHECK_AND_RETURN_LOG(uidIt != streamConcurrInfoMap_.end(), "uid is not exist");

    auto streamUsageIt = uidIt->second.find(config.rendererInfo.streamUsage);
    CHECK_AND_RETURN_LOG(streamUsageIt != uidIt->second.end(), "streamUsage is not exist");

    auto streamIdIt = streamUsageIt->second.find(streamId);
    CHECK_AND_RETURN_LOG(streamIdIt != streamUsageIt->second.end(), "streamId is not exist");

    streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage].erase(streamIdIt);

    CHECK_AND_RETURN(streamConcurrInfoMap_[config.appInfo.appUid][config.rendererInfo.streamUsage].empty());

    streamConcurrInfoMap_[config.appInfo.appUid].erase(streamUsageIt);

    CHECK_AND_RETURN(streamConcurrInfoMap_[config.appInfo.appUid].empty());

    streamConcurrInfoMap_.erase(uidIt);
}

AudioStreamConcurrencyDetector& AudioStreamConcurrencyDetector::GetInstance()
{
    static AudioStreamConcurrencyDetector instance;
    return instance;
}

AudioStreamConcurrencyDetector::AudioStreamConcurrencyDetector()
{
    // this flag is used to judge that is current platform allow concurrent play
    isEnabled_ = false;
}
}
}
