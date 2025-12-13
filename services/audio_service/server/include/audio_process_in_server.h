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

#ifndef AUDIO_PROCESS_IN_SERVER_H
#define AUDIO_PROCESS_IN_SERVER_H

#include <mutex>
#include <sstream>

#include "audio_process_stub.h"
#include "i_audio_process_stream.h"
#include "i_process_status_listener.h"
#include "player_dfx_writer.h"
#include "recorder_dfx_writer.h"
#include "audio_schedule_guard.h"
#include "audio_stream_monitor.h"
#include "audio_stream_checker.h"
#include "audio_proresampler.h"
#include "format_converter.h"
#include "audio_static_buffer_processor.h"
#include "audio_static_buffer_provider.h"

namespace OHOS {
namespace AudioStandard {
class ProcessReleaseCallback {
public:
    virtual ~ProcessReleaseCallback() = default;

    virtual int32_t OnProcessRelease(IAudioProcessStream *process, bool isSwitchStream = false) = 0;
};
class AudioProcessInServer;
class ProcessDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    ProcessDeathRecipient(AudioProcessInServer *processInServer, ProcessReleaseCallback *processHolder);
    virtual ~ProcessDeathRecipient() = default;
    // overridde for DeathRecipient
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    ProcessReleaseCallback *processHolder_ = nullptr;
    AudioProcessInServer *processInServer_ = nullptr;
    int64_t createTime_ = 0;
};

class AudioProcessInServer : public AudioProcessStub, public IAudioProcessStream {
public:

    enum HandleRendererDataType : uint32_t {
        NONE_ACTION = 0,
        CONVERT_TO_F32_ACTION = 0x1,
        RESAMPLE_ACTION = 0x10,
        CONVERT_TO_SERVER_ACTION = 0x100,
    };

    static sptr<AudioProcessInServer> Create(const AudioProcessConfig &processConfig,
        ProcessReleaseCallback *releaseCallback);
    virtual ~AudioProcessInServer();

    // override for AudioProcess
    int32_t ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
        uint32_t &spanSizeInFrame) override;

    int32_t GetSessionId(uint32_t &sessionId) override;

    int32_t Start() override;

    int32_t Pause(bool isFlush) override;

    int32_t Resume() override;

    int32_t Stop(int32_t stage) override;

    int32_t RequestHandleInfo() override;

    int32_t RequestHandleInfoAsync() override;

    int32_t Release(bool isSwitchStream) override;

    int32_t SetDefaultOutputDevice(int32_t defaultOutputDevice, bool skipForce = false) override;

    int32_t SetSilentModeAndMixWithOthers(bool on) override;

    int32_t SetSourceDuration(int64_t duration) override;

    int32_t SetUnderrunCount(uint32_t underrunCnt) override;

