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
#include "oh_audio_buffer.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const int32_t NUM_1 = 1;
const int32_t NUM_1000 = 1000;
const int32_t NUM_1024 = 1024;

typedef void (*TestFuncs)();

class AudioSharedMemoryFuzz : public AudioSharedMemory {
public:
    explicit AudioSharedMemoryFuzz() = default;
    virtual ~AudioSharedMemoryFuzz() = default;
    uint8_t *GetBase() { return nullptr; };
    size_t GetSize() { return 0; };
    int GetFd() { return 0; };
    std::string GetName() { return ""; };
};

void MarshallingFuzzTest()
{
    shared_ptr<AudioSharedMemoryFuzz> audioSharedMemory =
        std::make_shared<AudioSharedMemoryFuzz>();
    if (audioSharedMemory == nullptr) {
        return;
    }
    Parcel parcel;
    audioSharedMemory->Marshalling(parcel);
}

void UnmarshallingFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<AudioSharedMemoryFuzz> audioSharedMemory =
        std::make_shared<AudioSharedMemoryFuzz>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (audioSharedMemory == nullptr) {
        return;
    }
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    Parcel parcel;
    audioSharedMemory->Marshalling(parcel);
    audioSharedMemory->Unmarshalling(parcel);
    ohAudioBufferBase->Marshalling(parcel);
    ohAudioBufferBase->Unmarshalling(parcel);
}

void GetReadableDataFramesFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->GetReadableDataFrames();
}

void SharedMemoryWriteToParcelFuzzTest()
{
    shared_ptr<AudioSharedMemoryFuzz> audioSharedMemory =
        std::make_shared<AudioSharedMemoryFuzz>();
    if (audioSharedMemory == nullptr) {
        return;
    }
    MessageParcel parcel;
    audioSharedMemory->WriteToParcel(audioSharedMemory, parcel);
    audioSharedMemory->ReadFromParcel(parcel);
}

void BufferBaseReadFromParcelFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    MessageParcel parcel;
    ohAudioBufferBase->bufferHolder_ = selectedAudioBufferHolder;
    shared_ptr<AudioSharedMemoryFuzz> dataMem = std::make_shared<AudioSharedMemoryFuzz>();
    shared_ptr<AudioSharedMemoryFuzz> statusMem = std::make_shared<AudioSharedMemoryFuzz>();
    ohAudioBufferBase->dataMem_ = dataMem;
    ohAudioBufferBase->statusInfoMem_ = statusMem;
    ohAudioBufferBase->WriteToParcel(ohAudioBufferBase, parcel);
    ohAudioBufferBase->ReadFromParcel(parcel);
}

void GetInitializationInfoFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    shared_ptr<AudioSharedMemoryFuzz> dataMem = std::make_shared<AudioSharedMemoryFuzz>();
    shared_ptr<AudioSharedMemoryFuzz> statusMem = std::make_shared<AudioSharedMemoryFuzz>();
    ohAudioBufferBase->dataMem_ = dataMem;
    ohAudioBufferBase->statusInfoMem_ = statusMem;
    ohAudioBufferBase->GetInitializationInfo();
}

void SetStopFlagFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    bool isNeedStop = g_fuzzUtils.GetData<bool>();
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->SetStopFlag(isNeedStop);
    ohAudioBufferBase->GetStopFlag();
}

void GetSessionIdFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    ohAudioBufferBase->SetSessionId(g_fuzzUtils.GetData<uint32_t>());
    ohAudioBufferBase->GetSessionId();
}

void GetSizeParameterFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    ohAudioBufferBase->GetSizeParameter(totalSizeInFrame, byteSizePerFrame);
}

void GetUnderrunCountFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    if (ohAudioBufferBase->basicBufferInfo_ == nullptr) {
        return;
    }
    ohAudioBufferBase->basicBufferInfo_->underrunCount = g_fuzzUtils.GetData<uint32_t>();
    ohAudioBufferBase->GetUnderrunCount();
}

void GetHandleInfoFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    auto frames = g_fuzzUtils.GetData<uint64_t>();
    auto nanoTime = g_fuzzUtils.GetData<int64_t>();
    ohAudioBufferBase->SetHandleInfo(frames, nanoTime);
    ohAudioBufferBase->GetHandleInfo(frames, nanoTime);
}

void SetStreamVolumeFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->SetStreamVolume(g_fuzzUtils.GetData<float>());
}

void SetMuteFactorFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->SetMuteFactor(g_fuzzUtils.GetData<float>());
    ohAudioBufferBase->GetMuteFactor();
}

void SetDuckFactorFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->SetDuckFactor(g_fuzzUtils.GetData<float>());
    ohAudioBufferBase->GetDuckFactor();
}

void GetBasePosInFrameFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->GetBasePosInFrame();
}

void SetCurWriteFrameFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    if (ohAudioBufferBase->basicBufferInfo_ == nullptr) {
        return;
    }
    ohAudioBufferBase->basicBufferInfo_->basePosInFrame = g_fuzzUtils.GetData<uint64_t>();
    ohAudioBufferBase->basicBufferInfo_->curWriteFrame = g_fuzzUtils.GetData<uint64_t>();
    ohAudioBufferBase->SetCurWriteFrame(g_fuzzUtils.GetData<uint64_t>());
    ohAudioBufferBase->SetCurReadFrame(g_fuzzUtils.GetData<uint64_t>());
}

void GetOffsetByFrameFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    if (ohAudioBufferBase->basicBufferInfo_ == nullptr) {
        return;
    }
    ohAudioBufferBase->basicBufferInfo_->basePosInFrame = g_fuzzUtils.GetData<uint64_t>();
    size_t offset = g_fuzzUtils.GetData<size_t>();
    ohAudioBufferBase->GetOffsetByFrame(g_fuzzUtils.GetData<uint64_t>(), offset);
}

void GetOffsetByFrameForWriteFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    if (ohAudioBufferBase->basicBufferInfo_ == nullptr) {
        return;
    }
    ohAudioBufferBase->basicBufferInfo_->basePosInFrame = g_fuzzUtils.GetData<uint64_t>();
    ohAudioBufferBase->basicBufferInfo_->curReadFrame = g_fuzzUtils.GetData<uint64_t>();
    size_t offset = g_fuzzUtils.GetData<size_t>();
    ohAudioBufferBase->GetOffsetByFrameForWrite(g_fuzzUtils.GetData<uint64_t>(), offset);
    ohAudioBufferBase->GetOffsetByFrameForRead(g_fuzzUtils.GetData<uint64_t>(), offset);
}

void GetBufferByOffsetFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    RingBufferWrapper buffer;
    size_t offset = g_fuzzUtils.GetData<size_t>();
    size_t dataLength = g_fuzzUtils.GetData<size_t>();
    ohAudioBufferBase->GetBufferByOffset(offset, dataLength, buffer);
}

void TryGetContinuousBufferByOffsetFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    BufferDesc bufferDesc;
    size_t offset = g_fuzzUtils.GetData<size_t>();
    size_t dataLength = g_fuzzUtils.GetData<size_t>();
    ohAudioBufferBase->TryGetContinuousBufferByOffset(offset, dataLength, bufferDesc);
}

void GetBufferByFrameFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    RingBufferWrapper buffer;
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    if (ohAudioBufferBase->basicBufferInfo_ == nullptr) {
        return;
    }
    ohAudioBufferBase->totalSizeInFrame_ = g_fuzzUtils.GetData<uint32_t>();
    uint64_t beginPosInFrame = g_fuzzUtils.GetData<uint64_t>();
    uint64_t sizeInFrame = g_fuzzUtils.GetData<uint64_t>();
    byteSizePerFrame = (byteSizePerFrame % NUM_1024) + NUM_1;
    const uint32_t maxFrames = NUM_1000;
    totalSizeInFrame = totalSizeInFrame % maxFrames;
    uint32_t totalSizeInByte = totalSizeInFrame * byteSizePerFrame;
    ohAudioBufferBase->totalSizeInByte_ = totalSizeInByte;
    auto dataBuffer = std::make_unique<uint8_t[]>(totalSizeInByte);
    ohAudioBufferBase->dataBase_ = dataBuffer.get();
    ohAudioBufferBase->GetBufferByFrame(beginPosInFrame, sizeInFrame, buffer);
    ohAudioBufferBase->basicBufferInfo_->curWriteFrame = beginPosInFrame;
    ohAudioBufferBase->GetAllWritableBuffer(buffer);
    ohAudioBufferBase->basicBufferInfo_->curReadFrame = beginPosInFrame;
    ohAudioBufferBase->GetAllReadableBuffer(buffer);
}

void GetLastWrittenTimeFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    ohAudioBufferBase->SetLastWrittenTime(g_fuzzUtils.GetData<int64_t>());
    ohAudioBufferBase->GetLastWrittenTime();
}

void GetSyncReadFrameFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    uint32_t readFrame = g_fuzzUtils.GetData<uint32_t>();
    ohAudioBufferBase->SetSyncReadFrame(readFrame);
    ohAudioBufferBase->GetSyncReadFrame();
}

void GetSyncWriteFrameFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    uint32_t writeFrame = g_fuzzUtils.GetData<uint32_t>();
    ohAudioBufferBase->SetSyncWriteFrame(writeFrame);
    ohAudioBufferBase->GetSyncWriteFrame();
}

void GetRestoreInfoFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    RestoreInfo restoreInfo;
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->SetRestoreInfo(restoreInfo);
    ohAudioBufferBase->GetRestoreInfo(restoreInfo);
}

void GetTimeStampInfoFuzzTest()
{
    int32_t count = static_cast<uint32_t>(AudioBufferHolder::AUDIO_SERVER_INDEPENDENT)+ NUM_1;
    AudioBufferHolder selectedAudioBufferHolder =
        static_cast<AudioBufferHolder>(g_fuzzUtils.GetData<uint8_t>() % count);
    uint32_t totalSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint32_t byteSizePerFrame = g_fuzzUtils.GetData<uint32_t>();
    shared_ptr<OHAudioBufferBase> ohAudioBufferBase =
        std::make_shared<OHAudioBufferBase>(selectedAudioBufferHolder, totalSizeInFrame, byteSizePerFrame);
    if (ohAudioBufferBase == nullptr) {
        return;
    }
    uint64_t position = g_fuzzUtils.GetData<uint64_t>();
    uint64_t timeStamp = g_fuzzUtils.GetData<uint64_t>();
    auto basicInfo = std::make_shared<BasicBufferInfo>();
    ohAudioBufferBase->basicBufferInfo_ = basicInfo.get();
    ohAudioBufferBase->SetTimeStampInfo(position, timeStamp);
    ohAudioBufferBase->GetTimeStampInfo(position, timeStamp);
}

vector<TestFuncs> g_testFuncs = {
    MarshallingFuzzTest,
    UnmarshallingFuzzTest,
    GetReadableDataFramesFuzzTest,
    SharedMemoryWriteToParcelFuzzTest,
    BufferBaseReadFromParcelFuzzTest,
    GetInitializationInfoFuzzTest,
    SetStopFlagFuzzTest,
    GetSessionIdFuzzTest,
    GetSizeParameterFuzzTest,
    GetUnderrunCountFuzzTest,
    GetHandleInfoFuzzTest,
    SetStreamVolumeFuzzTest,
    SetMuteFactorFuzzTest,
    SetDuckFactorFuzzTest,
    GetBasePosInFrameFuzzTest,
    SetCurWriteFrameFuzzTest,
    GetOffsetByFrameFuzzTest,
    GetOffsetByFrameForWriteFuzzTest,
    GetBufferByOffsetFuzzTest,
    TryGetContinuousBufferByOffsetFuzzTest,
    GetBufferByFrameFuzzTest,
    GetLastWrittenTimeFuzzTest,
    GetSyncReadFrameFuzzTest,
    GetSyncWriteFrameFuzzTest,
    GetRestoreInfoFuzzTest,
    GetTimeStampInfoFuzzTest
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
