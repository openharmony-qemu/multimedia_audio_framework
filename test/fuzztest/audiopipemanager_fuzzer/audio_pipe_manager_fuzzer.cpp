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
#include <cstring>
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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 26;
const uint32_t RESIZENUM = 2;
const uint32_t IDNUM = 2;
const uint32_t NUM_2 = 2;
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
    AUDIO_FLAG_MAX,
};

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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void RemoveAudioPipeInfoFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> targetPipe = std::make_shared<AudioPipeInfo>();
    targetPipe->adapterName_ = "test_adapter";
    targetPipe->routeFlag_ = 1;

    audioPipeManager->AddAudioPipeInfo(targetPipe);
    audioPipeManager->RemoveAudioPipeInfo(targetPipe);
    auto pipeList = audioPipeManager->GetPipeList();
}

void RemoveAudioPipeInfoByIdFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> targetPipe = std::make_shared<AudioPipeInfo>();
    targetPipe->id_ = GetData<uint32_t>();
    targetPipe->adapterName_ = "test_adapter";

    audioPipeManager->GetPipeList();
    audioPipeManager->AddAudioPipeInfo(targetPipe);
    audioPipeManager->RemoveAudioPipeInfo(targetPipe->id_);
    audioPipeManager->GetPipeList();
}

void UpdateAudioPipeInfoFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    audioPipeManager->GetPipeList();

    std::shared_ptr<AudioPipeInfo> existingPipe = std::make_shared<AudioPipeInfo>();
    existingPipe->adapterName_ = "existing_adapter";
    existingPipe->routeFlag_ = 1;
    existingPipe->id_ = IDNUM;
    audioPipeManager->AddAudioPipeInfo(existingPipe);

    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "existing_adapter";
    newPipe->routeFlag_ = 1;
    newPipe->id_ = GetData<uint32_t>();
    audioPipeManager->UpdateAudioPipeInfo(newPipe);
    audioPipeManager->GetPipeList();
}

void IsSamePipeFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    info->adapterName_ = "test_adapter";
    info->routeFlag_ = 1;
    info->id_ = GetData<uint32_t>();

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = "test_adapter";
    cmpInfo->routeFlag_ = 1;
    cmpInfo->id_ = GetData<uint32_t>();
    audioPipeManager->IsSamePipe(info, cmpInfo);
}

void GetUnusedPipeFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);

    audioPipeManager->GetUnusedPipe();
}

void IsSpecialPipeFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (AudioFlagVec.size() == 0) {
        return;
    }
    uint32_t routeFlagCount = GetData<uint32_t>() % AudioFlagVec.size();
    uint32_t routeFlag = AudioFlagVec[routeFlagCount];
    audioPipeManager->IsSpecialPipe(routeFlag);
}

void IsNormalRecordPipeFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (AudioFlagVec.size() == 0 || audioPipeManager == nullptr) {
        return;
    }
    shared_ptr<AudioPipeInfo> pipe = std::make_shared<AudioPipeInfo>();
    pipe->adapterName_ = GetData<uint32_t>() % NUM_2 == 0 ? PRIMARY_CLASS : USB_CLASS;
    uint32_t routeFlagCount = GetData<uint32_t>() % AudioFlagVec.size();
    pipe->routeFlag_ == AudioFlagVec[routeFlagCount];
    audioPipeManager->IsNormalRecordPipe(pipe);
}

void GetStreamDescByIdFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = GetData<uint32_t>();
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = GetData<uint32_t>();
    audioPipeManager->GetStreamDescById(targetSessionId);
}

void DumpFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    std::string dumpString = "";
    audioPipeManager->Dump(dumpString);
}

void IsModemCommunicationIdExistFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    audioPipeManager->IsModemCommunicationIdExist(sessionId);
}

void GetModemCommunicationStreamDescByIdFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    if (audioPipeManager == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    audioPipeManager->modemCommunicationIdMap_.clear();
    audioPipeManager->GetModemCommunicationStreamDescById(sessionId);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();
    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);
    audioPipeManager->GetModemCommunicationStreamDescById(sessionId);
}

void GetPipeinfoByNameAndFlagFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "existing_adapter";
    pipe1->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipe1);

    std::string targetAdapterName = "existing_adapter";
    uint32_t targetRouteFlag = 1;
    audioPipeManager->GetPipeinfoByNameAndFlag(targetAdapterName, targetRouteFlag);
}

void GetAdapterNameBySessionIdFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "TestAdapter";
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = GetData<uint32_t>();
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    uint32_t targetSessionId = GetData<uint32_t>();

    audioPipeManager->GetAdapterNameBySessionId(targetSessionId);
}

void GetProcessDeviceInfoBySessionIdFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = GetData<uint32_t>();
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = GetData<uint32_t>();
    AudioStreamInfo info;
    audioPipeManager->GetProcessDeviceInfoBySessionId(targetSessionId, info);
}

void GetAllOutputStreamDescsFuzzTest()
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

void GetAllInputStreamDescsFuzzTest()
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

void GetStreamDescByIdInnerFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = GetData<uint32_t>();
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = GetData<uint32_t>();
    audioPipeManager->GetStreamDescByIdInner(targetSessionId);
}

void GetStreamCountFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "existing_adapter";
    pipeInfo->routeFlag_ = 1;
    pipeInfo->streamDescriptors_.resize(RESIZENUM);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetAdapterName = "existing_adapter";
    uint32_t targetRouteFlag = 1;
    audioPipeManager->GetStreamCount(targetAdapterName, targetRouteFlag);
}

void GetPaIndexByIoHandleFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = GetData<uint32_t>();
    pipeInfo->paIndex_ = GetData<uint32_t>();
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    AudioIOHandle targetId = GetData<uint32_t>();
    audioPipeManager->GetPaIndexByIoHandle(targetId);
}

void UpdateRendererPipeInfosFuzzTest()
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

void UpdateCapturerPipeInfosFuzzTest()
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

void PcmOffloadSessionCountFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->streamDescriptors_.resize(RESIZENUM);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    audioPipeManager->PcmOffloadSessionCount();
}

void AddModemCommunicationIdFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 99999;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();
    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);

    audioPipeManager->GetModemCommunicationMap();
}

void RemoveModemCommunicationIdFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 12345;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    audioPipeManager->modemCommunicationIdMap_[sessionId] = streamDesc;
    audioPipeManager->RemoveModemCommunicationId(sessionId);
    audioPipeManager->GetModemCommunicationMap();
}

void GetNormalSourceInfoFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> bluetoothPipe = std::make_shared<AudioPipeInfo>();
    bluetoothPipe->moduleInfo_.name = PRIMARY_MIC;
    bluetoothPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    audioPipeManager->AddAudioPipeInfo(bluetoothPipe);

    bool isEcFeatureEnable = true;
    audioPipeManager->GetNormalSourceInfo(isEcFeatureEnable);
}

void GetPipeByModuleAndFlagFuzzTest()
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "EXISTING_MODULE";
    pipeInfo->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetModuleName = "NON_EXISTING_MODULE";
    uint32_t targetRouteFlag = 1;
    audioPipeManager->GetPipeByModuleAndFlag(targetModuleName, targetRouteFlag);
}

TestFuncs g_testFuncs[TESTSIZE] = {
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
    GetAdapterNameBySessionIdFuzzTest,
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
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
