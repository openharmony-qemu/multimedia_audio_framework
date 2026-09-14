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

#include "audio_log.h"
#include "../fuzz_utils.h"
#include "native_audio_suite_base.h"
#include "OHAudioSuiteEngine.h"
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
static const uint32_t MAX_NODE_NUM = 5;
static const uint32_t MAX_CHANNEL_NUM = 16;
static const uint32_t MAX_FRAME_SIZE = 1000;
static const uint32_t MAX_FEATURE_NAME_SIZE = 256;
static bool g_downloadManagerInited = false;
AudioSuite::AudioSuiteDownloadManager *g_downloadManager = nullptr;

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

void AudioSuiteManagerInitFuzzTest()
{
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().Init();
}

void AudioSuiteManagerDeInitFuzzTest()
{
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().DeInit();
}

void AudioSuiteManagerCreatePipelineFuzzTest()
{
    AudioSuite::PipelineWorkMode workMode = GetData<AudioSuite::PipelineWorkMode>();
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().CreatePipeline(pipelineId, workMode);
}

void AudioSuiteManagerDestroyPipelineFuzzTest()
{
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().DestroyPipeline(pipelineId);
}

void AudioSuiteManagerStartPipelineFuzzTest()
{
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().StartPipeline(pipelineId);
}

void AudioSuiteManagerStopPipelineFuzzTest()
    {
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().StopPipeline(pipelineId);
}

void AudioSuiteManagerGetPipelineStateFuzzTest()
{
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    AudioSuite::AudioSuitePipelineState pipelineState;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetPipelineState(pipelineId, pipelineState);
}

void AudioSuiteManagerCreateNodeFuzzTest()
{
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioNodeBuilder nodeBuilder;
    nodeBuilder.nodeType = GetData<AudioSuite::AudioNodeType>();
    nodeBuilder.nodeFormat.audioChannelInfo.channelLayout = GetData<AudioChannelLayout>();
    nodeBuilder.nodeFormat.audioChannelInfo.numChannels = GetData<uint32_t>() % MAX_CHANNEL_NUM;
    nodeBuilder.nodeFormat.format = GetData<AudioSampleFormat>();
    nodeBuilder.nodeFormat.rate = GetData<AudioSamplingRate>();
    nodeBuilder.nodeFormat.encodingType = GetData<AudioSuite::AudioStreamEncodingType>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().CreateNode(pipelineId, nodeBuilder, nodeId);
}

void AudioSuiteManagerDestroyNodeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().DestroyNode(nodeId);
}

void AudioSuiteManagerBypassEffectNodeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    bool nodeEnable = GetData<bool>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().BypassEffectNode(nodeId, nodeEnable);
}

void AudioSuiteManagerGetNodeBypassStatusFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    bool nodeEnable;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetNodeBypassStatus(nodeId, nodeEnable);
}

void AudioSuiteManagerSetAudioFormatFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioFormat nodeFormat;
    nodeFormat.audioChannelInfo.channelLayout = GetData<AudioChannelLayout>();
    nodeFormat.audioChannelInfo.numChannels = GetData<uint32_t>() % MAX_CHANNEL_NUM;
    nodeFormat.format = GetData<AudioSampleFormat>();
    nodeFormat.rate = GetData<AudioSamplingRate>();
    nodeFormat.encodingType = GetData<AudioSuite::AudioStreamEncodingType>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetAudioFormat(nodeId, nodeFormat);
}

void AudioSuiteManagerSetRequestDataCallbackFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    std::shared_ptr<AudioSuite::InputNodeRequestDataCallBack> callback =
        std::make_shared<InputNodeRequestDataCallBackTestImpl>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetRequestDataCallback(nodeId, callback);
}

void AudioSuiteManagerConnectNodesFuzzTest()
{
    uint32_t srcNodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    uint32_t dstNodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().ConnectNodes(srcNodeId, dstNodeId);
}

