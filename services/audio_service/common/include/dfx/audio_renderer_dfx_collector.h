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

#ifndef ST_AUDIO_RENDERER_DFX_COLLECTOR_H
#define ST_AUDIO_RENDERER_DFX_COLLECTOR_H

#include <map>

#include "dfx_collector.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

struct PlayStat {
    int64_t frameCnt{};
    int64_t muteFrameCnt{};
    int64_t playDuration{};
    uint32_t underFlowCnt{};
};

class AudioRenderDfxCollector : public DfxCollector<RenderDfxInfo> {
public:
    void FlushDfxMsg(uint32_t index, int32_t appUid) override;
};

class RenderDfxBuilder {
public:
    RenderDfxBuilder& WriteActionMsg(uint32_t dfxIndex, RendererStage stage);
    RenderDfxBuilder& WriteInfoMsg(int64_t sourceDuration, const AudioRendererInfo &rendererInfo);
    RenderDfxBuilder& WriteStatMsg(const AudioRendererInfo &rendererInfo, const PlayStat &playStat);
    RenderDfxInfo GetResult();
private:
    RenderDfxInfo dfxInfo_{};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_RENDERER_DFX_COLLECTOR_H