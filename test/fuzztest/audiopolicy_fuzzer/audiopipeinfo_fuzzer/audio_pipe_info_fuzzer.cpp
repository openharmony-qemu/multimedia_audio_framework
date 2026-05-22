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
#include <fuzzer/FuzzedDataProvider.h>
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
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_server.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const uint32_t MAX_SESSION_ID = 100000;

typedef void (*TestFuncs)(FuzzedDataProvider& fdp);

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

const std::vector<AudioPipeRole> PIPE_ROLES = {
    PIPE_ROLE_OUTPUT,
    PIPE_ROLE_INPUT,
    PIPE_ROLE_NONE,
};

const std::vector<AudioPipeAction> PIPE_ACTIONS = {
    PIPE_ACTION_DEFAULT,
    PIPE_ACTION_NEW,
    PIPE_ACTION_UPDATE,
    PIPE_ACTION_RELOAD,
};

const std::vector<uint32_t> ROUTE_FLAGS = {
    AUDIO_FLAG_NONE,
    AUDIO_OUTPUT_FLAG_NORMAL,
    AUDIO_OUTPUT_FLAG_FAST,
    AUDIO_OUTPUT_FLAG_LOWPOWER,
    AUDIO_OUTPUT_FLAG_DIRECT,
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
    AUDIO_INPUT_FLAG_WAKEUP,
};

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

std::shared_ptr<AudioStreamDescriptor> MakeStreamDescriptor(FuzzedDataProvider& fdp, AudioPipeRole role)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return nullptr;
    }
    streamDesc->sessionId_ = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    streamDesc->audioMode_ = (role == PIPE_ROLE_INPUT) ? AUDIO_MODE_RECORD : AUDIO_MODE_PLAYBACK;
    streamDesc->routeFlag_ = PickValue(fdp, ROUTE_FLAGS);
    streamDesc->callerUid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->callerPid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->bundleName_ = ConsumeString(fdp, "audio_pipe_info_fuzzer");
    return streamDesc;
}

std::shared_ptr<AudioPipeInfo> MakePipeInfo(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (pipeInfo == nullptr) {
        return nullptr;
    }
    pipeInfo->id_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->paIndex_ = fdp.ConsumeIntegral<uint32_t>();
    pipeInfo->pipeRole_ = PickValue(fdp, PIPE_ROLES);
    pipeInfo->name_ = ConsumeString(fdp, "audio_pipe");
    pipeInfo->adapterName_ = ConsumeString(fdp, "primary");
    pipeInfo->routeFlag_ = PickValue(fdp, ROUTE_FLAGS);
    pipeInfo->pipeAction_ = PickValue(fdp, PIPE_ACTIONS);
    pipeInfo->softLinkFlag_ = fdp.ConsumeBool();
    pipeInfo->isRunning_ = fdp.ConsumeBool();
    pipeInfo->moduleInfo_.rate = ConsumeString(fdp, "48000");
    pipeInfo->moduleInfo_.channels = ConsumeString(fdp, "2");
    pipeInfo->moduleInfo_.format = ConsumeString(fdp, "s16le");
    pipeInfo->moduleInfo_.bufferSize = ConsumeString(fdp, "4096");
    pipeInfo->moduleInfo_.renderInIdleState = ConsumeString(fdp, "0");
    pipeInfo->moduleInfo_.sourceType = ConsumeString(fdp, "mic");
    pipeInfo->SetUltraFastFlag(fdp.ConsumeBool());
    if (fdp.ConsumeBool()) {
        auto streamDesc = MakeStreamDescriptor(fdp, pipeInfo->pipeRole_);
        if (streamDesc != nullptr) {
            pipeInfo->AddStream(streamDesc);
        }
    }
    return pipeInfo;
}

void DumpFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    if (pipeInfo == nullptr) {
        return;
    }
    AudioPipeInfo audioPipeInfo(pipeInfo);
    std::string dumpString = ConsumeString(fdp, "");
    audioPipeInfo.Dump(dumpString);
}

void DumpCommonAttrsFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::string dumpString = ConsumeString(fdp, "");
    pipeInfo->DumpCommonAttrs(dumpString);
}

void DumpOutputAttrsFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::string dumpString = ConsumeString(fdp, "");
    pipeInfo->DumpOutputAttrs(dumpString);
}

void DumpInputAttrsFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::string dumpString = ConsumeString(fdp, "");
    pipeInfo->DumpInputAttrs(dumpString);
}

void AudioPipeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr);
    AudioPipeInfo audioPipeInfo(pipeInfo);
    audioPipeInfo.InitAudioStreamInfo();
    audioPipeInfo.HasStream();
}

void ToStringFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr);
    pipeInfo->ToString();
    pipeInfo->IsRouteNormal();
    pipeInfo->IsRouteFast();
    pipeInfo->IsRouteOffload();
    pipeInfo->IsRouteDirect();
    pipeInfo->IsRenderPipeNeedMoveToNormal();
    pipeInfo->IsSameAdapter(ConsumeString(fdp, "primary"));
    pipeInfo->IsSameName(ConsumeString(fdp, "audio_pipe"));
    pipeInfo->IsSameNetworkId(ConsumeString(fdp, LOCAL_NETWORK_ID));
    pipeInfo->IsPipeMatch(ConsumeString(fdp, "primary"), PickValue(fdp, ROUTE_FLAGS));
}

void ContainStreamFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr);
    uint32_t sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    pipeInfo->ContainStream(sessionId);
}

void AddStreamFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = MakePipeInfo(fdp);
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::shared_ptr<AudioStreamDescriptor> stream = MakeStreamDescriptor(fdp, pipeInfo->pipeRole_);
    CHECK_AND_RETURN(stream != nullptr);
    pipeInfo->AddStream(stream);
    uint32_t sessionId = fdp.ConsumeBool() ? stream->GetSessionId() :
        fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    pipeInfo->RemoveStream(sessionId);
}

TestFuncs g_testFuncs[] = {
    DumpFuzzTest,
    DumpCommonAttrsFuzzTest,
    DumpOutputAttrsFuzzTest,
    DumpInputAttrsFuzzTest,
    AudioPipeInfoFuzzTest,
    ToStringFuzzTest,
    ContainStreamFuzzTest,
    AddStreamFuzzTest,
};

void FuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[fdp.ConsumeIntegral<uint32_t>() % len](fdp);
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}
