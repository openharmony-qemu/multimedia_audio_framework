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
#ifndef HPAE_SOFT_LINK_H
#define HPAE_SOFT_LINK_H
#define HDI_INVALID_ID 0xFFFFFFFF

#include "i_hpae_soft_link.h"
#include "i_capturer_stream.h"
#include "i_renderer_stream.h"
#include "hpae_define.h"
#include "audio_ring_cache.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

enum class HpaeSoftLinkState : int32_t {
    INVALID = -1,
    NEW,
    PREPARED,
    RUNNING,
    STOPPED,
    RELEASED,
};

enum HpaeSoftLinkStreamOperation : uint8_t {
    SOFTLINK_RENDERER_OPERATION = 1 << 0, // bit 0
    SOFTLINK_CAPTURER_OPERATION = 1 << 1, // bit 1
};

enum HpaeSoftLinkDeviceOperation : uint8_t {
    SOFTLINK_SINK_OPERATION = 1 << 0, // bit 0
    SOFTLINK_SOURCE_OPERATION = 1 << 1, // bit 1
};

class HpaeSoftLink : public std::enable_shared_from_this<HpaeSoftLink>,
                     public IHpaeSoftLink,
                     public IStreamStatusCallback,
                     public IStreamCallback,
                     public ICapturerStreamCallback {
public:
    HpaeSoftLink(uint32_t sinkIdx, uint32_t sourceIdx, SoftLinkMode mode);
    ~HpaeSoftLink();
    static uint32_t GenerateSessionId();
    int32_t Init() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Release() override;
    int32_t SetVolume(float volume) override;
    void OnStatusUpdate(IOperation operation, uint32_t streamIndex) override;
    int32_t OnStreamData(AudioCallBackStreamInfo& callbackStreamInfo) override;
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo& callbackStreamInfo) override;
    void OnDeviceInfoReceived(const HpaeSoftLinkDeviceOperation &operation);

    // for unit test
    HpaeSoftLinkState GetStreamStateById(uint32_t sessionId);
private:
    int32_t GetDeviceInfo();
    int32_t CreateStream();
    void FlushRingCache();
    void TransSinkInfoToStreamInfo(HpaeStreamInfo &info, const HpaeStreamClassType &streamClassType);
private:
    static uint32_t g_sessionId;
    inline static std::mutex sessionIdMutex_;
    uint32_t sinkIdx_ = HDI_INVALID_ID;
    uint32_t sourceIdx_ = HDI_INVALID_ID;
    SoftLinkMode linkMode_ = SoftLinkMode::HEARING_AID;
    HpaeSinkInfo sinkInfo_;
    HpaeSourceInfo sourceInfo_;
    HpaeStreamInfo rendererStreamInfo_;
    HpaeStreamInfo capturerStreamInfo_;
    std::unique_ptr<AudioRingCache> bufferQueue_ = nullptr;
    std::vector<char> tempBuffer_;
    HpaeSoftLinkState state_ = HpaeSoftLinkState::INVALID;
    std::mutex stateMutex_;
    uint8_t isStreamOperationFinish_ = 0;
    uint8_t isDeviceOperationFinish_ = 0;
    std::unordered_map<uint32_t, HpaeSoftLinkState> streamStateMap_;
    std::mutex callbackMutex_;
    std::condition_variable callbackCV_;
    bool isOperationFinish_ = false;
    int32_t overFlowCount_ = 0;
    int32_t underRunCount_ = 0;
};
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_SOFT_LINK_H
