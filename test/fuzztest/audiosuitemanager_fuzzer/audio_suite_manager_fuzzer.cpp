/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <algorithm>
#include "audio_log.h"
#include "../fuzz_utils.h"
#include "audio_suite_pipeline.h"
#include "audio_suite_download_manager.h"
#include "securec.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

static const uint32_t MAX_PIPELINE_NUM = 10;
static const uint32_t MAX_NODE_NUM = 7;
static const uint32_t MAX_CHANNEL_NUM = 16;
static const uint32_t MAX_FRAME_SIZE = 1000;
static const uint32_t MAX_FEATURE_NAME_SIZE = 256;
static bool g_downloadManagerInited = false;
AudioSuite::AudioSuiteDownloadManager *g_downloadManager = nullptr;

static std::vector<std::shared_ptr<AudioSuite::AudioSuitePipeline>> g_pipelines;
std::shared_ptr<AudioSuite::AudioSuitePipeline> g_pipeline = nullptr;
static std::vector<uint32_t> g_nodeIds;

class DownloadStatusCallbackTestImpl : public AudioSuite::DownloadStatusCallback {
public:
    ~DownloadStatusCallbackTestImpl() = default;
    void OnDownloadStatusChanged(const std::vector<AudioSuite::AudioSuiteDownloadStatusInfo>& status) override
    {
        return;
    }
};

std::shared_ptr<AudioSuite::DownloadStatusCallback> g_downloadCallback =
    std::make_shared<DownloadStatusCallbackTestImpl>();

typedef void (*TestPtr)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

class InputNodeRequestDataCallBackTestImpl : public AudioSuite::InputNodeRequestDataCallBack {
public:
    ~InputNodeRequestDataCallBackTestImpl() = default;
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) override
    {
        (void)audioData;
        (void)audioDataSize;
        (void)finished;
        return 0;
    }
};

static void EnsureFuzzSetup()
{
    if (g_pipeline != nullptr) {
        return;
    }

    int32_t ret = AudioSuite::AudioSuitePipeline::Create(AudioSuite::PIPELINE_EDIT_MODE, g_pipeline);
    if (ret != 0 || g_pipeline == nullptr) {
        return;
    }

    auto tryCreateNode = [](AudioSuite::AudioNodeType type) {
        AudioSuite::AudioNodeBuilder builder;
        builder.nodeType = type;
        uint32_t nodeId = 0;
        if (g_pipeline->CreateNode(builder, nodeId) == 0) {
            g_nodeIds.push_back(nodeId);
        }
    };

    tryCreateNode(AudioSuite::NODE_TYPE_INPUT);
    tryCreateNode(AudioSuite::NODE_TYPE_EQUALIZER);
    tryCreateNode(AudioSuite::NODE_TYPE_SPACE_RENDER);
    tryCreateNode(AudioSuite::NODE_TYPE_TEMPO_PITCH);
    tryCreateNode(AudioSuite::NODE_TYPE_PURE_VOICE_CHANGE);
    tryCreateNode(AudioSuite::NODE_TYPE_GENERAL_VOICE_CHANGE);
    tryCreateNode(AudioSuite::NODE_TYPE_OUTPUT);
}

static uint32_t GetRandomNodeId()
{
    if (g_nodeIds.empty()) {
        return GetData<uint32_t>() % MAX_NODE_NUM;
    }
    return g_nodeIds[GetData<uint32_t>() % g_nodeIds.size()];
}

static std::shared_ptr<AudioSuite::AudioSuitePipeline> GetOrCreatePipeline(uint32_t index)
{
    if (g_pipelines.empty()) {
        return nullptr;
    }
    return g_pipelines[index % g_pipelines.size()];
}

