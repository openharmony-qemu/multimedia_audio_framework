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
#include "audio_info.h"
#include "simd_utils.h"
#include "hpae_pcm_buffer.h"
#include "audio_log.h"
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

namespace OHOS {
namespace AudioStandard {
using namespace std;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const size_t TESTSIZE = 1024;
const uint32_t TESTFRAME = 1;
typedef void (*TestPtr)(const uint8_t *, size_t);

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

void GetPcmBufferInfoFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetPcmBufferInfo();
}

void GetChannelCountFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetChannelCount();
}

void GetFrameLenFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetFrameLen();
}

void GetSampleRateFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetSampleRate();
}

void IsValidFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->IsValid();
}

void IsSilenceFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->IsSilence();
}

void GetBufferStateFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetBufferState();
}

void GetChannelLayoutFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetChannelLayout();
}

void ReConfigFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->ReConfig(pcmBufferInfo);
}

void GetFrameDataFuzzTest1()
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.isMultiFrames = GetData<bool>();
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    HpaePcmBuffer frameData = HpaePcmBuffer(pcmBufferInfo);
    hpaePcmBuffer->GetFrameData(frameData);
}

void GetFrameDataFuzzTest2()
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.isMultiFrames = GetData<bool>();
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    std::vector<float> frameData(TESTSIZE, 0.0f);
    hpaePcmBuffer->GetFrameData(frameData);
}

void PushFrameDataFuzzTest1()
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.isMultiFrames = GetData<bool>();
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    HpaePcmBuffer frameData = HpaePcmBuffer(pcmBufferInfo);
    hpaePcmBuffer->PushFrameData(frameData);
}

void PushFrameDataFuzzTest2()
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.isMultiFrames = GetData<bool>();
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    std::vector<float> frameData(TESTSIZE, 0.0f);
    hpaePcmBuffer->PushFrameData(frameData);
}

void StoreFrameDataFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    HpaePcmBuffer frameData = HpaePcmBuffer(pcmBufferInfo);
    hpaePcmBuffer->StoreFrameData(frameData);
}

void SizeFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->Size();
}

void DataSizeFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->DataSize();
}

void GetFramesFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetFrames();
}

void GetReadPosFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetReadPos();
}

void GetWritePosFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetWritePos();
}

void UpdateReadPosFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    size_t readPos = hpaePcmBuffer -> GetReadPos();
    hpaePcmBuffer->UpdateReadPos(readPos);
}

void UpdateWritePosFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    size_t readPos = hpaePcmBuffer -> GetReadPos();
    hpaePcmBuffer->UpdateWritePos(readPos);
}

void SetBufferValidFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    bool vaild = GetData<bool>();
    hpaePcmBuffer->SetBufferValid(vaild);
}

void SetBufferSilenceFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    bool silence = GetData<bool>();
    hpaePcmBuffer->SetBufferSilence(silence);
}

void SetBufferStateFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    uint32_t state = hpaePcmBuffer -> GetBufferState();
    hpaePcmBuffer->SetBufferState(state);
}

void GetCurFramesFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetCurFrames();
}

void GetPcmDataBufferFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetPcmDataBuffer();
}

void GetFrameSampleFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetFrameSample();
}

void GetSourceBufferTypeFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetSourceBufferType();
}

void SetSourceBufferTypeFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    HpaeSourceBufferType type = hpaePcmBuffer->GetSourceBufferType();
    hpaePcmBuffer->SetSourceBufferType(type);
}

void GetSplitStreamTypeFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    hpaePcmBuffer->GetSplitStreamType();
}

void SetSplitStreamTypeFuzzTest()
{
    PcmBufferInfo pcmBufferInfo;
    auto hpaePcmBuffer = std::make_shared<HpaePcmBuffer>(pcmBufferInfo);
    HpaeSplitStreamType type = hpaePcmBuffer->GetSplitStreamType();
    hpaePcmBuffer->SetSplitStreamType(type);
}

void HpaePcmBufferOperator()
{
    PcmBufferInfo pcmBufferInfo;
    pcmBufferInfo.frames = TESTFRAME;
    pcmBufferInfo.isMultiFrames = GetData<bool>();
    HpaePcmBuffer srcHpaePcmBuffer(pcmBufferInfo);
    HpaePcmBuffer dstHpaePcmBuffer(pcmBufferInfo);
    std::vector<float> vec(TESTFRAME);
    std::vector<std::vector<float>> dstVec(TESTFRAME);
    srcHpaePcmBuffer = dstVec;
    dstHpaePcmBuffer = vec;
    srcHpaePcmBuffer = dstHpaePcmBuffer;
    srcHpaePcmBuffer += dstHpaePcmBuffer;
    srcHpaePcmBuffer -= dstHpaePcmBuffer;
    srcHpaePcmBuffer *= dstHpaePcmBuffer;
}

typedef void (*TestFuncs[32])();

TestFuncs g_testFuncs = {
    GetPcmBufferInfoFuzzTest,
    GetChannelCountFuzzTest,
    GetFrameLenFuzzTest,
    GetSampleRateFuzzTest,
    IsValidFuzzTest,
    IsSilenceFuzzTest,
    GetBufferStateFuzzTest,
    GetChannelLayoutFuzzTest,
    ReConfigFuzzTest,
    GetFrameDataFuzzTest1,
    GetFrameDataFuzzTest2,
    PushFrameDataFuzzTest1,
    PushFrameDataFuzzTest2,
    StoreFrameDataFuzzTest,
    SizeFuzzTest,
    DataSizeFuzzTest,
    GetFramesFuzzTest,
    GetReadPosFuzzTest,
    GetWritePosFuzzTest,
    UpdateReadPosFuzzTest,
    UpdateWritePosFuzzTest,
    SetBufferValidFuzzTest,
    SetBufferSilenceFuzzTest,
    SetBufferStateFuzzTest,
    GetCurFramesFuzzTest,
    GetPcmDataBufferFuzzTest,
    GetFrameSampleFuzzTest,
    GetSourceBufferTypeFuzzTest,
    SetSourceBufferTypeFuzzTest,
    GetSplitStreamTypeFuzzTest,
    SetSplitStreamTypeFuzzTest,
    HpaePcmBufferOperator,
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
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
