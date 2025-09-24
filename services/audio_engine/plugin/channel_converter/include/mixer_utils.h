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
#ifndef MIXER_UTILS_H
#define MIXER_UTILS_H
#include <vector>
#include "audio_stream_info.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t MAX_CHANNELS = 16;
// max framelength is sample rate 192000, 10s
constexpr uint32_t MAX_CHANNEL_CONVERT_FRAME_LENGTH = SAMPLE_RATE_192000 * 10;
constexpr float COEF_ZERO_F = 0.0f;
constexpr float COEF_0DB_F = 1.0f;
constexpr float COEF_M3DB_F = 0.7071f;
constexpr float COEF_M6DB_F = 0.5f;
constexpr float COEF_M435DB_F = 0.6057f;
constexpr float COEF_M45DB_F = 0.5946f;
constexpr float COEF_M9DB_F = 0.3544f;
constexpr float COEF_M899DB_F = 0.3552f;
constexpr float COEF_M12DB_F = 0.2509f;

enum {
    MIX_ERR_SUCCESS = 0,
    MIX_ERR_ALLOC_FAILED = -1,
    MIX_ERR_INVALID_ARG = -2
};

// used for setting up general mixing table for downmixing and upmixing
int32_t SetUpGeneralMixingTable(float (&coeffTable)[MAX_CHANNELS][MAX_CHANNELS], AudioChannelInfo inChannelInfo,
    AudioChannelInfo outChannelInfo, bool mixLfe);

bool SetDefaultChannelLayout(AudioChannel channels, AudioChannelLayout &channelLayout);

bool IsValidChLayout(AudioChannelLayout &chLayout, uint32_t chCounts);

uint32_t BitCounts(uint64_t bits);

bool CheckIsHOA(AudioChannelLayout layout);
} // HPAE
} // AudioStandard
} // OHOS
#endif