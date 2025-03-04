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

#ifndef ST_DFX_STAT_H
#define ST_DFX_STAT_H

#include <string>
#include <vector>
#include <chrono>

namespace OHOS {
namespace AudioStandard {

enum DfxPlayerType {
    DFX_PLAYER_TYPE_TS_RENDER = 0,
    DFX_PLAYER_TYPE_NATIVE_RENDER,
    DFX_PLAYER_TYPE_OPENSL_ES,
    DFX_PLAYER_TYPE_AVPLAYER,
    DFX_PLAYER_TYPE_SOUNDPOOL,
    DFX_PLAYER_TYPE_TONEPLAYER,
};

enum DfxAppState {
    DFX_APP_STATE_UNKNOWN = -1,
    DFX_APP_STATE_START = 0,
    DFX_APP_STATE_FOREGROUND = 2,
    DFX_APP_STATE_BACKGROUND = 4,
    DFX_APP_STATE_END = 16,
};

struct DfxStatInt32 {
    DfxStatInt32() = default;
    DfxStatInt32(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4)
        : firstByte(param1), secondByte(param2), thirdByte(param3), fourthByte(param4) {}

    uint8_t firstByte{};
    uint8_t secondByte{};
    uint8_t thirdByte{};
    uint8_t fourthByte{};
};

struct DfxStatAction : public DfxStatInt32 {
    DfxStatAction() = default;
    DfxStatAction(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);

    time_t timestamp{};
};

struct InterruptEffect {
    std::string bundleName{};
    uint8_t streamUsage{};
    uint8_t appState{};
    uint8_t interruptEvent{};
};

struct RendererStats {
    uint16_t samplingRate{};
    uint64_t duration{};
    uint16_t underrunCnt{};
    uint16_t originalFlag{};
    uint16_t zeroDataPercent{};
};

struct CapturerStats {
    uint16_t samplingRate{};
    uint64_t duration{};
};

struct RenderDfxInfo {
    DfxStatAction rendererAction{};
    DfxStatInt32 rendererInfo{};
    RendererStats rendererStat{};
};

struct CapturerDfxInfo {
    DfxStatAction capturerAction{};
    DfxStatInt32 capturerInfo{};
    CapturerStats capturerStat{};
};

struct InterruptDfxInfo {
    DfxStatAction interruptAction;
    DfxStatInt32 interruptInfo;
    std::vector<InterruptEffect> interruptEffectVec{};
};

struct DfxRunningAppInfo {
    int32_t appUid{-1};
    std::string appName{};
    std::string versionName{};
    std::vector<uint8_t> appStateVec{};
    std::vector<uint64_t> appStateTimeStampVec{};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DFX_STAT_H