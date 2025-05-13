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

#ifndef ST_AUDIO_INTERRUPT_DFX_COLLECTOR_H
#define ST_AUDIO_INTERRUPT_DFX_COLLECTOR_H

#include <map>
#include <tuple>

#include "dfx_collector.h"
#include "audio_info.h"
#include "audio_interrupt_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioInterruptDfxCollector : public DfxCollector<InterruptDfxInfo> {
public:
    void FlushDfxMsg(uint32_t index, int32_t appUid) override;
    std::tuple<uint8_t, uint8_t> &GetDfxIndexes(uint32_t index);
private:
    std::map<uint32_t, std::tuple<uint8_t, uint8_t>> dfxIdx2InfoIdx_;
};

class InterruptDfxBuilder {
public:
    InterruptDfxBuilder &WriteActionMsg(uint8_t infoIndex, uint8_t effectIdx, InterruptStage stage);
    InterruptDfxBuilder &WriteInfoMsg(const AudioInterrupt &audioInterrupt, const AudioSessionStrategy &strategy,
        InterruptRole interruptType);
    InterruptDfxBuilder &WriteEffectMsg(uint8_t appstate, const std::string &bundleName,
        const AudioInterrupt &audioInterrupt, const InterruptHint &hintType);
    InterruptDfxInfo GetResult();
private:
    InterruptDfxInfo dfxInfo_{};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_INTERRUPT_DFX_COLLECTOR_H