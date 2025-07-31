/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <vector>
#include <memory>
#include <queue>
#include <string>
#undef private
#include "audio_info.h"
#include "i_stream.h"
#include "hpae_capturer_manager.h"
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

std::shared_ptr<HpaeCapturerManager> HpaeCapturerManager_ = nullptr

namespace OHOS {
namespace AudioStandard {
using namespace std;
const uint8_t TESTSIZE = 32;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)(const uint8_t *,size_t)

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

void CreateStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeStreamInfo streamInfo;
    HpaeCapturerManager_->CreateStream(streamInfo);
}

void DestroyStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeCapturerManager_->DestroyStream(sessionId);
}

void StartFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeCapturerManager_->Start(sessionId);
}

void PauseFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeCapturerManager_->Pause(sessionId);
}

void FlushFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeCapturerManager_->Flush(sessionId);
}

void DrainFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeCapturerManager_->Drain(sessionId);
}

void StopFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeCapturerManager_->Stop(sessionId);
}

void ReleaseFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeCapturerManager_->Release(sessionId);
}

void MoveStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    const std::string &sourceName = sourceInfo.sourceName;
    HpaeCapturerManager_->MoveStream(sessionId,sourceName);
}

void MoveAllStreamFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    const std::vector<uint32_t> &sessionIds = {1,2,3};
    const std::string &sourceName = sourceInfo.sourceName;
    HpaeCapturerManager_->MoveAllStream(sourceName,sessionIds);
}

void SetMuteFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    bool isMute = false;
    HpaeCapturerManager_->SetMute(isMute);
}

void ProcessFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->Process();
}

void HandleMsgFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->HandleMsg();
}

void InitFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->Init();
}

void DeInitFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->DeInit();
}

void IsInitFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->IsInit();
}

void IsRunningFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->IsRunning();
}

void IsMsgProcessingFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->IsMsgProcessing();
}

void DeactivateThreadFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->DeactivateThread();
}

void RegisterReadCallbackFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    const std::weak_ptr<ICapturerStreamCallback> &callback = std::weak_ptr<ICapturerStreamCallback>();
    HpaeCapturerManager_->RegisterReadCallback(sessionId,callback);
}

void GetSourceOutputInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    HpaeSourceOutputInfo sourceOutputInfo;
    HpaeCapturerManager_->GetSourceOutputInfo(sessionId,sourceOutputInfo);
}

void GetSourceInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->GetSourceInfo();
}

void GetAllSourceInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->GetAllSourceInfo();
}

void OnNodeStatusUpdateFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    IOperation operation = IOperation::OPERATION_INVALID;
    HpaeCapturerManager_->OnNodeStatusUpdate(sessionId,operation);
}

void OnNotifyQueueFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->OnNotifyQueue();
}

void OnRequestLatencyFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t sessionId = 0;
    uint64_t latency = 0;
    HpaeCapturerManager_->OnRequestLatency(sessionId,latency);
}

void AddNodeToSourceFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCaptureMoveInfo moveInfo;
    HpaeCapturerManager_->AddNodeToSource(moveInfo);
}

void AddAllNodesToSourceFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    std::vector<HpaeCaptureMoveInfo> moveInfoVector;
    std::vector<HpaeCaptureMoveInfo> &moveInfos = moveInfoVector;
    bool isConnect = false;
    HpaeCapturerManager_->AddAllNodesToSource(moveInfos,isConnect);
}

void GetTheadNameFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->GetTheadName();
}

void SetCaptureIdFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    uint32_t captureId = 3;
    HpaeCapturerManager_->SetCaptureId(captureId);
}

void ReloadCaptureManagerFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->ReloadCaptureManager(sourceInfo);
}

void GetDeviceHDFDumpInfoFuzzTest()
{
    HpaeSourceInfo sourceInfo;
    HpaeCapturerManager_ = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeCapturerManager_->GetDeviceHDFDumpInfo();
}

typedef void (*TestFuncs[32])();

TestFuncs g_testFuncs = {
    CreateStreamFuzzTest,
    DestroyStreamFuzzTest,
    StartFuzzTest,
    PauseFuzzTest,
    FlushFuzzTest,
    DrainFuzzTest,
    StopFuzzTest,
    ReleaseFuzzTest,
    MoveStreamFuzzTest,
    MoveAllStreamFuzzTest,
    SetMuteFuzzTest,
    ProcessFuzzTest,
    HandleMsgFuzzTest,
    InitFuzzTest,
    DeInitFuzzTest,
    IsInitFuzzTest,
    IsRunningFuzzTest,
    IsMsgProcessingFuzzTest,
    DeactivateThreadFuzzTest,
    RegisterReadCallbackFuzzTest,
    GetSourceOutputInfoFuzzTest,
    GetSourceInfoFuzzTest,
    GetAllSourceInfoFuzzTest,
    OnNodeStatusUpdateFuzzTest,
    OnNotifyQueueFuzzTest,
    OnRequestLatencyFuzzTest,
    AddNodeToSourceFuzzTest,
    AddAllNodesToSourceFuzzTest,
    GetTheadNameFuzzTest,
    ReloadCaptureManagerFuzzTest,
    GetDeviceHDFDumpInfoFuzzTest,
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