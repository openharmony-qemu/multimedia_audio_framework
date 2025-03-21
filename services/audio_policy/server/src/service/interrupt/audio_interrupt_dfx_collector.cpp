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
#define LOG_TAG "AudioInterruptDfxCollector"

#include <map>

#include "audio_interrupt_dfx_collector.h"
#include "media_monitor_manager.h"
#include "running_process_info.h"
#include "audio_system_manager.h"
#include "dfx_msg_manager.h"

namespace OHOS {
namespace AudioStandard {

void AudioInterruptDfxCollector::FlushDfxMsg(uint32_t index, int32_t appUid)
{
    if (!IsExist(index) || appUid == DFX_INVALID_APP_UID) {
        AUDIO_INFO_LOG("flush failed index=%{public}d, appUid=%{public}d", index, appUid);
        return;
    }
    AUDIO_INFO_LOG("FlushDfxMsg..., index=%{public}u, appUid=%{public}d", index, appUid);
    auto &item = dfxInfos_[index];
    DfxMsgManager::GetInstance().Enqueue({.appUid = appUid, .interruptInfo = item});

    dfxInfos_.erase(index);
    if (dfxIdx2InfoIdx_.count(index) != 0) {
        dfxIdx2InfoIdx_.erase(index);
    }
}

std::tuple<uint8_t, uint8_t> &AudioInterruptDfxCollector::GetDfxIndexes(uint32_t index)
{
    auto iter = dfxIdx2InfoIdx_.find(index);
    if (iter == dfxIdx2InfoIdx_.end()) {
        dfxIdx2InfoIdx_.insert({index, {0, 0}});
    }
    return dfxIdx2InfoIdx_[index];
}


InterruptDfxBuilder &InterruptDfxBuilder::WriteActionMsg(uint8_t infoIndex, uint8_t effectIdx, InterruptStage stage)
{
    AUDIO_INFO_LOG("[WriteInfoMsg] infoIdx=%{public}d, effectIdx=%{public}d", infoIndex, effectIdx);
    dfxInfo_.interruptAction = {infoIndex, effectIdx, 0, stage};
    return *this;
}

InterruptDfxBuilder &InterruptDfxBuilder::WriteInfoMsg(const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("[WriteInfoMsg] streamUsage=%{public}d, concurrencyMode=%{public}d",
        audioInterrupt.streamUsage, audioInterrupt.sessionStrategy.concurrencyMode);
    uint8_t value3 = static_cast<uint8_t>(audioInterrupt.streamUsage);

    uint8_t interruptType = 0;
    uint8_t value4 = (static_cast<uint8_t>(audioInterrupt.sessionStrategy.concurrencyMode) & 0x0F) << 4 |
        (interruptType & 0x0F);

    dfxInfo_.interruptInfo = {0, 0, value3, value4};
    return *this;
}

InterruptDfxBuilder &InterruptDfxBuilder::WriteEffectMsg(uint8_t appstate, const std::string &bundleName,
    const AudioInterrupt &audioInterrupt, const InterruptHint &hintType)
{
    InterruptEffect interruptEffect{bundleName, audioInterrupt.streamUsage, appstate, hintType};
    dfxInfo_.interruptEffectVec.push_back(interruptEffect);
    return *this;
}

InterruptDfxInfo InterruptDfxBuilder::GetResult()
{
    return dfxInfo_;
}

} // namespace AudioStandard
} // namespace OHOS
