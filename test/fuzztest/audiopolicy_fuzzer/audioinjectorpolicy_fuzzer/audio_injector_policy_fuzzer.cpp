/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "audio_stream_descriptor.h"
#include "audio_module_info.h"
#include "audio_pipe_info.h"
#include "audio_injector_policy.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
static const uint8_t* RAW_DATA = nullptr;
static int32_t NUM_2 = 2;
static size_t g_dataSize = 0;
static size_t g_pos;
const int32_t MAX_VALUE_DATA1 = 10000;
const uint32_t MAX_VALUE_DATA2 = 100000;

typedef void (*TestFuncs)();

vector<VoipType> VoipTypeVec = {
    NO_VOIP,
    NORMAL_VOIP,
    FAST_VOIP,
};

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    if (memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize) != EOK) {
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

std::shared_ptr<AudioStreamDescriptor> MakeVoipStream(uint32_t sessionId,
    int32_t uid, bool running, bool recording, uint32_t routeFlag)
{
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = sessionId;
    stream->callerUid_ = uid;
    stream->audioMode_ = recording ? AUDIO_MODE_RECORD : AUDIO_MODE_PLAYBACK;
    stream->streamStatus_ = running ? STREAM_STATUS_STARTED : STREAM_STATUS_NEW;
    stream->routeFlag_ = routeFlag;
    stream->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    stream->streamAction_ = AUDIO_STREAM_ACTION_MOVE;
    return stream;
}

std::shared_ptr<AudioPipeInfo> MakeInputPipe(FuzzedDataProvider& fdp, uint32_t paIndex,
    uint32_t routeFlag, std::shared_ptr<AudioStreamDescriptor> stream)
{
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->paIndex_ = paIndex;
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = routeFlag;
    if (stream != nullptr) {
        pipe->streamDescriptors_.push_back(stream);
    }
    return pipe;
}

void ResetInjectorState()
{
    auto& injector = AudioInjectorPolicy::GetInstance();
    injector.rendererStreamMap_.clear();
    injector.rendererMuteStreamMap_.clear();
    injector.injectorStreamIds_.clear();
    injector.injectCaptureTargetUidMap_.clear();
    injector.injectCaptureTargetStreamIdMap_.clear();
    injector.injectorWorkMode_ = INJECTOR_MODE_NONE;
    injector.activeTargetUid_ = INVALID_UID;
    injector.activeTargetStreamId_ = INVALID_STREAM_ID;
    injector.isConnected_ = false;
    injector.voipType_ = NO_VOIP;
    injector.isNeedMuteRenderer_ = false;
}

void InitFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
}

void DeInitFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    AudioInjectorPolicy::GetInstance().DeInit();
}

void UpdateAudioInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioModuleInfo info;
    info.name = "test_module";
    info.lib = "libtest.so";
    info.format = "s16le";
    info.channels = "2";
    info.rate = "48000";
    AudioInjectorPolicy::GetInstance().UpdateAudioInfo(info);
}

void AddStreamDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t renderId = GetData<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    AudioInjectorPolicy::GetInstance().AddStreamDescriptor(renderId, desc);
}

void RemoveStreamDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t renderId = GetData<uint32_t>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    AudioInjectorPolicy::GetInstance().AddStreamDescriptor(renderId, desc);
    AudioInjectorPolicy::GetInstance().RemoveStreamDescriptor(renderId);
}

void IsContainStreamFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t renderId = GetData<uint32_t>();
    if (fdp.ConsumeBool()) {
        std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
        AudioInjectorPolicy::GetInstance().AddStreamDescriptor(renderId, desc);
    }
    bool result = AudioInjectorPolicy::GetInstance().IsContainStream(renderId);
}

void GetAdapterNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::string name = AudioInjectorPolicy::GetInstance().GetAdapterName();
}

void GetRendererStreamCountFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    int32_t count = AudioInjectorPolicy::GetInstance().GetRendererStreamCount();
}

void SetCapturePortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(idx);
}

void GetCapturePortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(idx);
    uint32_t result = AudioInjectorPolicy::GetInstance().GetCapturePortIdx();
}

void SetRendererPortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetRendererPortIdx(idx);
}

void GetRendererPortIdxFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t idx = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().SetRendererPortIdx(idx);
    uint32_t result = AudioInjectorPolicy::GetInstance().GetRendererPortIdx();
}

void GetAudioModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    AudioModuleInfo& info = AudioInjectorPolicy::GetInstance().GetAudioModuleInfo();
}

void GetIsConnectedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    bool result = AudioInjectorPolicy::GetInstance().GetIsConnected();
}

void SetVoipTypeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    uint32_t voipTypeCount = GetData<uint32_t>() % VoipTypeVec.size();
    VoipType type = VoipTypeVec[voipTypeCount];
    AudioInjectorPolicy::GetInstance().SetVoipType(type);
}

void PrepareLegacyRendererFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t renderId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    AudioInjectorPolicy::GetInstance().PrepareLegacyRenderer(renderId);
}

void PreparePreciseRendererFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t renderId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    int32_t targetUid = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1);
    uint32_t targetStreamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    if (fdp.ConsumeBool() && AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        auto stream = MakeVoipStream(targetStreamId, targetUid, true, true, AUDIO_INPUT_FLAG_NORMAL);
        auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.push_back(pipe);
    }
    AudioInjectorPolicy::GetInstance().PreparePreciseRenderer(renderId, targetUid, targetStreamId);
    if (AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.clear();
    }
}

void ClearRendererTargetFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t renderId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    AudioInjectorPolicy::GetInstance().ClearRendererTarget(renderId);
}

void IsPreciseRendererFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t renderId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().PreparePreciseRenderer(renderId,
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1),
            fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2));
    }
    bool result = AudioInjectorPolicy::GetInstance().IsPreciseRenderer(renderId);
}

void AddCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
        AudioInjectorPolicy::GetInstance().activeTargetStreamId_ =
            fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    } else {
        AudioInjectorPolicy::GetInstance().voipType_ = VoipTypeVec[GetData<uint32_t>() % VoipTypeVec.size()];
    }
    AudioInjectorPolicy::GetInstance().AddCaptureInjector();
}

void AddCaptureInjectorInnerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
        AudioInjectorPolicy::GetInstance().activeTargetStreamId_ =
            fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    } else {
        AudioInjectorPolicy::GetInstance().voipType_ = VoipTypeVec[GetData<uint32_t>() % VoipTypeVec.size()];
    }
    AudioInjectorPolicy::GetInstance().AddCaptureInjectorInner();
}

void AddCaptureInjectorByTargetStreamFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t renderId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    int32_t targetUid = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1);
    uint32_t targetStreamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    AudioInjectorPolicy::GetInstance().PreparePreciseRenderer(renderId, targetUid, targetStreamId);
    AudioInjectorPolicy::GetInstance().AddCaptureInjectorByTargetStream(renderId);
}

void AddCaptureInjectorByTargetStreamInnerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    uint32_t targetStreamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    AudioInjectorPolicy::GetInstance().AddCaptureInjectorByTargetStreamInner(targetStreamId);
}

void RemoveCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    } else if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().isConnected_ = true;
        AudioInjectorPolicy::GetInstance().voipType_ = VoipTypeVec[GetData<uint32_t>() % VoipTypeVec.size()];
    }
    bool noCapturer = GetData<uint32_t>() % NUM_2;
    AudioInjectorPolicy::GetInstance().RemoveCaptureInjector(noCapturer);
}

void RemoveCaptureInjectorInnerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    } else if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().isConnected_ = true;
        AudioInjectorPolicy::GetInstance().voipType_ = VoipTypeVec[GetData<uint32_t>() % VoipTypeVec.size()];
    }
    bool noCapturer = GetData<uint32_t>() % NUM_2;
    AudioInjectorPolicy::GetInstance().RemoveCaptureInjectorInner(noCapturer);
}

void RemoveCaptureInjectorByTargetStreamInnerFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().isConnected_ = true;
    }
    bool noCapturer = GetData<uint32_t>() % NUM_2;
    AudioInjectorPolicy::GetInstance().RemoveCaptureInjectorByTargetStreamInner(noCapturer);
}

void ReleaseCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
        AudioInjectorPolicy::GetInstance().activeTargetUid_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1);
        AudioInjectorPolicy::GetInstance().activeTargetStreamId_ =
            fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    } else if (AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        AudioInjectorPolicy::GetInstance().voipType_ = FAST_VOIP;
        uint32_t paIdx = fdp.ConsumeIntegral<uint32_t>();
        AudioInjectorPolicy::GetInstance().capturePortIdx_ = paIdx;
        auto stream = MakeVoipStream(fdp.ConsumeIntegral<uint32_t>(),
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1), true, true, AUDIO_INPUT_FLAG_FAST);
        auto pipe = MakeInputPipe(fdp, paIdx, AUDIO_INPUT_FLAG_FAST, stream);
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.push_back(pipe);
    }
    AudioInjectorPolicy::GetInstance().ReleaseCaptureInjector();
    if (AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.clear();
    }
}

void RebuildCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
        AudioInjectorPolicy::GetInstance().activeTargetStreamId_ = streamId;
        AudioInjectorPolicy::GetInstance().activeTargetUid_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1);
        if (fdp.ConsumeBool() && AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
            auto stream = MakeVoipStream(streamId,
                AudioInjectorPolicy::GetInstance().activeTargetUid_, true, true, AUDIO_INPUT_FLAG_NORMAL);
            auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
            AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.push_back(pipe);
        }
    } else if (fdp.ConsumeBool() && AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        uint32_t paIdx = fdp.ConsumeIntegral<uint32_t>();
        auto stream = MakeVoipStream(streamId,
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1),
            fdp.ConsumeBool(), true, AUDIO_INPUT_FLAG_NORMAL);
        auto pipe = MakeInputPipe(fdp, paIdx, AUDIO_INPUT_FLAG_NORMAL, stream);
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.push_back(pipe);
        AudioInjectorPolicy::GetInstance().capturePortIdx_ = paIdx;
    }
    AudioInjectorPolicy::GetInstance().RebuildCaptureInjector(streamId);
    if (AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.clear();
    }
}

void RebuildCaptureInjectorByTargetStreamFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    uint32_t targetStreamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    AudioInjectorPolicy::GetInstance().activeTargetStreamId_ = targetStreamId;
    int32_t targetUid = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1);
    AudioInjectorPolicy::GetInstance().activeTargetUid_ = targetUid;
    if (fdp.ConsumeBool() && AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        auto stream = MakeVoipStream(targetStreamId, targetUid, true, true, AUDIO_INPUT_FLAG_NORMAL);
        auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.push_back(pipe);
    }
    AudioInjectorPolicy::GetInstance().RebuildCaptureInjectorByTargetStream(targetStreamId);
    if (AudioInjectorPolicy::GetInstance().pipeManager_ != nullptr) {
        AudioInjectorPolicy::GetInstance().pipeManager_->curPipeList_.clear();
    }
}

void FindCaptureVoipPipeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    if (fdp.ConsumeBool()) {
        auto stream = MakeVoipStream(streamId,
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1), true, true, AUDIO_INPUT_FLAG_NORMAL);
        auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
        pipeInfos.push_back(pipe);
    }
    uint32_t outStreamId = streamId;
    std::shared_ptr<AudioPipeInfo> result =
        AudioInjectorPolicy::GetInstance().FindCaptureVoipPipe(pipeInfos, outStreamId);
}

void FindPipeByStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    VoipType type = NO_VOIP;
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    if (fdp.ConsumeBool()) {
        auto stream = MakeVoipStream(streamId,
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1), true, true, AUDIO_INPUT_FLAG_NORMAL);
        auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
        pipeInfos.push_back(pipe);
    }
    std::shared_ptr<AudioPipeInfo> result =
        AudioInjectorPolicy::GetInstance().FindPipeByStreamId(pipeInfos, type, streamId);
}

void FetchCapDeviceInjectPreProcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    bool removeFlag = GetData<uint32_t>() % NUM_2;
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
        int32_t targetUid = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1);
        AudioInjectorPolicy::GetInstance().activeTargetUid_ = targetUid;
        AudioInjectorPolicy::GetInstance().activeTargetStreamId_ = streamId;
        if (fdp.ConsumeBool()) {
            auto stream = MakeVoipStream(streamId, targetUid, true, true, AUDIO_INPUT_FLAG_NORMAL);
            auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
            pipeInfos.push_back(pipe);
        }
    } else if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().isOpened_ = true;
        auto stream = MakeVoipStream(streamId,
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1), true, true, AUDIO_INPUT_FLAG_NORMAL);
        auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
        pipeInfos.push_back(pipe);
    }
    AudioInjectorPolicy::GetInstance().FetchCapDeviceInjectPreProc(pipeInfos, removeFlag, streamId);
}

void FetchCapDeviceInjectPostProcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    bool removeFlag = GetData<uint32_t>() % NUM_2;
    uint32_t streamId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
        int32_t targetUid = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1);
        AudioInjectorPolicy::GetInstance().activeTargetUid_ = targetUid;
        AudioInjectorPolicy::GetInstance().activeTargetStreamId_ = streamId;
        if (fdp.ConsumeBool()) {
            auto stream = MakeVoipStream(streamId, targetUid, true, true, AUDIO_INPUT_FLAG_NORMAL);
            auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
            pipeInfos.push_back(pipe);
        }
    } else if (fdp.ConsumeBool()) {
        AudioInjectorPolicy::GetInstance().isOpened_ = true;
        auto stream = MakeVoipStream(streamId,
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1), true, true, AUDIO_INPUT_FLAG_NORMAL);
        auto pipe = MakeInputPipe(fdp, fdp.ConsumeIntegral<uint32_t>(), AUDIO_INPUT_FLAG_NORMAL, stream);
        pipeInfos.push_back(pipe);
    }
    AudioInjectorPolicy::GetInstance().FetchCapDeviceInjectPostProc(pipeInfos, removeFlag, streamId);
}

void HasRunningVoipStreamFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec;
    if (fdp.ConsumeBool()) {
        auto stream = MakeVoipStream(fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2),
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA1), true, true, AUDIO_INPUT_FLAG_NORMAL);
        streamVec.push_back(stream);
    }
    bool result = AudioInjectorPolicy::GetInstance().HasRunningVoipStream(streamVec);
}

void AddInjectorStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId);
}

void DeleteInjectorStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId);
    AudioInjectorPolicy::GetInstance().DeleteInjectorStreamId(streamId);
}

void IsActivateInterruptStreamIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId);
    bool result = AudioInjectorPolicy::GetInstance().IsActivateInterruptStreamId(streamId);
}

void SendInterruptEventToInjectorStreamsFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t renderId = fdp.ConsumeIntegralInRange<uint32_t>(1, MAX_VALUE_DATA2);
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    AudioInjectorPolicy::GetInstance().AddStreamDescriptor(renderId, desc);
    std::shared_ptr<AudioPolicyServerHandler> handler = nullptr;
    AudioInjectorPolicy::GetInstance().SendInterruptEventToInjectorStreams(handler);
}

void SetInjectStreamsMuteForInjectionFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().isNeedMuteRenderer_ = fdp.ConsumeBool();
    AudioInjectorPolicy::GetInstance().SetInjectStreamsMuteForInjection(streamId);
}

void SetInjectStreamsMuteForPlaybackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t streamId = GetData<uint32_t>();
    AudioInjectorPolicy::GetInstance().isNeedMuteRenderer_ = true;
    AudioInjectorPolicy::GetInstance().SetInjectStreamsMuteForInjection(streamId);
    AudioInjectorPolicy::GetInstance().SetInjectStreamsMuteForPlayback(streamId);
}

void SetInjectorStreamsMuteFuzzTest(FuzzedDataProvider& fdp)
{
    AudioInjectorPolicy::GetInstance().Init();
    ResetInjectorState();
    uint32_t streamId1 = fdp.ConsumeIntegralInRange<uint32_t>(1, 50000);
    uint32_t streamId2 = fdp.ConsumeIntegralInRange<uint32_t>(50001, MAX_VALUE_DATA2);
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId1);
    AudioInjectorPolicy::GetInstance().AddInjectorStreamId(streamId2);
    bool newMicrophoneMute = GetData<uint32_t>() % NUM_2;
    AudioInjectorPolicy::GetInstance().SetInjectorStreamsMute(newMicrophoneMute);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    InitFuzzTest,
    DeInitFuzzTest,
    UpdateAudioInfoFuzzTest,
    AddStreamDescriptorFuzzTest,
    RemoveStreamDescriptorFuzzTest,
    IsContainStreamFuzzTest,
    GetAdapterNameFuzzTest,
    GetRendererStreamCountFuzzTest,
    SetCapturePortIdxFuzzTest,
    GetCapturePortIdxFuzzTest,
    SetRendererPortIdxFuzzTest,
    GetRendererPortIdxFuzzTest,
    GetAudioModuleInfoFuzzTest,
    GetIsConnectedFuzzTest,
    SetVoipTypeFuzzTest,
    PrepareLegacyRendererFuzzTest,
    PreparePreciseRendererFuzzTest,
    ClearRendererTargetFuzzTest,
    IsPreciseRendererFuzzTest,
    AddCaptureInjectorFuzzTest,
    AddCaptureInjectorInnerFuzzTest,
    AddCaptureInjectorByTargetStreamFuzzTest,
    AddCaptureInjectorByTargetStreamInnerFuzzTest,
    RemoveCaptureInjectorFuzzTest,
    RemoveCaptureInjectorInnerFuzzTest,
    RemoveCaptureInjectorByTargetStreamInnerFuzzTest,
    ReleaseCaptureInjectorFuzzTest,
    RebuildCaptureInjectorFuzzTest,
    RebuildCaptureInjectorByTargetStreamFuzzTest,
    FindCaptureVoipPipeFuzzTest,
    FindPipeByStreamIdFuzzTest,
    FetchCapDeviceInjectPreProcFuzzTest,
    FetchCapDeviceInjectPostProcFuzzTest,
    HasRunningVoipStreamFuzzTest,
    AddInjectorStreamIdFuzzTest,
    DeleteInjectorStreamIdFuzzTest,
    IsActivateInterruptStreamIdFuzzTest,
    SendInterruptEventToInjectorStreamsFuzzTest,
    SetInjectStreamsMuteForInjectionFuzzTest,
    SetInjectStreamsMuteForPlaybackFuzzTest,
    SetInjectorStreamsMuteFuzzTest,
    });
    func(fdp);
}

void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}

void Init()
{
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