void AudioSuiteManagerCreatePipelineFuzzTest()
{
    AudioSuite::PipelineWorkMode workMode = GetData<AudioSuite::PipelineWorkMode>();
    std::shared_ptr<AudioSuite::AudioSuitePipeline> pipeline;
    int32_t ret = AudioSuite::AudioSuitePipeline::Create(workMode, pipeline);
    if (ret == 0 && pipeline && g_pipelines.size() < MAX_PIPELINE_NUM) {
        g_pipelines.push_back(pipeline);
    }

    pipeline = GetOrCreatePipeline(GetData<uint32_t>());
    if (pipeline == nullptr) {
        return;
    }

    AudioSuite::AudioNodeBuilder nodeBuilder;
    nodeBuilder.nodeType = GetData<AudioSuite::AudioNodeType>();
    nodeBuilder.nodeFormat.audioChannelInfo.channelLayout = GetData<AudioChannelLayout>();
    nodeBuilder.nodeFormat.audioChannelInfo.numChannels = GetData<uint32_t>() % MAX_CHANNEL_NUM;
    nodeBuilder.nodeFormat.format = GetData<AudioSampleFormat>();
    nodeBuilder.nodeFormat.rate = GetData<AudioSamplingRate>();
    nodeBuilder.nodeFormat.encodingType = GetData<AudioSuite::AudioStreamEncodingType>();
    uint32_t nodeId = 0;
    pipeline->CreateNode(nodeBuilder, nodeId);
    pipeline->DestroyNode(nodeId);
    nodeId = GetRandomNodeId();
    pipeline->DestroyNode(nodeId);

    if (g_pipelines.empty()) {
        return;
    }
    uint32_t index = GetData<uint32_t>() % g_pipelines.size();
    g_pipelines.erase(g_pipelines.begin() + index);
}

void AudioSuiteManagerStartPipelineFuzzTest()
{
    if (g_pipeline != nullptr) {
        g_pipeline->Start();
    }
}

void AudioSuiteManagerStopPipelineFuzzTest()
{
    if (g_pipeline != nullptr) {
        g_pipeline->Stop();
    }
}

void AudioSuiteManagerGetPipelineStateFuzzTest()
{
    if (g_pipeline != nullptr) {
        AudioSuite::AudioSuitePipelineState pipelineState;
        g_pipeline->GetPipelineState(pipelineState);
    }
}

void AudioSuiteManagerBypassEffectNodeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        bool nodeEnable = GetData<bool>();
        g_pipeline->BypassEffectNode(nodeId, nodeEnable);
    }
}

void AudioSuiteManagerGetNodeBypassStatusFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        bool nodeEnable;
        g_pipeline->GetNodeBypassStatus(nodeId, nodeEnable);
    }
}

void AudioSuiteManagerSetAudioFormatFuzzTest()
{
    if (g_pipeline == nullptr) {
        return;
    }
    uint32_t nodeId = GetRandomNodeId();
    AudioSuite::AudioFormat nodeFormat;
    nodeFormat.audioChannelInfo.channelLayout = GetData<AudioChannelLayout>();
    nodeFormat.audioChannelInfo.numChannels = GetData<uint32_t>() % MAX_CHANNEL_NUM;
    nodeFormat.format = GetData<AudioSampleFormat>();
    nodeFormat.rate = GetData<AudioSamplingRate>();
    nodeFormat.encodingType = GetData<AudioSuite::AudioStreamEncodingType>();
    g_pipeline->SetAudioFormat(nodeId, nodeFormat);
}

void AudioSuiteManagerSetRequestDataCallbackFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        std::shared_ptr<AudioSuite::InputNodeRequestDataCallBack> callback =
            std::make_shared<InputNodeRequestDataCallBackTestImpl>();
        g_pipeline->SetRequestDataCallback(nodeId, callback);
    }
}

void AudioSuiteManagerConnectNodesFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t srcNodeId = GetRandomNodeId();
        uint32_t dstNodeId = GetRandomNodeId();
        g_pipeline->ConnectNodes(srcNodeId, dstNodeId);
    }
}

void AudioSuiteManagerDisconnectNodesFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t srcNodeId = GetRandomNodeId();
        uint32_t dstNodeId = GetRandomNodeId();
        g_pipeline->DisconnectNodes(srcNodeId, dstNodeId);
    }
}

void AudioSuiteManagerSetEqFrequencyBandGainsModeFuzzTest()
{
    if (g_pipeline == nullptr) {
        return;
    }
    uint32_t nodeId = GetRandomNodeId();
    AudioSuite::AudioEqualizerFrequencyBandGains gains;
    for (int i = 0; i < EQUALIZER_BAND_NUM; i++) {
        gains.gains[i] = GetData<int32_t>();
    }
    g_pipeline->SetEqualizerFrequencyBandGains(nodeId, gains);
}

void AudioSuiteManagerSetSoundFieldTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::SoundFieldType soundFieldType = GetData<AudioSuite::SoundFieldType>();
        g_pipeline->SetSoundFieldType(nodeId, soundFieldType);
    }
}

void AudioSuiteManagerSetEnvironmentTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::EnvironmentType environmentType = GetData<AudioSuite::EnvironmentType>();
        g_pipeline->SetEnvironmentType(nodeId, environmentType);
    }
}

void AudioSuiteManagerSetVoiceBeautifierTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::VoiceBeautifierType voiceBeautifierType = GetData<AudioSuite::VoiceBeautifierType>();
        g_pipeline->SetVoiceBeautifierType(nodeId, voiceBeautifierType);
    }
}

void AudioSuiteEngineSetTempoAndPitchFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        float speed = GetData<float>();
        float pitch = GetData<float>();
        g_pipeline->SetTempoAndPitch(nodeId, speed, pitch);
    }
}

void AudioSuiteEngineSetPureVoiceChangeOptionFuzzTest()
{
    if (g_pipeline == nullptr) {
        return;
    }
    uint32_t nodeId = GetRandomNodeId();
    AudioSuite::AudioPureVoiceChangeOption option;
    option.optionGender = GetData<AudioSuite::AudioPureVoiceChangeGenderOption>();
    option.optionType = GetData<AudioSuite::AudioPureVoiceChangeType>();
    option.pitch = GetData<float>();
    g_pipeline->SetPureVoiceChangeOption(nodeId, option);
}

void AudioSuiteEngineSetGeneralVoiceChangeTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioGeneralVoiceChangeType changeType = GetData<AudioSuite::AudioGeneralVoiceChangeType>();
        g_pipeline->SetGeneralVoiceChangeType(nodeId, changeType);
    }
}

void AudioSuiteManagerGetEqFrequencyBandGainsModeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioEqualizerFrequencyBandGains gains;
        g_pipeline->GetEqualizerFrequencyBandGains(nodeId, gains);
    }
}

void AudioSuiteManagerGetSoundFieldTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::SoundFieldType soundFieldType;
        g_pipeline->GetSoundFieldType(nodeId, soundFieldType);
    }
}

void AudioSuiteManagerGetEnvironmentTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::EnvironmentType environmentType;
        g_pipeline->GetEnvironmentType(nodeId, environmentType);
    }
}

void AudioSuiteManagerGetVoiceBeautifierTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::VoiceBeautifierType voiceBeautifierType;
        g_pipeline->GetVoiceBeautifierType(nodeId, voiceBeautifierType);
    }
}

void AudioSuiteEngineGetTempoAndPitchFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        float speed;
        float pitch;
        g_pipeline->GetTempoAndPitch(nodeId, speed, pitch);
    }
}

void AudioSuiteEngineGetPureVoiceChangeOptionFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioPureVoiceChangeOption option;
        g_pipeline->GetPureVoiceChangeOption(nodeId, option);
    }
}

