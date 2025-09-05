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
#include <vector>
#include <memory>
#include <queue>
#include <string>
#undef private
#include "hpae_msg_channel.h"
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_info.h"
#include "i_renderer_stream.h"
#include "linear_pos_time_model.h"
#include "hpae_sink_input_node.h"
#include "hpae_sink_output_node.h"
#include "audio_engine_log.h"
using namespace std;
using namespace OHOS::AudioStandard::HPAE;


namespace OHOS {
namespace AudioStandard {
using namespace std;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)(const uint8_t *, size_t);
static vector<HpaeSessionState> sessionStateMap = {
    HPAE_SESSION_INVALID,
    HPAE_SESSION_NEW,
    HPAE_SESSION_PREPARED,
    HPAE_SESSION_RUNNING,
    HPAE_SESSION_PAUSING,
    HPAE_SESSION_PAUSED,
    HPAE_SESSION_STOPPING,
    HPAE_SESSION_STOPPED,
    HPAE_SESSION_RELEASED,
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


void DoProcessFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->DoProcess();
}

void ResetFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->Reset();
}

void ResetAllFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->ResetAll();
}

void RegisterWriteCallbackFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    const std::weak_ptr<IStreamCallback> &callback = std::weak_ptr<IStreamCallback>();
    hpaeSinkInputNode->RegisterWriteCallback(callback);
}

void FlushFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->Flush();
}

void DrainFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->Drain();
}

void SetStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    uint32_t index = GetData<uint32_t>() % sessionStateMap.size();
    HpaeSessionState renderState = sessionStateMap[index];
    hpaeSinkInputNode->SetState(renderState);
}

void GetStateFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->GetState();
}

void RewindHistoryBufferFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    uint64_t rewindTime = GetData<uint64_t>();
    hpaeSinkInputNode->RewindHistoryBuffer(rewindTime);
}

void SetAppUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    int32_t appUid = GetData<int32_t>();
    hpaeSinkInputNode->SetAppUid(appUid);
}

void GetAppUidFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->GetAppUid();
}

void SetOffloadEnabledFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->SetOffloadEnabled(true);
}

void GetOffloadEnabledFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->GetOffloadEnabled();
}

void SetLoudnessGainFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    float loudnessGain = GetData<float>();
    hpaeSinkInputNode->SetLoudnessGain(loudnessGain);
}

void GetLoudnessGainFuzzTest()
{
    HpaeNodeInfo nodeInfo;
    auto hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeSinkInputNode->GetLoudnessGain();
}

typedef void (*TestFuncs[17])();

TestFuncs g_testFuncs = {
    DoProcessFuzzTest,
    ResetFuzzTest,
    ResetAllFuzzTest,
    RegisterWriteCallbackFuzzTest,
    FlushFuzzTest,
    DrainFuzzTest,
    SetStateFuzzTest,
    GetStateFuzzTest,
    RewindHistoryBufferFuzzTest,
    SetAppUidFuzzTest,
    GetAppUidFuzzTest,
    SetOffloadEnabledFuzzTest,
    GetOffloadEnabledFuzzTest,
    SetLoudnessGainFuzzTest,
    GetLoudnessGainFuzzTest,
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
