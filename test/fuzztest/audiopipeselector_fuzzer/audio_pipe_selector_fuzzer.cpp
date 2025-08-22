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
const int32_t NUM_1 = 1;
const uint32_t IDNUM = 100;
constexpr int32_t AUDIO_MODE_COUNT = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + NUM_1;
constexpr int32_t AUDIO_FLAG_COUNT = static_cast<int32_t>(AudioFlag::AUDIO_FLAG_MAX) + NUM_1;

typedef void (*TestFuncs)();

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

void GetPipeTypeFuzzTest()
{
    AudioMode selectedAudioMode = static_cast<AudioMode>(GetData<int32_t>() % AUDIO_MODE_COUNT);
    uint32_t selectedFlag = static_cast<uint32_t>(GetData<int32_t>() % AUDIO_FLAG_COUNT);
    AudioPipeSelector::GetPipeSelector()->GetPipeType(selectedFlag, selectedAudioMode);
}

void GetAdapterNameByStreamDescFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::string result = audioPipeSelector->GetAdapterNameByStreamDesc(streamDesc);
}

void ConvertStreamDescToPipeInfoFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = 1;
    streamDesc->sessionId_ = GetData<uint32_t>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->capturerInfo_.sourceType = SourceType::SOURCE_TYPE_MIC;

    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = GetData<uint32_t>();
    streamPropInfo->channelLayout_ = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamPropInfo->bufferSize_ = GetData<uint32_t>();

    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = std::make_shared<AdapterPipeInfo>();
    pipeInfoPtr->paProp_.lib_ = "test_lib";
    pipeInfoPtr->paProp_.role_ = "test_role";
    pipeInfoPtr->paProp_.moduleName_ = "test_module";
    pipeInfoPtr->name_ = "test_name";
    pipeInfoPtr->role_ = PIPE_ROLE_OUTPUT;

    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = std::make_shared<PolicyAdapterInfo>();
    adapterInfoPtr->adapterName = "test_adapter";

    pipeInfoPtr->adapterInfo_ = adapterInfoPtr;
    streamPropInfo->pipeInfo_ = pipeInfoPtr;

    AudioPipeInfo info;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
}

void JudgeStreamActionFuzzTest()
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    uint8_t newRandomNum = GetData<uint8_t>();
    std::vector<std::string> testStrings = {"test_adapter", "new_adapter", "old_adapter"};
    std::string newAdapterName(testStrings[newRandomNum % testStrings.size()]);
    newPipe->adapterName_ = newAdapterName;
    AudioFlag newAudioFlag = static_cast<AudioFlag>(GetData<uint8_t>() % AUDIO_FLAG_COUNT);
    newPipe->routeFlag_ = newAudioFlag;
    std::vector<std::string> className = {
        "remote_offload",
        "name"
    };
    size_t idx = GetData<size_t>() % className.size();
    newPipe->moduleInfo_.className = className[idx];

    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    uint8_t oldRandomNum = GetData<uint8_t>();
    std::string oldAdapterName(testStrings[oldRandomNum % testStrings.size()]);
    oldPipe->adapterName_ = oldAdapterName;
    AudioFlag oldAudioFlag = static_cast<AudioFlag>(GetData<uint8_t>() % AUDIO_FLAG_COUNT);
    oldPipe->routeFlag_ = oldAudioFlag;
    idx = GetData<size_t>() % className.size();
    oldPipe->moduleInfo_.className = className[idx];

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
}

void FetchPipeAndExecuteFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = AUDIO_FLAG_NONE;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->adapterName_ = "test_adapter";
    pipeInfo->routeFlag_ = 1;
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> result = audioPipeSelector->FetchPipeAndExecute(streamDesc);
}

void FetchPipesAndExecuteFuzzTest()
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->FetchPipesAndExecute(streamDescs);
}

void ProcessConcurrencyFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> existingStream = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(existingStream != nullptr);
    std::shared_ptr<AudioStreamDescriptor> incomingStream = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(incomingStream != nullptr);
    existingStream->routeFlag_ = GetData<uint32_t>();
    existingStream->audioMode_ = GetData<AudioMode>();
    incomingStream->routeFlag_ = GetData<uint32_t>();
    incomingStream->audioMode_ = GetData<AudioMode>();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    CHECK_AND_RETURN(audioPipeSelector != nullptr);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove;
    audioPipeSelector->ProcessConcurrency(existingStream, incomingStream, streamsToMove);
}

TestFuncs g_testFuncs[] = {
    GetPipeTypeFuzzTest,
    GetAdapterNameByStreamDescFuzzTest,
    ConvertStreamDescToPipeInfoFuzzTest,
    JudgeStreamActionFuzzTest,
    FetchPipeAndExecuteFuzzTest,
    FetchPipesAndExecuteFuzzTest,
    ProcessConcurrencyFuzzTest,
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
