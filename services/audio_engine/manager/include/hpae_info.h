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

#ifndef HPAE_INFO_H
#define HPAE_INFO_H
#include "audio_effect.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
enum HpaeStreamClassType {
    HPAE_STREAM_CLASS_TYPE_INVALID = -1,
    HPAE_STREAM_CLASS_TYPE_PLAY,
    HPAE_STREAM_CLASS_TYPE_RECORD,
};

enum HpaeNodeType {
    HPAE_NODE_TYPE_INVALID = -1,
    HPAE_NODE_TYPE_SOURCE_INPUT,
    HPAE_NODE_TYPE_SOURCE_OUTPUT,
    HPAE_NODE_TYPE_SINK_INPUT,
    HPAE_NODE_TYPE_SINK_OUTPUT,
    HPAE_NODE_TYPE_PLUGIN,
};

struct HpaeEffectInfo {
    StreamUsage streamUsage;
    AudioVolumeType volumeType;
    AudioEffectScene effectScene;
    AudioEffectMode effectMode;
    AudioEnhanceScene enhanceScene;
    AudioEnhanceMode enhanceMode;
};

enum FadeType {
    DEFAULT_FADE = 0, // default one frame fade
    SHORT_FADE, // short 5ms fade
    NONE_FADE // do not fade
};

enum HpaeSessionState {
    HPAE_SESSION_INVALID = -1,
    HPAE_SESSION_NEW,
    HPAE_SESSION_PREPARED,
    HPAE_SESSION_RUNNING,
    HPAE_SESSION_PAUSING,
    HPAE_SESSION_PAUSED,
    HPAE_SESSION_STOPPING,
    HPAE_SESSION_STOPPED,
    HPAE_SESSION_RELEASED
};

// use for sink or source state
enum StreamManagerState {
    STREAM_MANAGER_INVALID = -1,
    STREAM_MANAGER_NEW,
    STREAM_MANAGER_IDLE,
    STREAM_MANAGER_RUNNING,
    STREAM_MANAGER_SUSPENDED,
    STREAM_MANAGER_RELEASED
};

struct HpaeStreamInfo {
    uint32_t sessionId;
    size_t frameLen;
    HpaeNodeType nodeType;
    AudioStreamType streamType;
    FadeType fadeType = NONE_FADE;
    AudioPipeType pipeType;
    AudioSamplingRate samplingRate;
    AudioSampleFormat format;
    AudioChannel channels;
    uint64_t channelLayout = 0ULL;
    HpaeStreamClassType streamClassType;
    SourceType sourceType;
    int32_t uid = -1;
    int32_t pid = 0;
    HpaeEffectInfo effectInfo;
    std::string deviceName;
};

static inline int32_t GetSizeFromFormat(int32_t format)
{
    return format != SAMPLE_F32LE ? ((format) + 1) : (4); // float 4
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
#endif