void AudioSuiteEngineGetGeneralVoiceChangeTypeFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioGeneralVoiceChangeType changeType;
        g_pipeline->GetGeneralVoiceChangeType(nodeId, changeType);
    }
}

void AudioSuiteRenderFrameFuzzTest()
{
    if (g_pipeline == nullptr) {
        return;
    }
    uint8_t audioData[MAX_FRAME_SIZE] = {0};
    uint32_t frameSize = GetData<uint32_t>() % MAX_FRAME_SIZE;
    for (uint32_t i = 0; i < frameSize; i++) {
        audioData[i] = GetData<uint8_t>();
    }
    int32_t writeLen = GetData<int32_t>();
    bool finishedFlag = GetData<bool>();
    g_pipeline->RenderFrame(audioData, frameSize, &writeLen, &finishedFlag);
}

void AudioSuiteMultiRenderFrameFuzzTest()
{
    if (g_pipeline == nullptr) {
        return;
    }
    uint8_t audioData[MAX_FRAME_SIZE] = {0};
    int32_t arraySize = GetData<int32_t>() % MAX_FRAME_SIZE;
    for (int i = 0; i < arraySize; i++) {
        audioData[i] = GetData<uint8_t>();
    }
    uint8_t *audioDataPtr = audioData;
    int32_t requestFrameSize = GetData<int32_t>();
    int32_t responseSize = GetData<int32_t>();
    bool finishedFlag = GetData<bool>();
    g_pipeline->MultiRenderFrame(&audioDataPtr, arraySize, requestFrameSize, &responseSize, &finishedFlag);
}

void AudioSuiteManagerSetSpaceRenderPositionParamsFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioSpaceRenderPositionParams spaceRenderPosition;
        spaceRenderPosition.x = GetData<float>();
        spaceRenderPosition.y = GetData<float>();
        spaceRenderPosition.z = GetData<float>();
        g_pipeline->SetSpaceRenderPositionParams(nodeId, spaceRenderPosition);
    }
}

void AudioSuiteManagerGetSpaceRenderPositionParamsFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioSpaceRenderPositionParams spaceRenderPosition;
        g_pipeline->GetSpaceRenderPositionParams(nodeId, spaceRenderPosition);
    }
}

void AudioSuiteManagerSetSpaceRenderRotationParamsFuzzTest()
{
    if (g_pipeline == nullptr) {
        return;
    }
    uint32_t nodeId = GetRandomNodeId();
    AudioSuite::AudioSpaceRenderRotationParams spaceRenderRotation;
    spaceRenderRotation.x = GetData<float>();
    spaceRenderRotation.y = GetData<float>();
    spaceRenderRotation.z = GetData<float>();
    spaceRenderRotation.surroundTime = GetData<int32_t>();
    spaceRenderRotation.surroundDirection = GetData<AudioSuite::AudioSurroundDirection>();
    g_pipeline->SetSpaceRenderRotationParams(nodeId, spaceRenderRotation);
}

void AudioSuiteManagerGetSpaceRenderRotationParamsFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioSpaceRenderRotationParams spaceRenderRotation;
        g_pipeline->GetSpaceRenderRotationParams(nodeId, spaceRenderRotation);
    }
}

void AudioSuiteManagerSetSpaceRenderExtensionParamsFuzzTest()
{
    if (g_pipeline == nullptr) {
        return;
    }
    uint32_t nodeId = GetRandomNodeId();
    AudioSuite::AudioSpaceRenderExtensionParams spaceRenderExtension;
    spaceRenderExtension.extRadius = GetData<float>();
    spaceRenderExtension.extAngle = GetData<int32_t>();
    g_pipeline->SetSpaceRenderExtensionParams(nodeId, spaceRenderExtension);
}

void AudioSuiteManagerGetSpaceRenderExtensionParamsFuzzTest()
{
    if (g_pipeline != nullptr) {
        uint32_t nodeId = GetRandomNodeId();
        AudioSuite::AudioSpaceRenderExtensionParams spaceRenderExtension;
        g_pipeline->GetSpaceRenderExtensionParams(nodeId, spaceRenderExtension);
    }
}

