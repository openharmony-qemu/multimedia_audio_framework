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
#define LOG_TAG "AudioRenderDfxCollector"

#include <map>
#include <cinttypes>

#include "audio_renderer_dfx_collector.h"
#include "media_monitor_manager.h"
#include "audio_common_log.h"
#include "dfx_msg_manager.h"

namespace OHOS {
namespace AudioStandard {

static const std::map<PlayerType, DfxPlayerType> DFX_PLAYER_TYPE_MAP = {
    {PLAYER_TYPE_DEFAULT, DFX_PLAYER_TYPE_NATIVE_RENDER},
    {PLAYER_TYPE_OH_AUDIO_RENDERER, DFX_PLAYER_TYPE_NATIVE_RENDER},
    {PLAYER_TYPE_ARKTS_AUDIO_RENDERER, DFX_PLAYER_TYPE_TS_RENDER},
    {PLAYER_TYPE_OPENSL_ES, DFX_PLAYER_TYPE_OPENSL_ES},
    {PLAYER_TYPE_SOUND_POOL, DFX_PLAYER_TYPE_SOUNDPOOL},
    {PLAYER_TYPE_AV_PLAYER, DFX_PLAYER_TYPE_AVPLAYER},
    {PLAYER_TYPE_TONE_PLAYER, DFX_PLAYER_TYPE_TONEPLAYER},
};

void AudioRenderDfxCollector::FlushDfxMsg(uint32_t index, int32_t appUid)
{
    if (appUid == DFX_INVALID_APP_UID) {
        AUDIO_INFO_LOG("flush failed index=%{public}d, appUid=%{public}d", index, appUid);
        return;
    }

    for (auto &item : dfxInfos_) {
        AUDIO_INFO_LOG("FlushDfxMsg..., index=%{public}u, appUid=%{public}d", item.first, appUid);
        DfxMsgManager::GetInstance().Enqueue({.appUid = appUid, .renderInfo = item.second});
    }

    dfxInfos_.clear();
}

RenderDfxBuilder& RenderDfxBuilder::WriteActionMsg(uint32_t dfxIndex, RendererStage stage)
{
    dfxInfo_.rendererAction = {dfxIndex, 0, 0, stage};
    return *this;
}

RenderDfxBuilder& RenderDfxBuilder::WriteInfoMsg(int64_t sourceDuration, const AudioRendererInfo &rendererInfo)
{
    std::chrono::milliseconds durationMs(sourceDuration);
    auto durationSec = std::chrono::duration_cast<std::chrono::duration<int64_t, std::deci>>(durationMs).count();
    auto dfxDurationSec = static_cast<uint16_t>(std::clamp(
        durationSec, static_cast<int64_t>(MIN_DFX_NUMERIC_COUNT),
            static_cast<int64_t>(std::numeric_limits<uint16_t>::max())));
    AUDIO_INFO_LOG("[Start] duration=%{public}" PRId16, dfxDurationSec);

    auto pos = DFX_PLAYER_TYPE_MAP.find(rendererInfo.playerType);
    DfxPlayerType playerType = (pos == DFX_PLAYER_TYPE_MAP.end()) ? DFX_PLAYER_TYPE_NATIVE_RENDER : pos->second;

    dfxInfo_.rendererInfo = {(dfxDurationSec >> 8) & 0xFF, dfxDurationSec & 0xFF,
        playerType, rendererInfo.streamUsage};
    return *this;
}

RenderDfxBuilder& RenderDfxBuilder::WriteStatMsg(const AudioRendererInfo &rendererInfo, const PlayStat &playStat)
{
    auto writeFrame = playStat.frameCnt;
    auto muteWriteFrame = playStat.muteFrameCnt;
    auto lastPlayduration = playStat.playDuration;

    uint16_t dfxZerodataPercent{0};
    if (muteWriteFrame != 0) {
        auto zerodataPercent = static_cast<int32_t>(
            static_cast<double>(writeFrame) / (muteWriteFrame * MAX_DFX_NUMERIC_PERCENTAGE));
        dfxZerodataPercent = std::clamp(zerodataPercent, MIN_DFX_NUMERIC_COUNT, MAX_DFX_NUMERIC_PERCENTAGE);
    }
    AUDIO_INFO_LOG("[WritePlayingAudioStatMsg] writeFrame=%{public}" PRId64 \
        "muteWriteFrame=%{public}" PRId64 "zerodataPercent=%{public}" PRId32 \
        "lastPlayduration=%{public}" PRId64,
        writeFrame, muteWriteFrame, dfxZerodataPercent, lastPlayduration);

    dfxInfo_.rendererStat = {rendererInfo.samplingRate, lastPlayduration, playStat.underFlowCnt,
        rendererInfo.originalFlag, dfxZerodataPercent};
    return *this;
}

RenderDfxInfo RenderDfxBuilder::GetResult()
{
    return dfxInfo_;
}
} // namespace AudioStandard
} // namespace OHOS
