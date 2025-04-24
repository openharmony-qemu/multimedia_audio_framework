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

#include "hpae_node_common.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint64_t TIME_US_PER_S = 1000000;
static std::map<AudioStreamType, HpaeProcessorType> g_streamTypeToSceneTypeMap = {
    {STREAM_MUSIC, HPAE_SCENE_MUSIC},
    {STREAM_GAME, HPAE_SCENE_GAME},
    {STREAM_MOVIE, HPAE_SCENE_MOVIE},
    {STREAM_GAME, HPAE_SCENE_GAME},
    {STREAM_SPEECH, HPAE_SCENE_SPEECH},
    {STREAM_VOICE_RING, HPAE_SCENE_RING},
    {STREAM_VOICE_COMMUNICATION, HPAE_SCENE_VOIP_DOWN},
    {STREAM_MEDIA, HPAE_SCENE_OTHERS}
};

static std::unordered_map<SourceType, HpaeProcessorType> g_sourceTypeToSceneTypeMap = {
    {SOURCE_TYPE_MIC, HPAE_SCENE_RECORD},
    {SOURCE_TYPE_CAMCORDER, HPAE_SCENE_RECORD},
    {SOURCE_TYPE_VOICE_CALL, HPAE_SCENE_VOIP_UP},
    {SOURCE_TYPE_VOICE_COMMUNICATION, HPAE_SCENE_VOIP_UP},
    {SOURCE_TYPE_VOICE_TRANSCRIPTION, HPAE_SCENE_PRE_ENHANCE},
    {SOURCE_TYPE_VOICE_MESSAGE, HPAE_SCENE_VOICE_MESSAGE}
};


static std::unordered_set<HpaeProcessorType> g_processorTypeNeedEcSet = {
    HPAE_SCENE_VOIP_UP,
    HPAE_SCENE_PRE_ENHANCE,
};

static std::unordered_set<HpaeProcessorType> g_processorTypeNeedMicRefSet = {
    HPAE_SCENE_VOIP_UP,
    HPAE_SCENE_RECORD,
};

static std::unordered_map<HpaeProcessorType, AudioEnhanceScene> g_processorTypeToSceneTypeMap = {
    {HPAE_SCENE_RECORD, SCENE_RECORD},
    {HPAE_SCENE_VOIP_UP, SCENE_VOIP_UP},
    {HPAE_SCENE_PRE_ENHANCE, SCENE_PRE_ENHANCE},
    {HPAE_SCENE_VOICE_MESSAGE, SCENE_VOICE_MESSAGE}
};

static std::unordered_map<HpaseSessionState, std::string> g_sessionStateToStrMap = {
    {HPAE_SESSION_NEW, "NEW"},
    {HPAE_SESSION_PREPARED, "PREPARED"},
    {HPAE_SESSION_RUNNING, "RUNNING"},
    {HPAE_SESSION_PAUSING, "PAUSING"},
    {HPAE_SESSION_PAUSED, "PAUSED"},
    {HPAE_SESSION_STOPPING, "STOPPING"},
    {HPAE_SESSION_STOPPED, "STOPPED"},
    {HPAE_SESSION_RELEASED, "RELEASED"}
};

static std::unordered_map<StreamManagerState, std::string> g_streamMgrStateToStrMap = {
    {STREAM_MANAGER_NEW, "NEW"},
    {STREAM_MANAGER_IDLE, "IDLE"},
    {STREAM_MANAGER_RUNNING, "RUNNING"},
    {STREAM_MANAGER_SUSPENDED, "SUSPENDED"},
    {STREAM_MANAGER_RELEASED, "RELEASED"}

};

std::string ConvertSessionState2Str(HpaeSessionState state)
{
    if (g_sessionStateToStrMap.find(state) == g_sessionStateToStrMap.end()) {
        return "UNKNOWN";
    }
    return g_sessionStateToStrMap[state];
}

std::string ConvertStreamManagerState2Str(StreamManagerState state)
{
    if (g_streamMgrStateToStrMap.find(state) == g_streamMgrStateToStrMap.end()) {
        return "UNKNOWN";
    }
    return g_streamMgrStateToStrMap[state];
}