void AudioSuiteManagerRegisterDownloadCallbackFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    g_downloadManager->RegisterDownloadStatusCallback(g_downloadCallback);
}

void AudioSuiteManagerStartDownloadFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    char featureName[MAX_FEATURE_NAME_SIZE] = {0};
    uint32_t nameLen = GetData<uint32_t>() % MAX_FEATURE_NAME_SIZE;
    for (uint32_t i = 0; i < nameLen; i++) {
        featureName[i] = GetData<uint8_t>();
    }
    if (nameLen == 0) {
        featureName[0] = 'v';
        featureName[1] = '\0';
    } else {
        featureName[nameLen] = '\0';
    }
    g_downloadManager->StartDownload(std::vector<std::string>{std::string(featureName)}, true);
}

void AudioSuiteManagerCancelDownloadFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    char featureName[MAX_FEATURE_NAME_SIZE] = {0};
    uint32_t nameLen = GetData<uint32_t>() % MAX_FEATURE_NAME_SIZE;
    for (uint32_t i = 0; i < nameLen; i++) {
        featureName[i] = GetData<uint8_t>();
    }
    if (nameLen == 0) {
        featureName[0] = 'v';
        featureName[1] = '\0';
    } else {
        featureName[nameLen] = '\0';
    }
    g_downloadManager->CancelDownload(std::vector<std::string>{std::string(featureName)});
}

void AudioSuiteManagerStartBackgroundDownloadFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    char featureName[MAX_FEATURE_NAME_SIZE] = {0};
    uint32_t nameLen = GetData<uint32_t>() % MAX_FEATURE_NAME_SIZE;
    for (uint32_t i = 0; i < nameLen; i++) {
        featureName[i] = GetData<uint8_t>();
    }
    if (nameLen == 0) {
        featureName[0] = 'v';
        featureName[1] = '\0';
    } else {
        featureName[nameLen] = '\0';
    }
    g_downloadManager->StartBackgroundDownload(std::vector<std::string>{std::string(featureName)});
}

void AudioSuiteManagerGetDownloadStatusFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    char featureName[MAX_FEATURE_NAME_SIZE] = {0};
    uint32_t nameLen = GetData<uint32_t>() % MAX_FEATURE_NAME_SIZE;
    for (uint32_t i = 0; i < nameLen; i++) {
        featureName[i] = GetData<uint8_t>();
    }
    if (nameLen == 0) {
        featureName[0] = 'v';
        featureName[1] = '\0';
    } else {
        featureName[nameLen] = '\0';
    }
    std::vector<AudioSuite::AudioSuiteDownloadStatusInfo> status;
    g_downloadManager->GetDownloadStatus(std::vector<std::string>{std::string(featureName)}, status);
}

void AudioSuiteManagerGetFeatureNewVersionFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    char featureName[MAX_FEATURE_NAME_SIZE] = {0};
    uint32_t nameLen = GetData<uint32_t>() % MAX_FEATURE_NAME_SIZE;
    for (uint32_t i = 0; i < nameLen; i++) {
        featureName[i] = GetData<uint8_t>();
    }
    if (nameLen == 0) {
        featureName[0] = 'v';
        featureName[1] = '\0';
    } else {
        featureName[nameLen] = '\0';
    }
    std::vector<AudioSuite::AudioSuiteCloudRomVersionInfo> versionInfos;
    g_downloadManager->GetFeatureNewVersion(std::vector<std::string>{std::string(featureName)}, versionInfos);
}

void AudioSuiteManagerUninstallCloudRomFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    char featureName[MAX_FEATURE_NAME_SIZE] = {0};
    uint32_t nameLen = GetData<uint32_t>() % MAX_FEATURE_NAME_SIZE;
    for (uint32_t i = 0; i < nameLen; i++) {
        featureName[i] = GetData<uint8_t>();
    }
    if (nameLen == 0) {
        featureName[0] = 'v';
        featureName[1] = '\0';
    } else {
        featureName[nameLen] = '\0';
    }
    g_downloadManager->UninstallCloudRom(std::vector<std::string>{std::string(featureName)});
}

void AudioSuiteManagerUnregisterDownloadCallbackFuzzTest()
{
    if (!g_downloadManagerInited) {
        g_downloadManager = &AudioSuite::AudioSuiteDownloadManager::GetInstance();
        g_downloadManagerInited = true;
    }
    if (g_downloadManager == nullptr) {
        return;
    }
    g_downloadManager->UnregisterDownloadStatusCallback();
}

vector g_testFuncs = {
    AudioSuiteManagerCreatePipelineFuzzTest,
    AudioSuiteManagerStartPipelineFuzzTest,
    AudioSuiteManagerStopPipelineFuzzTest,
    AudioSuiteManagerGetPipelineStateFuzzTest,
    AudioSuiteManagerBypassEffectNodeFuzzTest,
    AudioSuiteManagerGetNodeBypassStatusFuzzTest,
    AudioSuiteManagerSetAudioFormatFuzzTest,
    AudioSuiteManagerSetRequestDataCallbackFuzzTest,
    AudioSuiteManagerConnectNodesFuzzTest,
    AudioSuiteManagerDisconnectNodesFuzzTest,
    AudioSuiteManagerSetEqFrequencyBandGainsModeFuzzTest,
    AudioSuiteManagerSetSoundFieldTypeFuzzTest,
    AudioSuiteManagerSetEnvironmentTypeFuzzTest,
    AudioSuiteManagerSetVoiceBeautifierTypeFuzzTest,
    AudioSuiteEngineSetTempoAndPitchFuzzTest,
    AudioSuiteEngineSetPureVoiceChangeOptionFuzzTest,
    AudioSuiteEngineSetGeneralVoiceChangeTypeFuzzTest,
    AudioSuiteManagerGetEqFrequencyBandGainsModeFuzzTest,
    AudioSuiteManagerGetSoundFieldTypeFuzzTest,
    AudioSuiteManagerGetEnvironmentTypeFuzzTest,
    AudioSuiteManagerGetVoiceBeautifierTypeFuzzTest,
    AudioSuiteEngineGetTempoAndPitchFuzzTest,
    AudioSuiteEngineGetPureVoiceChangeOptionFuzzTest,
    AudioSuiteEngineGetGeneralVoiceChangeTypeFuzzTest,
    AudioSuiteRenderFrameFuzzTest,
    AudioSuiteMultiRenderFrameFuzzTest,
    AudioSuiteManagerSetSpaceRenderPositionParamsFuzzTest,
    AudioSuiteManagerGetSpaceRenderPositionParamsFuzzTest,
    AudioSuiteManagerSetSpaceRenderRotationParamsFuzzTest,
    AudioSuiteManagerGetSpaceRenderRotationParamsFuzzTest,
    AudioSuiteManagerSetSpaceRenderExtensionParamsFuzzTest,
    AudioSuiteManagerGetSpaceRenderExtensionParamsFuzzTest,
    AudioSuiteManagerRegisterDownloadCallbackFuzzTest,
    AudioSuiteManagerStartDownloadFuzzTest,
    AudioSuiteManagerCancelDownloadFuzzTest,
    AudioSuiteManagerStartBackgroundDownloadFuzzTest,
    AudioSuiteManagerGetDownloadStatusFuzzTest,
    AudioSuiteManagerGetFeatureNewVersionFuzzTest,
    AudioSuiteManagerUninstallCloudRomFuzzTest,
    AudioSuiteManagerUnregisterDownloadCallbackFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::EnsureFuzzSetup();
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
