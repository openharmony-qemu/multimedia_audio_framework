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
#include "audio_errors.h"
#include "hpae_capturer_manager.h"
#include "audio_info.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

vector<HpaeProcessorType> HpaeProcessorTypeVec = {
    HPAE_SCENE_DEFAULT,
    HPAE_SCENE_MUSIC,
    HPAE_SCENE_GAME,
    HPAE_SCENE_MOVIE,
    HPAE_SCENE_SPEECH,
    HPAE_SCENE_RING,
    HPAE_SCENE_VOIP_DOWN,
    HPAE_SCENE_OTHERS,
    HPAE_SCENE_EFFECT_NONE,
    HPAE_SCENE_EFFECT_OUT,
    HPAE_SCENE_SPLIT_MEDIA,
    HPAE_SCENE_SPLIT_NAVIGATION,
    HPAE_SCENE_SPLIT_COMMUNICATION,
    HPAE_SCENE_VOIP_UP,
    HPAE_SCENE_RECORD,
    HPAE_SCENE_PRE_ENHANCE,
    HPAE_SCENE_ASR,
    HPAE_SCENE_VOICE_MESSAGE,
    HPAE_SCENE_COLLABORATIVE,
};

vector<AudioEnhanceScene> AudioEnhanceSceneVec = {
    SCENE_VOIP_UP,
    SCENE_RECORD,
    SCENE_PRE_ENHANCE,
    SCENE_ASR,
    SCENE_VOICE_MESSAGE,
    SCENE_NONE,
};

vector<HpaeSessionState> HpaeSessionStateVec = {
    HPAE_SESSION_INVALID,
    HPAE_SESSION_NEW,
    HPAE_SESSION_PREPARED,
    HPAE_SESSION_RUNNING,
    HPAE_SESSION_PAUSING,
    HPAE_SESSION_PAUSED,
    HPAE_SESSION_STOPPING,
    HPAE_SESSION_STOPPED,
    HPAE_SESSION_RELEASED
};

vector<IOperation> IOperationVec = {
    OPERATION_INVALID,
    OPERATION_STARTED,
    OPERATION_PAUSED,
    OPERATION_STOPPED,
    OPERATION_FLUSHED,
    OPERATION_DRAINED,
    OPERATION_RELEASED,
    OPERATION_UNDERRUN,
    OPERATION_UNDERFLOW,
    OPERATION_SET_OFFLOAD_ENABLE,
    OPERATION_UNSET_OFFLOAD_ENABLE,
    OPERATION_DATA_LINK_CONNECTING,
    OPERATION_DATA_LINK_CONNECTED,
};

vector<MoveSessionType> MoveSessionTypeVec = {
    MOVE_SINGLE,
    MOVE_ALL,
    MOVE_PREFER,
};

HPAE::HpaeSourceInfo GetSourceInfo()
{
    HPAE::HpaeSourceInfo sourceInfo;
    sourceInfo.sourceId = g_fuzzUtils.GetData<uint32_t>();
    sourceInfo.deviceNetId = "device_net_id";
    sourceInfo.deviceClass = "device_class";
    sourceInfo.adapterName = "adapter_name";
    sourceInfo.sourceName = "source_name";
    sourceInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    sourceInfo.filePath = "/data/test_file.wav";
    sourceInfo.deviceName = "device_name";
    sourceInfo.frameLen = g_fuzzUtils.GetData<size_t>();
    sourceInfo.format = SAMPLE_U8;
    sourceInfo.channels = CHANNEL_3;
    sourceInfo.channelLayout = g_fuzzUtils.GetData<uint64_t>();
    sourceInfo.volume = g_fuzzUtils.GetData<float>();
    sourceInfo.ecType = HPAE_EC_TYPE_SAME_ADAPTER;
    sourceInfo.ecFormat = SAMPLE_U8;
    sourceInfo.ecChannels = CHANNEL_4;
    sourceInfo.micRefFormat = SAMPLE_U8;
    sourceInfo.micRefChannels = CHANNEL_5;
    sourceInfo.ecAdapterName = "ec_adapter_name";
    return sourceInfo;
}

void CapturerSourceStartFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    hpaeCapturerManager->CapturerSourceStart();
}

void CreateOutputSessionFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo;
    uint32_t sessionId = 0;
    hpaeCapturerManager->CreateOutputSession(streamInfo);
    hpaeCapturerManager->DeleteOutputSession(sessionId);
}

void CaptureEffectCreateFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % HpaeProcessorTypeVec.size();
    HpaeProcessorType processorType = HpaeProcessorTypeVec[index];
    index = g_fuzzUtils.GetData<uint32_t>() % AudioEnhanceSceneVec.size();
    AudioEnhanceScene sceneType = AudioEnhanceSceneVec[index];
    HpaeNodeInfo nodeInfo;
    if (hpaeCapturerManager->sceneClusterMap_[processorType] == nullptr) {
        hpaeCapturerManager->sceneClusterMap_[processorType] = std::make_shared<HpaeSourceProcessCluster>(
            nodeInfo);
    }
    hpaeCapturerManager->CaptureEffectCreate(processorType, sceneType);
    hpaeCapturerManager->CaptureEffectRelease(processorType);
}

void SetSessionStateFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeNodeInfo nodeInfo;
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    HpaeCapturerSessionInfo sessionInfo;
    hpaeCapturerManager->sessionNodeMap_[sessionId] = sessionInfo;
    hpaeCapturerManager->sourceOutputNodeMap_[sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % HpaeSessionStateVec.size();
    HpaeSessionState capturerState = HpaeSessionStateVec[index];
    hpaeCapturerManager->SetSessionState(sessionId, capturerState);
}

void SetMuteFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    bool isMute = g_fuzzUtils.GetData<bool>();
    hpaeCapturerManager->SetMute(isMute);
}

void PrepareCapturerFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeNodeInfo ecNodeInfo;
    hpaeCapturerManager->sourceInfo_ = sourceInfo;
    hpaeCapturerManager->sourceInfo_.ecType = g_fuzzUtils.GetData<HpaeEcType>();
    hpaeCapturerManager->PrepareCapturerEc(ecNodeInfo);
    hpaeCapturerManager->sourceInfo_.micRef =  g_fuzzUtils.GetData<HpaeMicRefSwitch>();
    hpaeCapturerManager->PrepareCapturerMicRef(ecNodeInfo);
    hpaeCapturerManager->GetSourceInfo();
    hpaeCapturerManager->GetAllSourceOutputsInfo();
}

void CheckIfAnyStreamRunningFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    HpaeCapturerSessionInfo sessionInfo;
    sessionInfo.state = HPAE_SESSION_RUNNING;
    hpaeCapturerManager->sessionNodeMap_[sessionId] = sessionInfo;
    hpaeCapturerManager->CheckIfAnyStreamRunning();
}

void DumpSourceInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    hpaeCapturerManager->isInit_ = true;
    hpaeCapturerManager->DumpSourceInfo();
}

void OnRequestLatencyFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    hpaeCapturerManager->OnRequestLatency(sessionId, latency);
}

void OnNotifyQueueFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    hpaeCapturerManager->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    if (hpaeCapturerManager->hpaeSignalProcessThread_ == nullptr) {
        return;
    }
    hpaeCapturerManager->OnNotifyQueue();
}

void MoveStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeNodeInfo nodeInfo;
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    HpaeCapturerSessionInfo sessionInfo;
    hpaeCapturerManager->sessionNodeMap_[sessionId] = sessionInfo;
    hpaeCapturerManager->sourceOutputNodeMap_[sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    if (hpaeCapturerManager->sourceOutputNodeMap_.empty()) {
        return;
    }
    std::string sourceName = "source_name";
    hpaeCapturerManager->MoveStream(sessionId, sourceName);
}

void DeactivateThreadFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    hpaeCapturerManager->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    if (hpaeCapturerManager->hpaeSignalProcessThread_ == nullptr) {
        return;
    }
    hpaeCapturerManager->DeactivateThread();
}

void GetSourceOutputInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    HpaeSourceOutputInfo sourceOutputInfo;
    hpaeCapturerManager->GetSourceOutputInfo(sessionId, sourceOutputInfo);
}

void RegisterReadCallbackFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeNodeInfo nodeInfo;
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeCapturerManager->sourceOutputNodeMap_[sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    if (hpaeCapturerManager->sourceOutputNodeMap_.empty()) {
        return;
    }
    const std::weak_ptr<ICapturerStreamCallback> callback = std::weak_ptr<ICapturerStreamCallback>();
    hpaeCapturerManager->RegisterReadCallback(sessionId, callback);
}

void OnNodeStatusUpdateFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % IOperationVec.size();
    IOperation operation = IOperationVec[index];
    hpaeCapturerManager->OnNodeStatusUpdate(sessionId, operation);
}

void AddAllNodesToSourceFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeCaptureMoveInfo moveInfo;
    HpaeNodeInfo nodeInfo;
    moveInfo.sessionId = g_fuzzUtils.GetData<uint32_t>();
    moveInfo.sessionInfo.sceneType = HPAE_SCENE_DEFAULT;
    moveInfo.sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    moveInfos.push_back(moveInfo);
    bool isConnect = g_fuzzUtils.GetData<bool>();
    hpaeCapturerManager->AddAllNodesToSource(moveInfos, isConnect);
}

void AddNodeToSourceFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeCaptureMoveInfo moveInfo;
    hpaeCapturerManager->AddNodeToSource(moveInfo);
}

void AddSingleNodeToSourceFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    HpaeCaptureMoveInfo moveInfo;
    bool isConnect = g_fuzzUtils.GetData<bool>();
    hpaeCapturerManager->AddSingleNodeToSource(moveInfo, isConnect);
}

void MoveAllStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    std::string sourceName = "source_name";
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    std::vector<uint32_t> sessionIds;
    sessionIds.push_back(sessionId);
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % MoveSessionTypeVec.size();
    MoveSessionType moveType = MoveSessionTypeVec[index];
    hpaeCapturerManager->MoveAllStream(sourceName, sessionIds, moveType);
}

void UpdateAppsUidAndSessionIdFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    int32_t appsUid = g_fuzzUtils.GetData<int32_t>();
    int32_t sessionId = g_fuzzUtils.GetData<int32_t>();
    hpaeCapturerManager->appsUid_.push_back(appsUid);
    hpaeCapturerManager->sessionsId_.push_back(sessionId);
    HpaeNodeInfo nodeInfo;
    hpaeCapturerManager->sourceOutputNodeMap_[sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    if (hpaeCapturerManager->sourceOutputNodeMap_.empty()) {
        return;
    }
    hpaeCapturerManager->UpdateAppsUidAndSessionId();
}

void ReleaseFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    int32_t sessionId = g_fuzzUtils.GetData<int32_t>();
    hpaeCapturerManager->Release(sessionId);
}

void DrainFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    hpaeCapturerManager->isInit_ = true;
    int32_t sessionId = g_fuzzUtils.GetData<int32_t>();
    HpaeNodeInfo nodeInfo;
    hpaeCapturerManager->sourceOutputNodeMap_[sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    if (hpaeCapturerManager->sourceOutputNodeMap_.empty()) {
        return;
    }
    hpaeCapturerManager->Drain(sessionId);
}

void FlushFuzzTest()
{
    HpaeSourceInfo sourceInfo = GetSourceInfo();
    std::shared_ptr<HpaeCapturerManager> hpaeCapturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    if (hpaeCapturerManager == nullptr) {
        return;
    }
    hpaeCapturerManager->isInit_ = true;
    int32_t sessionId = g_fuzzUtils.GetData<int32_t>();
    HpaeNodeInfo nodeInfo;
    hpaeCapturerManager->sourceOutputNodeMap_[sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    if (hpaeCapturerManager->sourceOutputNodeMap_.empty()) {
        return;
    }
    hpaeCapturerManager->Flush(sessionId);
}

vector<TestFuncs> g_testFuncs = {
    CapturerSourceStartFuzzTest,
    CreateOutputSessionFuzzTest,
    CaptureEffectCreateFuzzTest,
    SetSessionStateFuzzTest,
    SetMuteFuzzTest,
    PrepareCapturerFuzzTest,
    CheckIfAnyStreamRunningFuzzTest,
    DumpSourceInfoFuzzTest,
    OnRequestLatencyFuzzTest,
    OnNotifyQueueFuzzTest,
    MoveStreamFuzzTest,
    DeactivateThreadFuzzTest,
    GetSourceOutputInfoFuzzTest,
    OnNodeStatusUpdateFuzzTest,
    AddAllNodesToSourceFuzzTest,
    AddNodeToSourceFuzzTest,
    AddSingleNodeToSourceFuzzTest,
    MoveAllStreamFuzzTest,
    UpdateAppsUidAndSessionIdFuzzTest,
    ReleaseFuzzTest,
    DrainFuzzTest,
    FlushFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}