void AudioSuiteManagerDisConnectNodesFuzzTest()
{
    uint32_t srcNodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    uint32_t dstNodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().DisConnectNodes(srcNodeId, dstNodeId);
}

void AudioSuiteManagerSetEqFrequencyBandGainsModeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioEqualizerFrequencyBandGains gains;
    for (int i = 0; i < EQUALIZER_BAND_NUM; i++) {
        gains.gains[i] = GetData<int32_t>();
    }
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetEqualizerFrequencyBandGains(nodeId, gains);
}

void AudioSuiteManagerSetSoundFieldTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::SoundFieldType soundFieldType = GetData<AudioSuite::SoundFieldType>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetSoundFieldType(nodeId, soundFieldType);
}

void AudioSuiteManagerSetEnvironmentTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::EnvironmentType environmentType = GetData<AudioSuite::EnvironmentType>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetEnvironmentType(nodeId, environmentType);
}

void AudioSuiteManagerSetVoiceBeautifierTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::VoiceBeautifierType voiceBeautifierType = GetData<AudioSuite::VoiceBeautifierType>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetVoiceBeautifierType(nodeId, voiceBeautifierType);
}

void AudioSuiteEngineSetTempoAndPitchFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    float speed = GetData<float>();
    float pitch = GetData<float>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetTempoAndPitch(nodeId, speed, pitch);
}

void AudioSuiteEngineSetPureVoiceChangeOptionFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioPureVoiceChangeOption option;
    option.optionGender = GetData<AudioSuite::AudioPureVoiceChangeGenderOption>();
    option.optionType = GetData<AudioSuite::AudioPureVoiceChangeType>();
    option.pitch = GetData<float>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetPureVoiceChangeOption(nodeId, option);
}

void AudioSuiteEngineSetGeneralVoiceChangeTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioGeneralVoiceChangeType changeType = GetData<AudioSuite::AudioGeneralVoiceChangeType>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetGeneralVoiceChangeType(nodeId, changeType);
}

void AudioSuiteManagerGetEqFrequencyBandGainsModeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioEqualizerFrequencyBandGains gains;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetEqualizerFrequencyBandGains(nodeId, gains);
}

void AudioSuiteManagerGetSoundFieldTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::SoundFieldType soundFieldType;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetSoundFieldType(nodeId, soundFieldType);
}

void AudioSuiteManagerGetEnvironmentTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::EnvironmentType environmentType;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetEnvironmentType(nodeId, environmentType);
}

void AudioSuiteManagerGetVoiceBeautifierTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::VoiceBeautifierType voiceBeautifierType;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetVoiceBeautifierType(nodeId, voiceBeautifierType);
}

void AudioSuiteEngineGetTempoAndPitchFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    float speed;
    float pitch;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetTempoAndPitch(nodeId, speed, pitch);
}

void AudioSuiteEngineGetPureVoiceChangeOptionFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioPureVoiceChangeOption option;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetPureVoiceChangeOption(nodeId, option);
}

void AudioSuiteEngineGetGeneralVoiceChangeTypeFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioGeneralVoiceChangeType changeType;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetGeneralVoiceChangeType(nodeId, changeType);
}

void AudioSuiteRenderFrameFuzzTest()
{
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    uint8_t audioData[MAX_FRAME_SIZE] = {0};
    uint32_t frameSize = GetData<uint32_t>() % MAX_FRAME_SIZE;
    for (uint32_t i = 0; i < frameSize; i++) {
        audioData[i] = GetData<uint8_t>();
    }
    int32_t writeLen = GetData<int32_t>();
    bool finishedFlag = GetData<bool>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().RenderFrame(pipelineId, audioData,
        frameSize, &writeLen, &finishedFlag);
}