HpaeProcessorType TransStreamTypeToSceneType(AudioStreamType streamType)
{
    if (g_streamTypeToSceneTypeMap.find(streamType) == g_streamTypeToSceneTypeMap.end()) {
        return HPAE_SCENE_EFFECT_NONE;
    } else {
        return g_streamTypeToSceneTypeMap[streamType];
    }
}

HpaeProcessorType TransSourceTypeToSceneType(SourceType sourceType)
{
    if (g_sourceTypeToSceneTypeMap.find(sourceType) == g_sourceTypeToSceneTypeMap.end()) {
        return HPAE_SCENE_EFFECT_NONE;
    } else {
        return g_sourceTypeToSceneTypeMap[sourceType];
    }
}

bool CheckSceneTypeNeedEc(HpaeProcessorType processorType)
{
    return g_processorTypeNeedEcSet.find(processorType) != g_processorTypeNeedEcSet.end();
}

bool CheckSceneTypeNeedMicRef(HpaeProcessorType processorType)
{
    return g_processorTypeNeedMicRefSet.find(processorType) != g_processorTypeNeedMicRefSet.end();
}

static std::unordered_map<HpaeProcessorType, std::string> g_processorTypeToEffectSceneTypeMap = {
    {HPAE_SCENE_DEFAULT, "HPAE_SCENE_DEFAULT"},
    {HPAE_SCENE_OTHERS, "SCENE_OTHERS"},
    {HPAE_SCENE_MUSIC, "SCENE_MUSIC"},
    {HPAE_SCENE_GAME, "SCENE_GAME"},
    {HPAE_SCENE_MOVIE, "SCENE_MOVIE"},
    {HPAE_SCENE_SPEECH, "SCENE_SPEECH"},
    {HPAE_SCENE_RING, "SCENE_RING"},
    {HPAE_SCENE_VOIP_DOWN, "SCENE_VOIP_DOWN"}};

std::string TransProcessorTypeToSceneType(HpaeProcessorType processorType)
{
    if (g_processorTypeToEffectSceneTypeMap.find(processorType) == g_processorTypeToEffectSceneTypeMap.end()) {
        return "SCENE_EXTRA";
    } else {
        return g_processorTypeToEffectSceneTypeMap[processorType];
    }
}

bool CheckHpaeNodeInfoIsSame(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &curNodeInfo)
{
    return preNodeInfo.channels == curNodeInfo.channels &&  //&& preNodeInfo.format == curNodeInfo.format todo
           preNodeInfo.samplingRate == curNodeInfo.samplingRate &&
           preNodeInfo.channelLayout == curNodeInfo.channelLayout;
}

std::string TransNodeInfoToStringKey(HpaeNodeInfo& nodeInfo)
{
    std::string nodeKey = std::to_string(nodeInfo.sourceBufferType) + "_" +
                          std::to_string(nodeInfo.samplingRate) + "_" +
                          std::to_string(nodeInfo.channels) + "_" +
                          std::to_string(nodeInfo.format);
    return nodeKey;
}

AudioEnhanceScene TransProcessType2EnhanceScene(const HpaeProcessorType &processorType)
{
    if (g_processorTypeToSceneTypeMap.find(processorType) == g_processorTypeToSceneTypeMap.end()) {
        return SCENE_NONE;
    } else {
        return g_processorTypeToSceneTypeMap[processorType];
    }
}

size_t ConvertUsToFrameCount(uint64_t usTime, const HpaeNodeInfo &nodeInfo)
{
    return usTime * nodeInfo.samplingRate / TIME_US_PER_S /
        (nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
}

uint64_t ConvertDatalenToUs(size_t bufferSize, const HpaeNodeInfo &nodeInfo)
{
    if (nodeInfo.channels == 0 || GetSizeFromFormat(nodeInfo.format) == 0 || nodeInfo.samplingRate == 0) {
        AUDIO_ERR_LOG("invalid nodeInfo");
        return 0;
    }

    double samples = static_cast<double>(bufferSize) /
                     (nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    double seconds = samples / static_cast<int32_t>(nodeInfo.samplingRate);
    double microseconds = seconds * TIME_US_PER_S;

    return static_cast<uint64_t>(microseconds);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS