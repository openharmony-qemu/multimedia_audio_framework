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

#include "recorder_dfx_writer.h"

namespace OHOS {
namespace AudioStandard {

RecorderDfxWriter::RecorderDfxWriter(const AppInfo &appInfo, uint32_t index)
{
    dfxCollector_ = std::make_unique<AudioCapturerDfxCollector>();
    appInfo_ = appInfo;
    index_ = index;
}

RecorderDfxWriter::~RecorderDfxWriter()
{
    AUDIO_INFO_LOG("enter deconstruct");
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    dfxCollector_->FlushDfxMsg(index_, appInfo_.appUid);
}

void RecorderDfxWriter::WriteDfxStartMsg(uint32_t index, CapturerStage stage, const AudioProcessConfig &processConfig)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    if (stage != CAPTURER_STAGE_START_OK &&
        stage != CAPTURER_STAGE_START_FAIL) {
        return;
    }

    CapturerDfxBuilder dfxBuilder;
    auto dfxInfo = dfxBuilder.WriteActionMsg(++dfxCollector_->dfxIndex_, stage).WriteInfoMsg(
        processConfig.capturerInfo).GetResult();
    dfxCollector_->AddDfxMsg(index, dfxInfo);
}

void RecorderDfxWriter::WriteDfxStopMsg(uint32_t index, CapturerStage stage,
    int64_t duration, const AudioProcessConfig &processConfig)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    if (stage != CAPTURER_STAGE_STOP_OK &&
        stage != CAPTURER_STAGE_PAUSE_OK &&
        stage != CAPTURER_STAGE_STOP_BY_RELEASE) {
        return;
    }

    CapturerDfxBuilder dfxBuilder;
    dfxBuilder.WriteActionMsg(dfxCollector_->dfxIndex_, stage);
    dfxBuilder.WriteStatMsg(processConfig, {duration});
    dfxCollector_->AddDfxMsg(index, dfxBuilder.GetResult());
}

void RecorderDfxWriter::WriteDfxActionMsg(uint32_t index, CapturerStage stage)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "nullptr");
    CapturerDfxBuilder dfxBuilder;
    auto dfxInfo = dfxBuilder.WriteActionMsg(dfxCollector_->dfxIndex_, stage).GetResult();
    dfxCollector_->AddDfxMsg(index, dfxInfo);
}

} // namespace AudioStandard
} // namespace OHOS
