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

#ifndef LOG_TAG
#define LOG_TAG "OHAudioCommon"
#endif

#include "OHAudioCommon.h"

namespace OHOS {
namespace AudioStandard {
AudioChannel OHAudioCommon::ConvertLayoutToChannel(OH_AudioChannelLayout layout)
{
    AudioChannel channel = AudioChannel::CHANNEL_UNKNOW;
    switch (layout) {
        case OH_AudioChannelLayout::CH_LAYOUT_MONO:
            channel = AudioChannel::MONO;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_STEREO:
            channel = AudioChannel::STEREO;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_2POINT1:
        case OH_AudioChannelLayout::CH_LAYOUT_3POINT0:
            channel = AudioChannel::CHANNEL_3;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_3POINT1:
        case OH_AudioChannelLayout::CH_LAYOUT_4POINT0:
        case OH_AudioChannelLayout::CH_LAYOUT_QUAD:
            channel = AudioChannel::CHANNEL_4;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_5POINT0:
        case OH_AudioChannelLayout::CH_LAYOUT_2POINT1POINT2:
            channel = AudioChannel::CHANNEL_5;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_5POINT1:
        case OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL:
        case OH_AudioChannelLayout::CH_LAYOUT_3POINT1POINT2:
            channel = AudioChannel::CHANNEL_6;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT0:
            channel = AudioChannel::CHANNEL_7;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT1:
            channel = AudioChannel::CHANNEL_8;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT1POINT2:
            channel = AudioChannel::CHANNEL_10;
            break;
        case OH_AudioChannelLayout::CH_LAYOUT_7POINT1POINT4:
            channel = AudioChannel::CHANNEL_12;
            break;
        default:
            channel = AudioChannel::CHANNEL_UNKNOW;
            break;
    }
    return channel;
}

} // namespace AudioStandard
} // namespace OHOS
