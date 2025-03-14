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
#define LOG_TAG "AudioCapturerDfxCollector"

#include <map>

#include "audio_capturer_dfx_collector.h"
#include "media_monitor_manager.h"
#include "audio_common_log.h"
#include "dfx_msg_manager.h"

namespace OHOS {
namespace AudioStandard {

void AudioCapturerDfxCollector::FlushDfxMsg(uint32_t index, int32_t appUid)
{
    if (appUid == DFX_INVALID_APP_UID) {
        AUDIO_INFO_LOG("flush failed index=%{public}d, appUid=%{public}d", index, appUid);
        return;
    }

    for (auto &item : dfxInfos_) {
        AUDIO_INFO_LOG("FlushDfxMsg..., index=%{public}u, appUid=%{public}d", item.first, appUid);
        DfxMsgManager::GetInstance().Enqueue({.appUid = appUid, .captureInfo = item.second});
    }
    dfxInfos_.clear();
}

CapturerDfxBuilder &CapturerDfxBuilder::WriteActionMsg(uint32_t dfxIndex, CapturerStage stage)
{
    dfxInfo_.capturerAction = {dfxIndex, 0, 0, stage};
    return *this;
}

CapturerDfxBuilder &CapturerDfxBuilder::WriteInfoMsg(SourceType sourceType)
{
    dfxInfo_.capturerInfo = {0, 0, sourceType, 0};
    return *this;
}

CapturerDfxBuilder &CapturerDfxBuilder::WriteStatMsg(const AudioCapturerInfo &info, const RecordStat &stat)
{
    dfxInfo_.capturerStat = {info.samplingRate, stat.recordDuration_};
    return *this;
}

CapturerDfxInfo CapturerDfxBuilder::GetResult()
{
    return dfxInfo_;
}

} // namespace AudioStandard
} // namespace OHOS