void AudioSuiteMultiRenderFrameFuzzTest()
{
    uint32_t pipelineId = GetData<uint32_t>() % MAX_PIPELINE_NUM;
    AudioSuite::AudioDataArray audioDataArray;
    uint8_t audioData[MAX_FRAME_SIZE] = {0};
    audioDataArray.arraySize = GetData<int32_t>() % MAX_FRAME_SIZE;
    for (int i = 0; i < audioDataArray.arraySize; i++) {
        audioData[i] = GetData<uint8_t>();
    }
    audioDataArray.audioDataArray = (void**)&audioData;
    audioDataArray.requestFrameSize = GetData<int32_t>();
    int32_t responseSize = GetData<int32_t>();
    bool finishedFlag = GetData<bool>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().MultiRenderFrame(pipelineId, &audioDataArray,
        &responseSize, &finishedFlag);
}

void AudioSuiteManagerSetSpaceRenderPositionParamsFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioSpaceRenderPositionParams spaceRenderPosition;
    spaceRenderPosition.x = GetData<float>();
    spaceRenderPosition.y = GetData<float>();
    spaceRenderPosition.z = GetData<float>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetSpaceRenderPositionParams(nodeId, spaceRenderPosition);
}
 
void AudioSuiteManagerGetSpaceRenderPositionParamsFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioSpaceRenderPositionParams spaceRenderPosition;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetSpaceRenderPositionParams(nodeId, spaceRenderPosition);
}
 
void AudioSuiteManagerSetSpaceRenderRotationParamsFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioSpaceRenderRotationParams spaceRenderRotation;
    spaceRenderRotation.x = GetData<float>();
    spaceRenderRotation.y = GetData<float>();
    spaceRenderRotation.z = GetData<float>();
    spaceRenderRotation.surroundTime = GetData<int32_t>();
    spaceRenderRotation.surroundDirection = GetData<AudioSuite::AudioSurroundDirection>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetSpaceRenderRotationParams(nodeId, spaceRenderRotation);
}
 
void AudioSuiteManagerGetSpaceRenderRotationParamsFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioSpaceRenderRotationParams spaceRenderRotation;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetSpaceRenderRotationParams(nodeId, spaceRenderRotation);
}
 
void AudioSuiteManagerSetSpaceRenderExtensionParamsFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioSpaceRenderExtensionParams spaceRenderExtension;
    spaceRenderExtension.extRadius = GetData<float>();
    spaceRenderExtension.extAngle = GetData<int32_t>();
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().SetSpaceRenderExtensionParams(nodeId, spaceRenderExtension);
}
 
void AudioSuiteManagerGetSpaceRenderExtensionParamsFuzzTest()
{
    uint32_t nodeId = GetData<uint32_t>() % MAX_NODE_NUM;
    AudioSuite::AudioSpaceRenderExtensionParams spaceRenderExtension;
    AudioSuite::IAudioSuiteManager::GetAudioSuiteManager().GetSpaceRenderExtensionParams(nodeId, spaceRenderExtension);
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
    AudioSuiteManagerInitFuzzTest,
    AudioSuiteManagerDeInitFuzzTest,
    AudioSuiteManagerCreatePipelineFuzzTest,
    AudioSuiteManagerDestroyPipelineFuzzTest,
    AudioSuiteManagerStartPipelineFuzzTest,
    AudioSuiteManagerStopPipelineFuzzTest,
    AudioSuiteManagerGetPipelineStateFuzzTest,
    AudioSuiteManagerCreateNodeFuzzTest,
    AudioSuiteManagerDestroyNodeFuzzTest,
    AudioSuiteManagerBypassEffectNodeFuzzTest,
    AudioSuiteManagerGetNodeBypassStatusFuzzTest,
    AudioSuiteManagerSetAudioFormatFuzzTest,
    AudioSuiteManagerSetRequestDataCallbackFuzzTest,
    AudioSuiteManagerConnectNodesFuzzTest,
    AudioSuiteManagerDisConnectNodesFuzzTest,
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

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}