    int32_t SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, const std::string& adjustTime,
        uint32_t code) override;

    int32_t RegisterProcessCb(const sptr<IRemoteObject>& object) override;

    int32_t RegisterThreadPriority(int32_t tid, const std::string &bundleName,
        uint32_t method) override;

    int32_t SetRebuildFlag() override;

    int32_t GetServerKeepRunning(bool &keepRunning) override;
    
    int32_t SetAudioHapticsSyncId(int32_t audioHapticsSyncId) override;
    void CheckAudioHapticsSyncId(int32_t &audioHapticsSyncId);

    // override for IAudioProcessStream, used in endpoint
    std::shared_ptr<OHAudioBufferBase> GetStreamBuffer() override;
    AudioStreamInfo GetStreamInfo() override;
    uint32_t GetAudioSessionId() override;
    AudioStreamType GetAudioStreamType() override;
    StreamUsage GetUsage() override;
    SourceType GetSource() override;
    AudioProcessConfig GetAudioProcessConfig() override;
    void EnableStandby() override;

    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void Dump(std::string &dumpString);

    int32_t ConfigProcessBuffer(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        AudioStreamInfo &serverStreamInfo);

    int32_t AddProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener);
    int32_t RemoveProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener);

    void SetNonInterruptMute(const bool muteFlag);
    bool GetMuteState() override;
    uint32_t GetSessionId();
    int32_t GetStandbyStatus(bool &isStandby, int64_t &enterStandbyTime);

    // for inner-cap
    void SetInnerCapState(bool isInnerCapped, int32_t innerCapId) override;
    bool GetInnerCapState(int32_t innerCapId) override;
    std::unordered_map<int32_t, bool> GetInnerCapState() override;

    AppInfo GetAppInfo() override final;
    BufferDesc &GetConvertedBuffer() override;

    bool NeedUseTempBuffer(const RingBufferWrapper &ringBuffer, size_t spanSizeInByte);
    virtual bool PrepareRingBuffer(uint64_t curRead, RingBufferWrapper& ringBuffer,
        int32_t &audioHapticsSyncId) override;
    virtual void PrepareStreamDataBuffer(size_t spanSizeInByte,
        RingBufferWrapper &ringBuffer, AudioStreamData &streamData) override;

    void WriteDumpFile(void *buffer, size_t bufferSize) override final;

    std::time_t GetStartMuteTime() override;
    void SetStartMuteTime(std::time_t time) override;
 
    bool GetSilentState() override;
    void SetSilentState(bool state) override;
    void SetKeepRunning(bool keepRunning) override;
    bool GetKeepRunning() override;
    void AddMuteFrameSize(int64_t muteFrameCnt) override;
    void AddNormalFrameSize() override;
    void AddNoDataFrameSize() override;
    StreamStatus GetStreamStatus() override;
    RestoreStatus RestoreSession(RestoreInfo restoreInfo);
    int32_t StopSession();
    StreamStatus GetStreamInServerStatus() override;

    bool TurnOnMicIndicator(CapturerState capturerState);
    bool TurnOffMicIndicator(CapturerState capturerState);

    uint32_t GetSpanSizeInFrame() override;
    uint32_t GetByteSizePerFrame() override;

    int32_t WriteToSpecialProcBuf(AudioCaptureDataProcParams &procParams) override;
    void UpdateStreamInfo() override;

    void DfxOperationAndCalcMuteFrame(BufferDesc &bufferDesc) override;

    int32_t PreSetLoopTimes(int64_t bufferLoopTimes) override;
    int32_t GetStaticBufferInfo(StaticBufferInfo &staticBufferInfo) override;
    int32_t SetStaticRenderRate(uint32_t renderRate) override;
public:
    const AudioProcessConfig processConfig_;

private:
    int32_t StartInner();
    int64_t GetLastAudioDuration();
    void PrepareStreamDataBufferInner(size_t spanSizeInByte, RingBufferWrapper &ringBuffer, BufferDesc &dstBufferDesc);
    AudioProcessInServer(const AudioProcessConfig &processConfig, ProcessReleaseCallback *releaseCallback);
    int32_t InitBufferStatus();
    void InitRendererStream(uint32_t spanTime,
        const AudioStreamInfo &clientStreamInfo, const AudioStreamInfo &serverStreamInfo);
    void InitCapturerStream(uint32_t spanSizeInByte,
        const AudioStreamInfo &clientStreamInfo, const AudioStreamInfo &serverStreamInfo);
    void RendererResample(BufferDesc &buffer);
    void RendererConvertF32(BufferDesc &buffer);
    void RendererConvertServer(BufferDesc &buffer);

    bool CheckBGCapturer();
    void WriterRenderStreamStandbySysEvent(uint32_t sessionId, int32_t standby);
    void ReportDataToResSched(std::unordered_map<std::string, std::string> payload, uint32_t type);
    void NotifyXperfOnPlayback(AudioMode audioMode, XperfEventId eventId);

    int32_t HandleCapturerDataParams(RingBufferWrapper &writeBuf, AudioCaptureDataProcParams &procParams);
    void SetCaptureStreamInfo(AudioStreamInfo &srcInfo, AudioCaptureDataProcParams &procParams);
    int32_t CaptureDataResampleProcess(const size_t bufLen, BufferDesc &outBuf, AudioStreamInfo &srcInfo,
                                       AudioCaptureDataProcParams &procParams);
    int32_t CapturerDataFormatAndChnConv(RingBufferWrapper &writeBuf, BufferDesc &resampleOutBuf,
                                         const AudioStreamInfo &srcInfo, const AudioStreamInfo &dstInfo);
    int32_t WriteToRingBuffer(RingBufferWrapper &writeBuf, const BufferDesc &buffer);
    void RemoveStreamInfo();
    void ReleaseCaptureInjector();
    void RebuildCaptureInjector();
    bool IsNeedRecordResampleConv(AudioSamplingRate srcSamplingRate);

    int32_t CreateServerBuffer();
    int32_t ProcessAndSetStaticBuffer();
