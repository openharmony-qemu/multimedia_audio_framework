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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const uint32_t RESIZENUM = 2;
const uint32_t IDNUM = 2;
const uint32_t NUM_100 = 100;
const uint32_t NUM_1000 = 1000;
const uint32_t NUM_2000 = 2000;
const int32_t MEDIA_SERVICE_UID = 1013;
typedef void (*TestFuncs)();

vector<AudioFlag> AudioFlagVec = {
    AUDIO_FLAG_NONE,
    AUDIO_OUTPUT_FLAG_NORMAL,
    AUDIO_OUTPUT_FLAG_DIRECT,
    AUDIO_OUTPUT_FLAG_HD,
    AUDIO_OUTPUT_FLAG_MULTICHANNEL,
    AUDIO_OUTPUT_FLAG_LOWPOWER,
    AUDIO_OUTPUT_FLAG_FAST,
    AUDIO_OUTPUT_FLAG_VOIP,
    AUDIO_OUTPUT_FLAG_VOIP_FAST,
    AUDIO_OUTPUT_FLAG_HWDECODING,
    AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD,
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
    AUDIO_INPUT_FLAG_VOIP,
    AUDIO_INPUT_FLAG_VOIP_FAST,
    AUDIO_INPUT_FLAG_WAKEUP,
    AUDIO_INPUT_FLAG_AI,
    AUDIO_INPUT_FLAG_ULTRASONIC,
    AUDIO_FLAG_MAX,
};

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void RemoveAudioPipeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> targetPipe = std::make_shared<AudioPipeInfo>();
    targetPipe->adapterName_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    targetPipe->routeFlag_ = fdp.ConsumeIntegral<uint32_t>();

    audioPipeManager->AddAudioPipeInfo(targetPipe);
    audioPipeManager->RemoveAudioPipeInfo(targetPipe);
    auto pipeList = audioPipeManager->GetPipeList();
}

void RemoveAudioPipeInfoByIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> targetPipe = std::make_shared<AudioPipeInfo>();
    targetPipe->id_ = fdp.ConsumeIntegral<uint32_t>();
    targetPipe->adapterName_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);

    audioPipeManager->GetPipeList();
    audioPipeManager->AddAudioPipeInfo(targetPipe);
    audioPipeManager->RemoveAudioPipeInfo(targetPipe->id_);
    audioPipeManager->GetPipeList();
}

void UpdateAudioPipeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    audioPipeManager->GetPipeList();

    std::shared_ptr<AudioPipeInfo> existingPipe = std::make_shared<AudioPipeInfo>();
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uint32_t routeFlag = fdp.ConsumeIntegral<uint32_t>();
    existingPipe->adapterName_ = adapterName;
    existingPipe->routeFlag_ = routeFlag;
    existingPipe->id_ = IDNUM;
    audioPipeManager->AddAudioPipeInfo(existingPipe);

    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = adapterName;
    newPipe->routeFlag_ = routeFlag;
    newPipe->id_ = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->UpdateAudioPipeInfo(newPipe);
    audioPipeManager->GetPipeList();
}

void IsSamePipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uint32_t routeFlag = fdp.ConsumeIntegral<uint32_t>();
    info->adapterName_ = adapterName;
    info->routeFlag_ = routeFlag;
    info->id_ = fdp.ConsumeIntegral<uint32_t>();

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = adapterName;
    cmpInfo->routeFlag_ = routeFlag;
    cmpInfo->id_ = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->IsSamePipe(info, cmpInfo);
}

void GetUnusedPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);

    audioPipeManager->GetUnusedPipe();
}

void IsSpecialPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (AudioFlagVec.size() == 0) {
        return;
    }
    uint32_t routeFlag = PickValue(fdp, AudioFlagVec);
    audioPipeManager->IsSpecialPipe(routeFlag);
}

void IsNormalRecordPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (AudioFlagVec.size() == 0 || audioPipeManager == nullptr) {
        return;
    }
    shared_ptr<AudioPipeInfo> pipe = std::make_shared<AudioPipeInfo>();
    pipe->adapterName_ = fdp.ConsumeBool() ? PRIMARY_CLASS : USB_CLASS;
    pipe->routeFlag_ = PickValue(fdp, AudioFlagVec);
    audioPipeManager->IsNormalRecordPipe(pipe);
}

void GetStreamDescByIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->GetStreamDescById(targetSessionId);
}

void DumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    std::string dumpString = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioPipeManager->Dump(dumpString);
}

void IsModemCommunicationIdExistFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->IsModemCommunicationIdExist(sessionId);
}

void GetModemCommunicationStreamDescByIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->modemCommunicationIdMap_.clear();
    audioPipeManager->GetModemCommunicationStreamDescById(sessionId);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();
    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);
    audioPipeManager->GetModemCommunicationStreamDescById(sessionId);
}

void GetPipeinfoByNameAndFlagFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uint32_t routeFlag = fdp.ConsumeIntegral<uint32_t>();
    pipe1->adapterName_ = adapterName;
    pipe1->routeFlag_ = routeFlag;
    audioPipeManager->AddAudioPipeInfo(pipe1);

    std::string targetAdapterName = adapterName;
    uint32_t targetRouteFlag = routeFlag;
    audioPipeManager->GetPipeinfoByNameAndFlag(targetAdapterName, targetRouteFlag);
}

void GetModuleNameBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    uint32_t targetSessionId = fdp.ConsumeIntegral<uint32_t>();

    audioPipeManager->GetModuleNameBySessionId(targetSessionId);
}

void GetProcessDeviceInfoBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = fdp.ConsumeIntegral<uint32_t>();
    AudioStreamInfo info;
    audioPipeManager->GetProcessDeviceInfoBySessionId(targetSessionId, info);
}

void GetAllOutputStreamDescsFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    audioPipeManager->GetAllOutputStreamDescs();
}

void GetAllInputStreamDescsFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    audioPipeManager->GetAllInputStreamDescs();
}

void GetStreamDescByIdInnerFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->GetStreamDescByIdInner(targetSessionId);
}

void GetStreamCountFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uint32_t routeFlag = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->adapterName_ = adapterName;
    pipeInfo->routeFlag_ = routeFlag;
    pipeInfo->streamDescriptors_.resize(RESIZENUM);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetAdapterName = adapterName;
    uint32_t targetRouteFlag = routeFlag;
    audioPipeManager->GetStreamCount(targetAdapterName, targetRouteFlag);
}

void GetPaIndexByIoHandleFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->paIndex_ = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    AudioIOHandle targetId = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->GetPaIndexByIoHandle(targetId);
}

void UpdateRendererPipeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> inputPipe = std::make_shared<AudioPipeInfo>();
    inputPipe->pipeRole_ = PIPE_ROLE_INPUT;
    audioPipeManager->AddAudioPipeInfo(inputPipe);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfos.push_back(newPipe);

    audioPipeManager->UpdateRendererPipeInfos(pipeInfos);
    audioPipeManager->GetPipeList();
}

void UpdateCapturerPipeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    audioPipeManager->AddAudioPipeInfo(outputPipe);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfos.push_back(newPipe);
    audioPipeManager->UpdateCapturerPipeInfos(pipeInfos);

    audioPipeManager->GetPipeList();
}

void PcmOffloadSessionCountFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->streamDescriptors_.resize(RESIZENUM);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    audioPipeManager->PcmOffloadSessionCount();
}

void AddModemCommunicationIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();
    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);

    audioPipeManager->GetModemCommunicationMap();
}

void RemoveModemCommunicationIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    audioPipeManager->modemCommunicationIdMap_[sessionId] = streamDesc;
    audioPipeManager->RemoveModemCommunicationId(sessionId);
    audioPipeManager->GetModemCommunicationMap();
}

void GetNormalSourceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> bluetoothPipe = std::make_shared<AudioPipeInfo>();
    bluetoothPipe->moduleInfo_.name = PRIMARY_MIC;
    bluetoothPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    audioPipeManager->AddAudioPipeInfo(bluetoothPipe);

    bool isEcFeatureEnable = fdp.ConsumeBool();
    audioPipeManager->GetNormalSourceInfo(isEcFeatureEnable);
}

void GetPipeByModuleAndFlagFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::string moduleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uint32_t routeFlag = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->moduleInfo_.name = moduleName;
    pipeInfo->routeFlag_ = routeFlag;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetModuleName = fdp.ConsumeBool() ? moduleName : fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uint32_t targetRouteFlag = routeFlag;
    audioPipeManager->GetPipeByModuleAndFlag(targetModuleName, targetRouteFlag);
}

void StartClientFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    desc->sessionId_ = sessionId;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    audioPipeManager->StartClient(sessionId);

    uint32_t invalidSessionId = sessionId + 1;
    audioPipeManager->StartClient(invalidSessionId);
}

void GetUnusedRecordPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> softLinkPipe = std::make_shared<AudioPipeInfo>();
    softLinkPipe->pipeRole_ = PIPE_ROLE_INPUT;
    softLinkPipe->adapterName_ = PRIMARY_CLASS;
    softLinkPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    softLinkPipe->softLinkFlag_ = true;
    softLinkPipe->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(softLinkPipe);

    std::shared_ptr<AudioPipeInfo> unusedPipe = std::make_shared<AudioPipeInfo>();
    unusedPipe->pipeRole_ = PIPE_ROLE_INPUT;
    unusedPipe->adapterName_ = PRIMARY_CLASS;
    unusedPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    unusedPipe->softLinkFlag_ = false;
    unusedPipe->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(unusedPipe);

    std::shared_ptr<AudioPipeInfo> usedPipe = std::make_shared<AudioPipeInfo>();
    usedPipe->pipeRole_ = PIPE_ROLE_INPUT;
    usedPipe->adapterName_ = PRIMARY_CLASS;
    usedPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    usedPipe->streamDescriptors_.push_back(std::make_shared<AudioStreamDescriptor>());
    audioPipeManager->AddAudioPipeInfo(usedPipe);

    std::shared_ptr<AudioPipeInfo> outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    outputPipe->adapterName_ = PRIMARY_CLASS;
    outputPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    outputPipe->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(outputPipe);

    audioPipeManager->GetUnusedRecordPipe();
}

void GetAdapterNameBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    desc->sessionId_ = sessionId;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    audioPipeManager->GetAdapterNameBySessionId(sessionId);

    uint32_t invalidSessionId = sessionId + 1;
    audioPipeManager->GetAdapterNameBySessionId(invalidSessionId);

    std::shared_ptr<AudioPipeInfo> pipeInfo2 = std::make_shared<AudioPipeInfo>();
    pipeInfo2->adapterName_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    desc2->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    desc2->newDeviceDescs_.clear();
    pipeInfo2->streamDescriptors_.push_back(desc2);
    audioPipeManager->AddAudioPipeInfo(pipeInfo2);

    audioPipeManager->GetAdapterNameBySessionId(desc2->sessionId_);
}

void GetClientUidBySessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo1 = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    uint32_t sessionId1 = fdp.ConsumeIntegral<uint32_t>();
    desc1->sessionId_ = sessionId1;
    desc1->callerUid_ = MEDIA_SERVICE_UID;
    desc1->appInfo_.appUid = NUM_1000;
    pipeInfo1->streamDescriptors_.push_back(desc1);
    audioPipeManager->AddAudioPipeInfo(pipeInfo1);

    audioPipeManager->GetClientUidBySessionId(sessionId1);

    std::shared_ptr<AudioPipeInfo> pipeInfo2 = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    uint32_t sessionId2 = fdp.ConsumeIntegral<uint32_t>();
    desc2->sessionId_ = sessionId2;
    desc2->callerUid_ = NUM_2000;
    pipeInfo2->streamDescriptors_.push_back(desc2);
    audioPipeManager->AddAudioPipeInfo(pipeInfo2);

    audioPipeManager->GetClientUidBySessionId(sessionId2);

    uint32_t invalidSessionId = sessionId2 + 1;
    audioPipeManager->GetClientUidBySessionId(invalidSessionId);
}

void GetModemCommunicationStreamDescFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->modemCommunicationIdMap_.clear();

    audioPipeManager->GetModemCommunicationStreamDesc();

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);

    audioPipeManager->GetModemCommunicationStreamDesc();
}

void UpdateModemStreamStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->modemCommunicationIdMap_.clear();

    AudioStreamStatus status = STREAM_STATUS_STARTED;
    audioPipeManager->UpdateModemStreamStatus(status);

    uint32_t sessionId1 = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->modemCommunicationIdMap_[sessionId1] = nullptr;
    audioPipeManager->UpdateModemStreamStatus(status);

    uint32_t sessionId2 = sessionId1 + 1;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_[sessionId2] = streamDesc;
    audioPipeManager->UpdateModemStreamStatus(status);
}

void UpdateModemStreamDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->modemCommunicationIdMap_.clear();

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    deviceDescs.push_back(std::make_shared<AudioDeviceDescriptor>());
    deviceDescs.front()->deviceType_ = DEVICE_TYPE_SPEAKER;

    audioPipeManager->UpdateModemStreamDevice(deviceDescs);

    uint32_t sessionId1 = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->modemCommunicationIdMap_[sessionId1] = nullptr;
    audioPipeManager->UpdateModemStreamDevice(deviceDescs);
    uint32_t sessionId2 = sessionId1 + 1;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_[sessionId2] = streamDesc;
    audioPipeManager->UpdateModemStreamDevice(deviceDescs);
}

void UpdateRingAndVoipStreamStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->ringAndVoipDescMap_.clear();

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_RINGING);

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_VOICE_RINGING);

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_PHONE_CHAT);

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_DEFAULT);
}

void UpdateRingAndVoipStreamDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->ringAndVoipDescMap_.clear();

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ringDeviceDescs;
    ringDeviceDescs.push_back(std::make_shared<AudioDeviceDescriptor>());
    ringDeviceDescs.front()->deviceType_ = DEVICE_TYPE_SPEAKER;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> voipDeviceDescs;
    voipDeviceDescs.push_back(std::make_shared<AudioDeviceDescriptor>());
    voipDeviceDescs.front()->deviceType_ = DEVICE_TYPE_EARPIECE;

    audioPipeManager->UpdateRingAndVoipStreamDevice(ringDeviceDescs, voipDeviceDescs);

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_PHONE_CHAT);

    audioPipeManager->UpdateRingAndVoipStreamDevice(ringDeviceDescs, voipDeviceDescs);
}

void GetStreamDescForAudioSceneFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->ringAndVoipDescMap_.clear();

    audioPipeManager->GetStreamDescForAudioScene(AUDIO_SCENE_RINGING);

    audioPipeManager->GetStreamDescForAudioScene(AUDIO_SCENE_VOICE_RINGING);

    audioPipeManager->GetStreamDescForAudioScene(AUDIO_SCENE_PHONE_CHAT);

    audioPipeManager->GetStreamDescForAudioScene(AUDIO_SCENE_DEFAULT);

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_RINGING);

    audioPipeManager->GetStreamDescForAudioScene(AUDIO_SCENE_RINGING);

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_PHONE_CHAT);

    audioPipeManager->GetStreamDescForAudioScene(AUDIO_SCENE_PHONE_CHAT);
}

void GetRingAndVoipDescMapFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->ringAndVoipDescMap_.clear();

    audioPipeManager->GetRingAndVoipDescMap();

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->ringAndVoipDescMap_[sessionId] = nullptr;
    audioPipeManager->GetRingAndVoipDescMap();

    audioPipeManager->UpdateRingAndVoipStreamStatus(AUDIO_SCENE_PHONE_CHAT);
    audioPipeManager->GetRingAndVoipDescMap();
}

void IsModemStreamDeviceChangedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->modemCommunicationIdMap_.clear();

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;

    audioPipeManager->IsModemStreamDeviceChanged(deviceDesc);

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioPipeManager->modemCommunicationIdMap_[sessionId] = nullptr;
    audioPipeManager->IsModemStreamDeviceChanged(deviceDesc);

    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_[sessionId] = streamDesc1;
    audioPipeManager->IsModemStreamDeviceChanged(deviceDesc);

    streamDesc1->oldDeviceDescs_.push_back(nullptr);
    audioPipeManager->IsModemStreamDeviceChanged(deviceDesc);

    std::shared_ptr<AudioDeviceDescriptor> oldDevice = std::make_shared<AudioDeviceDescriptor>();
    oldDevice->deviceType_ = DEVICE_TYPE_EARPIECE;
    streamDesc1->oldDeviceDescs_.clear();
    streamDesc1->oldDeviceDescs_.push_back(oldDevice);
    audioPipeManager->IsModemStreamDeviceChanged(deviceDesc);

    audioPipeManager->IsModemStreamDeviceChanged(oldDevice);
}

void IsStreamUsageActiveFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    StreamUsage testUsage = STREAM_USAGE_MEDIA;

    audioPipeManager->IsStreamUsageActive(testUsage);

    std::shared_ptr<AudioPipeInfo> outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    audioPipeManager->AddAudioPipeInfo(outputPipe);
    audioPipeManager->IsStreamUsageActive(testUsage);

    outputPipe->streamDescriptors_.push_back(nullptr);
    audioPipeManager->IsStreamUsageActive(testUsage);

    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    desc1->rendererInfo_.streamUsage = STREAM_USAGE_NOTIFICATION;
    desc1->streamStatus_ = STREAM_STATUS_STARTED;
    outputPipe->streamDescriptors_.push_back(desc1);
    audioPipeManager->IsStreamUsageActive(testUsage);

    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    desc2->rendererInfo_.streamUsage = testUsage;
    desc2->streamStatus_ = STREAM_STATUS_PAUSED;
    outputPipe->streamDescriptors_.push_back(desc2);
    audioPipeManager->IsStreamUsageActive(testUsage);

    std::shared_ptr<AudioStreamDescriptor> desc3 = std::make_shared<AudioStreamDescriptor>();
    desc3->rendererInfo_.streamUsage = testUsage;
    desc3->streamStatus_ = STREAM_STATUS_STARTED;
    outputPipe->streamDescriptors_.push_back(desc3);
    audioPipeManager->IsStreamUsageActive(testUsage);
}

void IsCaptureVoipCallFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    audioPipeManager->IsCaptureVoipCall();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->paIndex_ = 1;
    pipeInfo->streamDescriptors_.push_back(nullptr);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    audioPipeManager->IsCaptureVoipCall();

    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    desc1->streamStatus_ = STREAM_STATUS_STOPPED;
    pipeInfo->streamDescriptors_.push_back(desc1);
    audioPipeManager->IsCaptureVoipCall();

    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    desc2->streamStatus_ = STREAM_STATUS_STARTED;
    desc2->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc2->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipeInfo->streamDescriptors_.push_back(desc2);
    audioPipeManager->IsCaptureVoipCall();

    std::shared_ptr<AudioStreamDescriptor> desc3 = std::make_shared<AudioStreamDescriptor>();
    desc3->streamStatus_ = STREAM_STATUS_STARTED;
    desc3->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    pipeInfo->streamDescriptors_.push_back(desc3);
    audioPipeManager->IsCaptureVoipCall();

    std::shared_ptr<AudioStreamDescriptor> desc4 = std::make_shared<AudioStreamDescriptor>();
    desc4->streamStatus_ = STREAM_STATUS_STARTED;
    desc4->routeFlag_ = AUDIO_INPUT_FLAG_VOIP;
    pipeInfo->streamDescriptors_.push_back(desc4);
    audioPipeManager->IsCaptureVoipCall();
}

void GetPaIndexByNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::string portName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);

    audioPipeManager->GetPaIndexByName(portName);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->paIndex_ = NUM_100;
    pipeInfo->name_ = portName;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    audioPipeManager->GetPaIndexByName(portName);

    audioPipeManager->GetPaIndexByName(portName);

    audioPipeManager->GetPaIndexByName(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
}

void IsOnPrimaryAdapterFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();

    audioPipeManager->IsOnPrimaryAdapter(sessionId);

    std::shared_ptr<AudioPipeInfo> pipeInfo1 = std::make_shared<AudioPipeInfo>();
    pipeInfo1->adapterName_ = USB_CLASS;
    pipeInfo1->streamDescMap_[sessionId] = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->AddAudioPipeInfo(pipeInfo1);
    audioPipeManager->IsOnPrimaryAdapter(sessionId);

    std::shared_ptr<AudioPipeInfo> pipeInfo2 = std::make_shared<AudioPipeInfo>();
    pipeInfo2->adapterName_ = ADAPTER_TYPE_PRIMARY;
    audioPipeManager->AddAudioPipeInfo(pipeInfo2);
    audioPipeManager->IsOnPrimaryAdapter(sessionId);

    std::shared_ptr<AudioPipeInfo> pipeInfo3 = std::make_shared<AudioPipeInfo>();
    pipeInfo3->adapterName_ = ADAPTER_TYPE_PRIMARY;
    pipeInfo3->streamDescMap_[sessionId] = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->AddAudioPipeInfo(pipeInfo3);
    audioPipeManager->IsOnPrimaryAdapter(sessionId);
}

void HasRunningStreamFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    audioPipeManager->HasRunningStream();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    audioPipeManager->HasRunningStream();

    pipeInfo->streamDescriptors_.push_back(nullptr);
    audioPipeManager->HasRunningStream();

    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    desc1->streamStatus_ = STREAM_STATUS_STOPPED;
    pipeInfo->streamDescriptors_.push_back(desc1);
    audioPipeManager->HasRunningStream();

    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    desc2->streamStatus_ = STREAM_STATUS_STARTED;
    pipeInfo->streamDescriptors_.push_back(desc2);
    audioPipeManager->HasRunningStream();
}

void HasFastOutputPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    audioPipeManager->HasFastOutputPipe();
    std::shared_ptr<AudioPipeInfo> pipeInfo1 = std::make_shared<AudioPipeInfo>();
    pipeInfo1->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    audioPipeManager->AddAudioPipeInfo(pipeInfo1);
    audioPipeManager->HasFastOutputPipe();

    std::shared_ptr<AudioPipeInfo> pipeInfo2 = std::make_shared<AudioPipeInfo>();
    pipeInfo2->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    audioPipeManager->AddAudioPipeInfo(pipeInfo2);
    audioPipeManager->HasFastOutputPipe();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    RemoveAudioPipeInfoFuzzTest,
    RemoveAudioPipeInfoByIdFuzzTest,
    UpdateAudioPipeInfoFuzzTest,
    IsSamePipeFuzzTest,
    GetUnusedPipeFuzzTest,
    IsSpecialPipeFuzzTest,
    IsNormalRecordPipeFuzzTest,
    GetStreamDescByIdFuzzTest,
    DumpFuzzTest,
    IsModemCommunicationIdExistFuzzTest,
    GetModemCommunicationStreamDescByIdFuzzTest,
    GetPipeinfoByNameAndFlagFuzzTest,
    GetModuleNameBySessionIdFuzzTest,
    GetProcessDeviceInfoBySessionIdFuzzTest,
    GetAllOutputStreamDescsFuzzTest,
    GetAllInputStreamDescsFuzzTest,
    GetStreamDescByIdInnerFuzzTest,
    GetStreamCountFuzzTest,
    GetPaIndexByIoHandleFuzzTest,
    UpdateRendererPipeInfosFuzzTest,
    UpdateCapturerPipeInfosFuzzTest,
    PcmOffloadSessionCountFuzzTest,
    AddModemCommunicationIdFuzzTest,
    RemoveModemCommunicationIdFuzzTest,
    GetNormalSourceInfoFuzzTest,
    GetPipeByModuleAndFlagFuzzTest,
    StartClientFuzzTest,
    GetUnusedRecordPipeFuzzTest,
    GetAdapterNameBySessionIdFuzzTest,
    GetClientUidBySessionIdFuzzTest,
    GetModemCommunicationStreamDescFuzzTest,
    UpdateModemStreamStatusFuzzTest,
    UpdateModemStreamDeviceFuzzTest,
    UpdateRingAndVoipStreamStatusFuzzTest,
    UpdateRingAndVoipStreamDeviceFuzzTest,
    GetStreamDescForAudioSceneFuzzTest,
    GetRingAndVoipDescMapFuzzTest,
    IsModemStreamDeviceChangedFuzzTest,
    IsStreamUsageActiveFuzzTest,
    IsCaptureVoipCallFuzzTest,
    GetPaIndexByNameFuzzTest,
    IsOnPrimaryAdapterFuzzTest,
    HasRunningStreamFuzzTest,
    HasFastOutputPipeFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
