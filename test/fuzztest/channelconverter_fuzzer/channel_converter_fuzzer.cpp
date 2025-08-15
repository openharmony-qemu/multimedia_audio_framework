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
#include "channel_converter.h"
#include "audio_stream_info.h"
#include "audio_log.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::AudioStandard::HPAE;
using namespace std;
using namespace HPAE;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t THRESHOLD = 10;

void SetParamFuzzTest()
{
    std::shared_ptr<ChannelConverter> channelConverter = std::make_shared<ChannelConverter>();
    CHECK_AND_RETURN(channelConverter != nullptr);
    AudioChannelInfo inChannelInfo = {
        .channelLayout = g_fuzzUtils.GetData<AudioChannelLayout>(),
        .numChannels = g_fuzzUtils.GetData<uint32_t>(),
    };
    AudioChannelInfo outChannelInfo = {
        .channelLayout = g_fuzzUtils.GetData<AudioChannelLayout>(),
        .numChannels = g_fuzzUtils.GetData<uint32_t>(),
    };
    AudioSampleFormat format = g_fuzzUtils.GetData<AudioSampleFormat>();
    bool mixLfe = g_fuzzUtils.GetData<bool>();

    channelConverter->SetParam(inChannelInfo, outChannelInfo, format, mixLfe);
}

void SetInChannelInfoFuzzTest()
{
    std::shared_ptr<ChannelConverter> channelConverter = std::make_shared<ChannelConverter>();
    CHECK_AND_RETURN(channelConverter != nullptr);
    AudioChannelInfo inChannelInfo = {
        .channelLayout = g_fuzzUtils.GetData<AudioChannelLayout>(),
        .numChannels = g_fuzzUtils.GetData<uint32_t>(),
    };
    channelConverter->SetInChannelInfo(inChannelInfo);
}

void SetOutChannelInfoFuzzTest()
{
    std::shared_ptr<ChannelConverter> channelConverter = std::make_shared<ChannelConverter>();
    CHECK_AND_RETURN(channelConverter != nullptr);
    AudioChannelInfo outChannelInfo = {
        .channelLayout = g_fuzzUtils.GetData<AudioChannelLayout>(),
        .numChannels = g_fuzzUtils.GetData<uint32_t>(),
    };
    channelConverter->SetOutChannelInfo(outChannelInfo);
}

void SetParamsecondFuzzTest()
{
    std::shared_ptr<ChannelConverter> channelConverter = std::make_shared<ChannelConverter>();
    CHECK_AND_RETURN(channelConverter != nullptr);
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = g_fuzzUtils.GetData<int32_t>();
    outChannelInfo.numChannels = g_fuzzUtils.GetData<int32_t>();
    AudioSampleFormat workFormat = g_fuzzUtils.GetData<AudioSampleFormat>();
    bool mixLfe = g_fuzzUtils.GetData<bool>();
    channelConverter->SetParam(inChannelInfo, outChannelInfo, workFormat, mixLfe);
}

vector <TestFuncs> g_testFuncs = {
    SetParamFuzzTest,
    SetInChannelInfoFuzzTest,
    SetOutChannelInfoFuzzTest,
    SetParamsecondFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
