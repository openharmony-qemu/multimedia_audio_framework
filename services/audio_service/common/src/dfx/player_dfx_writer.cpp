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
#define LOG_TAG "PlayerDfxWriter"

#include <map>

#include "player_dfx_writer.h"

namespace OHOS {
namespace AudioStandard {

PlayerDfxWriter::PlayerDfxWriter(const AppInfo &appInfo, uint32_t index)
{
    dfxCollector_ = std::make_unique<AudioRenderDfxCollector>();
    appInfo_ = appInfo;
    index_ = index;
}

PlayerDfxWriter::~PlayerDfxWriter()
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    dfxCollector_->FlushDfxMsg(index_, appInfo_.appUid);
}

void PlayerDfxWriter::WriteDfxStartMsg(uint32_t index, RendererStage stage,
    int64_t sourceDuration, const AudioProcessConfig &processConfig)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    if (stage != RENDERER_STAGE_START_OK &&
        stage != RENDERER_STAGE_START_FAIL) {
        return;
    }

    RenderDfxBuilder dfxBuilder;
    ++dfxCollector_->dfxIndex_;
    dfxBuilder.WriteInfoMsg(sourceDuration, processConfig.rendererInfo);

    dfxBuilder.WriteActionMsg(dfxCollector_->dfxIndex_, stage);
    dfxCollector_->AddDfxMsg(index, dfxBuilder.GetResult());
}

void PlayerDfxWriter::WriteDfxStopMsg(uint32_t index, RendererStage stage,
    const PlayStat &stat, const AudioProcessConfig &processConfig)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    if (stage != RENDERER_STAGE_STOP_OK &&
        stage != RENDERER_STAGE_STOP_BY_RELEASE) {
        return;
    }

    RenderDfxBuilder dfxBuilder;
    dfxBuilder.WriteStatMsg(
        processConfig, {stat.frameCnt, stat.muteFrameCnt,
        stat.playDuration, stat.underFlowCnt}).GetResult();

    dfxBuilder.WriteActionMsg(dfxCollector_->dfxIndex_, stage);
    dfxCollector_->AddDfxMsg(index, dfxBuilder.GetResult());
}

void PlayerDfxWriter::WriteDfxActionMsg(uint32_t index, RendererStage stage)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    RenderDfxBuilder dfxBuilder;
    dfxBuilder.WriteActionMsg(dfxCollector_->dfxIndex_, stage);
    dfxCollector_->AddDfxMsg(index, dfxBuilder.GetResult());
}

} // namespace AudioStandard
} // namespace OHOS
