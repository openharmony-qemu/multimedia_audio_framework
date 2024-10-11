/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef RENDERER_IN_SERVER_H
#define RENDERER_IN_SERVER_H

#include <mutex>
#include "i_renderer_stream.h"
#include "i_stream_listener.h"
#include "oh_audio_buffer.h"
#include "i_stream_manager.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
class StreamCallbacks : public IStatusCallback, public IWriteCallback {
public:
    explicit StreamCallbacks(uint32_t streamIndex);
    virtual ~StreamCallbacks() = default;
    void OnStatusUpdate(IOperation operation) override;
    int32_t OnWriteData(size_t length) override;
private:
    uint32_t streamIndex_ = 0;
};

class RendererInServer : public IStatusCallback, public IWriteCallback,
    public std::enable_shared_from_this<RendererInServer> {
public:
    RendererInServer(AudioProcessConfig processConfig, std::weak_ptr<IStreamListener> streamListener);
    virtual ~RendererInServer();
    void OnStatusUpdate(IOperation operation) override;
    void OnStatusUpdateExt(IOperation operation, std::shared_ptr<IStreamListener> stateListener);
    void HandleOperationFlushed();
    int32_t OnWriteData(size_t length) override;

    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer);
    int32_t GetSessionId(uint32_t &sessionId);
    int32_t Start();
    int32_t Pause();
    int32_t Flush();
    int32_t Drain(bool stopFlag = false);
    int32_t Stop();
    int32_t Release();

    int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp);
    int32_t GetAudioPosition(uint64_t &framePos, uint64_t &timestamp);
    int32_t GetLatency(uint64_t &latency);
    int32_t SetRate(int32_t rate);
    int32_t SetLowPowerVolume(float volume);
    int32_t GetLowPowerVolume(float &volume);
    int32_t SetAudioEffectMode(int32_t effectMode);
    int32_t GetAudioEffectMode(int32_t &effectMode);
    int32_t SetPrivacyType(int32_t privacyType);
    int32_t GetPrivacyType(int32_t &privacyType);

    int32_t SetOffloadMode(int32_t state, bool isAppBack);
    int32_t UnsetOffloadMode();
    int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa);
    int32_t OffloadSetVolume(float volume);
    int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled);

    int32_t Init();
    int32_t ConfigServerBuffer();
    int32_t InitBufferStatus();
    int32_t UpdateWriteIndex();
    BufferDesc DequeueBuffer(size_t length);
    void VolumeHandle(BufferDesc &desc);
    int32_t WriteData();
    void WriteEmptyData();
    int32_t DrainAudioBuffer();

    // for inner-cap
    int32_t EnableInnerCap();
    int32_t DisableInnerCap();
    int32_t InitDupStream();

    // for dual tone
    int32_t EnableDualTone();
    int32_t DisableDualTone();
    int32_t InitDualToneStream();

    int32_t GetStreamManagerType() const noexcept;
    int32_t SetSilentModeAndMixWithOthers(bool on);
    int32_t SetClientVolume();
    int32_t GetActualStreamManagerType() const noexcept;
public:
    const AudioProcessConfig processConfig_;
private:
    void OnStatusUpdateSub(IOperation operation);
    bool IsHighResolution() const noexcept;
    void DoFadingOut(BufferDesc& bufferDesc);
    void WriteMuteDataSysEvent(uint8_t *buffer, size_t bufferSize);
    void ReportDataToResSched(bool isSilent);
    void OtherStreamEnqueue(const BufferDesc &bufferDesc);
    void StandByCheck();
    bool ShouldEnableStandBy();

private:
    std::mutex statusLock_;
    std::condition_variable statusCv_;
    std::shared_ptr<IRendererStream> stream_ = nullptr;
    uint32_t streamIndex_ = -1;
    std::string traceTag_;
    IStatus status_ = I_STATUS_IDLE;
    bool offloadEnable_ = false;
    bool standByEnable_ = false;

    // for inner-cap
    std::mutex dupMutex_;
    std::atomic<bool> isInnerCapEnabled_ = false;
    uint32_t dupStreamIndex_ = 0;
    std::shared_ptr<StreamCallbacks> dupStreamCallback_ = nullptr;
    std::shared_ptr<IRendererStream> dupStream_ = nullptr;

    // for dual sink tone
    std::mutex dualToneMutex_;
    std::atomic<bool> isDualToneEnabled_ = false;
    uint32_t dualToneStreamIndex_ = 0;
    std::shared_ptr<IRendererStream> dualToneStream_ = nullptr;

    std::weak_ptr<IStreamListener> streamListener_;
    size_t totalSizeInFrame_ = 0;
    size_t spanSizeInFrame_ = 0;
    size_t spanSizeInByte_ = 0;
    size_t byteSizePerFrame_ = 0;
    bool isBufferConfiged_  = false;
    std::atomic<bool> isInited_ = false;
    std::shared_ptr<OHAudioBuffer> audioServerBuffer_ = nullptr;
    std::atomic<size_t> needForceWrite_ = 0;
    bool afterDrain = false;
    float lowPowerVolume_ = 1.0f;
    bool isNeedFade_ = false;
    float oldAppliedVolume_ = MAX_FLOAT_VOLUME;
    std::mutex updateIndexLock_;
    uint32_t underrunCount_ = 0;
    uint32_t standByCounter_ = 0;
    int64_t lastWriteTime_ = 0;
    bool resetTime_ = false;
    uint64_t resetTimestamp_ = 0;
    std::mutex writeLock_;
    FILE *dumpC2S_ = nullptr; // client to server dump file
    std::string dumpFileName_ = "";
    ManagerType managerType_;
    std::mutex fadeoutLock_;
    int32_t fadeoutFlag_ = 0;
    std::time_t startMuteTime_ = 0;
    int32_t silentState_ = 1; // 0:silent 1:unsilent
    std::atomic<bool> silentModeAndMixWithOthers_ = false;
    int32_t renderEmptyCountForInnerCap_ = 0;
    int32_t effectModeWhenDual_ = EFFECT_DEFAULT;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // RENDERER_IN_SERVER_H
