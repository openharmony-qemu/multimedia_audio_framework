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
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    AudioVolumeType systemVolumeType = STREAM_MUSIC;
    AudioEffectScene effectScene = SCENE_OTHERS;
    AudioEffectMode effectMode = EFFECT_NONE;
    AudioEnhanceScene enhanceScene = SCENE_NONE;
    AudioEnhanceMode enhanceMode = ENHANCE_NONE;
    AudioEffectScene lastEffectScene = SCENE_OTHERS;
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

enum MoveSessionType {
    MOVE_SINGLE,
    MOVE_ALL,
    MOVE_PREFER,
};

struct HpaeStreamInfo {
    uint32_t sessionId = 0;
    size_t frameLen = 0;
    HpaeNodeType nodeType = HPAE_NODE_TYPE_INVALID;
    AudioStreamType streamType = STREAM_DEFAULT;
    FadeType fadeType = NONE_FADE;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    uint32_t customSampleRate = 0;
    AudioSampleFormat format = INVALID_WIDTH;
    AudioChannel channels = CHANNEL_UNKNOW;
    uint64_t channelLayout = 0ULL;
    HpaeStreamClassType streamClassType = HPAE_STREAM_CLASS_TYPE_INVALID;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t uid = -1;
    int32_t pid = 0;
    uint32_t tokenId = 0;
    HpaeEffectInfo effectInfo;
    std::string deviceName;
    bool isMoveAble = true;
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
};

struct HpaeSinkInfo {
    uint32_t sinkId;
    std::string deviceNetId;
    std::string deviceClass;
    std::string adapterName;
    std::string lib;
    std::string filePath;
    std::string deviceName;
    size_t frameLen = 0;
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    AudioSampleFormat format = INVALID_WIDTH;
    AudioChannel channels = CHANNEL_UNKNOW;
    uint32_t suspendTime = 0; // in ms
    uint64_t channelLayout = 0ULL;
    int32_t deviceType = 0;
    float volume = 0.0f;
    uint32_t openMicSpeaker = 0;
    uint32_t renderInIdleState = 0;
    uint32_t sourceType = 0;
    uint32_t offloadEnable = 0;
    uint32_t fixedLatency = 0;
    uint32_t sinkLatency = 0;
    std::string splitMode;
    bool needEmptyChunk = true;
};

enum HpaeEcType {
    HPAE_EC_TYPE_NONE,
    HPAE_EC_TYPE_SAME_ADAPTER,
    HPAE_EC_TYPE_DIFF_ADAPTER
};

enum HpaeMicRefSwitch {
    HPAE_REF_OFF = 0,
    HPAE_REF_ON
};

struct HpaeSourceInfo {
    uint32_t sourceId;
    std::string deviceNetId;
    std::string deviceClass;
    std::string adapterName;
    std::string sourceName;
    SourceType sourceType;
    std::string filePath;
    std::string deviceName;
    size_t frameLen = 0;
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    AudioSampleFormat format = INVALID_WIDTH;
    AudioChannel channels = CHANNEL_UNKNOW;
    uint64_t channelLayout = 0ULL;
    int32_t deviceType = 0;
    float volume = 0.0f;
    HpaeEcType ecType = HPAE_EC_TYPE_NONE;
    size_t ecFrameLen = 0;
    std::string ecAdapterName;
    AudioSamplingRate ecSamplingRate = SAMPLE_RATE_8000;
    AudioSampleFormat ecFormat = INVALID_WIDTH;
    AudioChannel ecChannels = CHANNEL_UNKNOW;
    HpaeMicRefSwitch micRef = HPAE_REF_OFF;
    size_t micRefFrameLen = 0;
    AudioSamplingRate micRefSamplingRate = SAMPLE_RATE_8000;
    AudioSampleFormat micRefFormat = INVALID_WIDTH;
    AudioChannel micRefChannels = CHANNEL_UNKNOW;
    uint32_t openMicSpeaker = 0;
    std::string macAddress = "";
};

static inline int32_t GetSizeFromFormat(int32_t format)
{
    return format != SAMPLE_F32LE ? ((format) + 1) : (4); // float 4
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
#endif