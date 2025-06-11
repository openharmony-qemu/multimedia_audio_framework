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

#ifndef HPAE_DEFINE_H
#define HPAE_DEFINE_H
#include "hpae_msg_channel.h"
#include "i_stream.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t MILLISECOND_PER_SECOND = 1000;

struct HpaeSessionInfo {
    HpaeStreamInfo streamInfo;
    HpaeSessionState state = HPAE_SESSION_NEW;
    std::weak_ptr<IStatusCallback> statusCallback;
    int32_t offloadType = OFFLOAD_DEFAULT;
    bool offloadEnable = false;
};


constexpr int32_t SCENE_TYPE_NUM = 9;

struct HpaeRenderSessionInfo {
    uint32_t sinkInputNodeId = 0;
    HpaeProcessorType sceneType = HPAE_SCENE_DEFAULT;
    HpaeSessionState state = HPAE_SESSION_NEW;
    bool isMoveAble = true;
    bool bypass = false;
};

struct HpaeSinkInputInfo {
    HpaeRenderSessionInfo rendererSessionInfo;
    HpaeNodeInfo nodeInfo;
};

struct HpaeSinkInfo {
    uint32_t sinkId;
    std::string deviceNetId;
    std::string deviceClass;
    std::string adapterName;
    std::string lib;
    std::string filePath;
    std::string deviceName;
    size_t frameLen;
    AudioSamplingRate samplingRate;
    AudioSampleFormat format;
    AudioChannel channels;
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
};

struct HpaeCapturerSessionInfo {
    HpaeProcessorType sceneType = HPAE_SCENE_DEFAULT;
    HpaeSessionState state = HPAE_SESSION_NEW;
    bool isMoveAble = true;
};

struct HpaeSourceOutputInfo {
    HpaeCapturerSessionInfo capturerSessionInfo;
    HpaeNodeInfo nodeInfo;
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
    size_t frameLen;
    AudioSamplingRate samplingRate;
    AudioSampleFormat format;
    AudioChannel channels;
    uint64_t channelLayout = 0ULL;
    int32_t deviceType = 0;
    float volume = 0.0f;
    HpaeEcType ecType;
    size_t ecFrameLen;
    std::string ecAdapterName;
    AudioSamplingRate ecSamplingRate;
    AudioSampleFormat ecFormat;
    AudioChannel ecChannels;
    HpaeMicRefSwitch micRef;
    size_t micRefFrameLen;
    AudioSamplingRate micRefSamplingRate;
    AudioSampleFormat micRefFormat;
    AudioChannel micRefChannels;
    uint32_t openMicSpeaker;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif // HPAE_DEFINE_H