private:
    std::atomic<bool> muteFlag_ = false;
    std::atomic<bool> silentModeAndMixWithOthers_ = false;
    std::mutex innerCapStateMutex_;
    std::unordered_map<int32_t, bool> innerCapStates_;
    ProcessReleaseCallback *releaseCallback_ = nullptr;
    std::mutex registerProcessCbLock_;
    sptr<IRemoteObject> object_ = nullptr;
    sptr<ProcessDeathRecipient> deathRecipient_ = nullptr;

    bool needCheckBackground_ = false;
    bool isMicIndicatorOn_ = false;

    uint32_t sessionId_ = 0;
    std::atomic<bool> isInited_ = false;
    std::atomic<StreamStatus> *streamStatus_ = nullptr;
    std::mutex statusLock_;

    std::string clientBundleName_;

    uint32_t totalSizeInframe_ = 0;
    uint32_t spanSizeInframe_ = 0;
    uint32_t byteSizePerFrame_ = 0;
    bool isBufferConfiged_ = false;
    std::shared_ptr<OHAudioBufferBase> processBuffer_ = nullptr;
    std::vector<uint8_t> processTmpBuffer_;
    std::mutex listenerListLock_;
    std::vector<std::shared_ptr<IProcessStatusListener>> listenerList_;
    AudioStreamInfo serverStreamInfo_;
    BufferDesc resampleBuffer_ = {};
    BufferDesc f32Buffer_ = {};
    BufferDesc convertedBuffer_ = {};
    std::unique_ptr<uint8_t []> resampleBufferNew_ = nullptr;
    std::unique_ptr<uint8_t []> f32BufferNew_ = nullptr;
    std::unique_ptr<uint8_t []> convertedBufferNew_ = nullptr;

    FormatKey dataToServerKey_ = {};
    FormatKey clientToResampleKey_ = {};
    uint32_t handleRendererDataType_ = NONE_ACTION;
    
    std::string dumpFileName_;
    FILE *dumpFile_ = nullptr;
    int64_t enterStandbyTime_ = 0;
    std::time_t startMuteTime_ = 0;
    bool isInSilentState_ = false;
    bool keepRunning_ = false;
    int64_t lastStartTime_{};
    int64_t lastStopTime_{};
    int64_t lastWriteFrame_{};
    int64_t lastWriteMuteFrame_{};
    std::atomic<uint32_t> underrunCount_ = 0;
    int64_t sourceDuration_ = -1;
    std::unique_ptr<PlayerDfxWriter> playerDfx_;
    std::unique_ptr<RecorderDfxWriter> recorderDfx_;

    std::array<std::shared_ptr<SharedAudioScheduleGuard>, METHOD_MAX> scheduleGuards_ = {};
    std::mutex scheduleGuardsMutex_;
    std::shared_ptr<AudioStreamChecker> audioStreamChecker_ = nullptr;
    
    std::mutex syncIdLock_;
    int32_t audioHapticsSyncId_ = 0;
    uint32_t audioCheckFreq_ = 0;
    std::atomic<uint32_t> checkCount_ = 0;

    StreamStatus streamStatusInServer_ = STREAM_INVALID;

    std::unique_ptr<HPAE::ProResampler> resampler_ = nullptr;

    std::atomic<bool> rebuildFlag_ = false;

    std::string logUtilsTag_ = "";

    mutable int64_t volumeDataCount_ = 0;

    AudioRendererRate audioRenderRate_ = RENDER_RATE_NORMAL;
    std::shared_ptr<AudioStaticBufferProcessor> staticBufferProcessor_ = nullptr;
    std::shared_ptr<AudioStaticBufferProvider> staticBufferProvider_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PROCESS_IN_SERVER_H
