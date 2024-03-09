/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <chrono>
#include <thread>
#include <vector>

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "parameters.h"
#include "audio_stream.h"
#include "hisysevent.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"

using namespace std;
using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AudioStandard {
const unsigned long long TIME_CONVERSION_US_S = 1000000ULL; /* us to s */
const unsigned long long TIME_CONVERSION_NS_US = 1000ULL; /* ns to us */
const unsigned long long TIME_CONVERSION_NS_S = 1000000000ULL; /* ns to s */
constexpr int32_t WRITE_RETRY_DELAY_IN_US = 500;
constexpr int32_t CB_WRITE_BUFFERS_WAIT_IN_MS = 80;
constexpr int32_t CB_READ_BUFFERS_WAIT_IN_MS = 80;
#ifdef SONIC_ENABLE
constexpr int32_t MAX_BUFFER_SIZE = 100000;
#endif
static AppExecFwk::BundleInfo gBundleInfo_;

static AppExecFwk::BundleInfo GetBundleInfoFromUid(int32_t appUid)
{
    std::string bundleName {""};
    AppExecFwk::BundleInfo bundleInfo;
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        return bundleInfo;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        return bundleInfo;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        return bundleInfo;
    }
    if (bundleMgrProxy != nullptr) {
        bundleMgrProxy->GetNameForUid(appUid, bundleName);
    }

    bundleMgrProxy->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO |
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_HASH_VALUE,
        bundleInfo,
        AppExecFwk::Constants::ALL_USERID);

    return bundleInfo;
}

AudioStream::AudioStream(AudioStreamType eStreamType, AudioMode eMode, int32_t appUid)
    : eStreamType_(eStreamType),
      eMode_(eMode),
      state_(NEW),
      resetTime_(false),
      resetTimestamp_(0),
      renderMode_(RENDER_MODE_NORMAL),
      captureMode_(CAPTURE_MODE_NORMAL),
      isReadyToWrite_(false),
      isReadyToRead_(false),
      isFirstRead_(false),
      isFirstWrite_(false),
      isPausing_(false),
      pfd_(nullptr)
{
    AUDIO_DEBUG_LOG("AudioStream ctor, appUID = %{public}d", appUid);
    audioStreamTracker_ =  std::make_unique<AudioStreamTracker>(eMode, appUid);
    gBundleInfo_ = GetBundleInfoFromUid(appUid);
    AUDIO_DEBUG_LOG("AudioStreamTracker created");
}

AudioStream::~AudioStream()
{
    isReadyToWrite_ = false;
    isReadyToRead_ = false;

    if (writeThread_ && writeThread_->joinable()) {
        writeThread_->join();
    }

    if (readThread_ && readThread_->joinable()) {
        readThread_->join();
    }

    if (state_ != RELEASED && state_ != NEW) {
        ReleaseAudioStream(false);
    }

    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("Calling update tracker");
        AudioRendererInfo rendererInfo = {};
        AudioCapturerInfo capturerInfo = {};
        state_ = RELEASED;
        audioStreamTracker_->UpdateTracker(sessionId_, state_, GetClientPid(), rendererInfo, capturerInfo);
    }

    if (pfd_ != nullptr) {
        fclose(pfd_);
        pfd_ = nullptr;
    }
}

static size_t GetFormatSize(const AudioStreamParams& info)
{
    size_t result = 0;
    size_t bitWidthSize = 0;
    switch (info.format) {
        case SAMPLE_U8:
            bitWidthSize = 1; // size is 1
            break;
        case SAMPLE_S16LE:
            bitWidthSize = 2; // size is 2
            break;
        case SAMPLE_S24LE:
            bitWidthSize = 3; // size is 3
            break;
        case SAMPLE_S32LE:
            bitWidthSize = 4; // size is 4
            break;
        default:
            bitWidthSize = 2; // size is 2
            break;
    }
    result = bitWidthSize * info.channels;
    return result;
}

void AudioStream::SetRendererInfo(const AudioRendererInfo &rendererInfo)
{
    rendererInfo_ = rendererInfo;
    SetStreamUsage(rendererInfo.streamUsage);
}

void AudioStream::SetCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    capturerInfo_ = capturerInfo;
}

State AudioStream::GetState()
{
    return state_;
}

int32_t AudioStream::GetAudioSessionID(uint32_t &sessionID)
{
    if ((state_ == RELEASED) || (state_ == NEW)) {
        return ERR_ILLEGAL_STATE;
    }

    if (GetSessionID(sessionID) != 0) {
        return ERR_INVALID_INDEX;
    }

    sessionId_ = sessionID;

    return SUCCESS;
}

bool AudioStream::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    if (state_ == STOPPED) {
        return false;
    }
    uint64_t paTimeStamp = 0;
    if (GetCurrentTimeStamp(paTimeStamp) == SUCCESS) {
        if (resetTime_) {
            AUDIO_INFO_LOG("AudioStream::GetAudioTime resetTime_ %{public}d", resetTime_);
            resetTime_ = false;
            resetTimestamp_ = paTimeStamp;
        }
        if (eMode_ == AUDIO_MODE_PLAYBACK) {
            timestamp.framePosition = GetStreamFramesWritten() * speed_;
        } else {
            timestamp.framePosition = GetStreamFramesRead();
        }

        uint64_t delta = paTimeStamp > resetTimestamp_ ? paTimeStamp - resetTimestamp_ : 0;
        timestamp.time.tv_sec = static_cast<time_t>(delta / TIME_CONVERSION_US_S);
        timestamp.time.tv_nsec
            = static_cast<time_t>((delta - (timestamp.time.tv_sec * TIME_CONVERSION_US_S))
            * TIME_CONVERSION_NS_US);
        timestamp.time.tv_sec += baseTimestamp_.tv_sec;
        timestamp.time.tv_nsec += baseTimestamp_.tv_nsec;
        timestamp.time.tv_sec += (timestamp.time.tv_nsec / TIME_CONVERSION_NS_S);
        timestamp.time.tv_nsec = (timestamp.time.tv_nsec % TIME_CONVERSION_NS_S);

        return true;
    }
    return false;
}

bool AudioStream::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    if (state_ != RUNNING) {
        return false;
    }
    uint64_t framePosition = 0;
    uint64_t timeStamp = 0;
    if (GetCurrentPosition(framePosition, timeStamp) == SUCCESS) {
        timestamp.framePosition = framePosition;
        timestamp.time.tv_sec = static_cast<time_t>(timeStamp / TIME_CONVERSION_NS_S);
        timestamp.time.tv_nsec
            = static_cast<time_t>(timeStamp - (timestamp.time.tv_sec * TIME_CONVERSION_NS_S));
        return true;
    }
    return false;
}

int32_t AudioStream::GetBufferSize(size_t &bufferSize)
{
    AUDIO_DEBUG_LOG("Get Buffer size");
    if (eMode_ == AUDIO_MODE_RECORD) {
        CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Stream state is released");
        return GetBufferSizeForCapturer(bufferSize);
    }

    if (streamParams_.encoding == ENCODING_AUDIOVIVID) {
        CHECK_AND_RETURN_RET(converter_ != nullptr && converter_->GetInputBufferSize(bufferSize),
            ERR_OPERATION_FAILED);
        return SUCCESS;
    }

    int32_t ret = GetMinimumBufferSize(bufferSize);
    CHECK_AND_RETURN_RET(ret == 0, ERR_OPERATION_FAILED);

    return SUCCESS;
}

int32_t AudioStream::GetFrameCount(uint32_t &frameCount)
{
    AUDIO_INFO_LOG("Get frame count");
    if (eMode_ == AUDIO_MODE_RECORD) {
        CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Stream state is released");
        return GetFrameCountForCapturer(frameCount);
    }

    int32_t ret = GetMinimumFrameCount(frameCount);
    CHECK_AND_RETURN_RET(ret == 0, ERR_OPERATION_FAILED);

    return SUCCESS;
}

int32_t AudioStream::GetLatency(uint64_t &latency)
{
    if (GetAudioLatency(latency) != SUCCESS) {
        return ERR_OPERATION_FAILED;
    } else {
        return SUCCESS;
    }
}

vector<AudioSampleFormat> AudioStream::GetSupportedFormats() const
{
    return AUDIO_SUPPORTED_FORMATS;
}

vector<AudioEncodingType> AudioStream::GetSupportedEncodingTypes() const
{
    return AUDIO_SUPPORTED_ENCODING_TYPES;
}

vector<AudioSamplingRate> AudioStream::GetSupportedSamplingRates() const
{
    return AUDIO_SUPPORTED_SAMPLING_RATES;
}

int32_t AudioStream::GetAudioStreamInfo(AudioStreamParams &audioStreamInfo)
{
    AUDIO_INFO_LOG("AudioStream: GetAudioStreamInfo");
    if (GetAudioStreamParams(audioStreamInfo) != 0) {
        return ERR_OPERATION_FAILED;
    }

    return SUCCESS;
}

void AudioStream::RegisterTracker(const std::shared_ptr<AudioClientTracker> &proxyObj)
{
    if (audioStreamTracker_ && audioStreamTracker_.get() && !streamTrackerRegistered_) {
        (void)GetSessionID(sessionId_);
        AUDIO_DEBUG_LOG("Calling register tracker, sessionid = %{public}d", sessionId_);

        AudioRegisterTrackerInfo registerTrackerInfo;

        registerTrackerInfo.sessionId = sessionId_;
        registerTrackerInfo.clientPid = GetClientPid();
        registerTrackerInfo.state = state_;
        registerTrackerInfo.rendererInfo = rendererInfo_;
        registerTrackerInfo.capturerInfo = capturerInfo_;
        registerTrackerInfo.channelCount = streamParams_.channels;
        registerTrackerInfo.appTokenId = GetAppTokenId();

        audioStreamTracker_->RegisterTracker(registerTrackerInfo, proxyObj);
        streamTrackerRegistered_ = true;
    }
}

int32_t AudioStream::SetAudioStreamInfo(const AudioStreamParams info,
    const std::shared_ptr<AudioClientTracker> &proxyObj)
{
    AUDIO_INFO_LOG("AudioStreamInfo, Sampling rate: %{public}d, channels: %{public}d, format: %{public}d,"
        " stream type: %{public}d, encoding type: %{public}d", info.samplingRate, info.channels, info.format,
        eStreamType_, info.encoding);

    if (!IsFormatValid(info.format) || !IsSamplingRateValid(info.samplingRate) || !IsEncodingTypeValid(info.encoding)) {
        AUDIO_ERR_LOG("AudioStream: Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }
    if (state_ != NEW) {
        AUDIO_INFO_LOG("AudioStream: State is not new, release existing stream");
        StopAudioStream();
        ReleaseAudioStream(false);
    }
    AudioStreamParams param = info;
    streamOriginParams_ = info;
    int32_t ret = 0;

    if ((ret = InitFromParams(param)) != SUCCESS) {
        AUDIO_ERR_LOG("InitFromParams error");
        return ret;
    }

    if (CreateStream(param, eStreamType_) != SUCCESS) {
        AUDIO_ERR_LOG("AudioStream:Create stream failed");
        return ERROR;
    }
    state_ = PREPARED;
    proxyObj_ = proxyObj;
    AUDIO_DEBUG_LOG("AudioStream:Set stream Info SUCCESS");
    streamParams_ = param;
    RegisterTracker(proxyObj);
    GetBufferSize(bufferSize_);
    return SUCCESS;
}

bool AudioStream::StartAudioStream(StateChangeCmdType cmdType)
{
    CHECK_AND_RETURN_RET_LOG((state_ == PREPARED) || (state_ == STOPPED) || (state_ == PAUSED),
        false, "Illegal state:%{public}u", state_);
    CHECK_AND_RETURN_RET_LOG(!isPausing_, false,
        "Illegal isPausing_:%{public}u", isPausing_);

    int32_t ret = StartStream(cmdType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "StartStream Start failed:%{public}d", ret);

    resetTime_ = true;
    int32_t retCode = clock_gettime(CLOCK_MONOTONIC, &baseTimestamp_);
    if (retCode != 0) {
        AUDIO_ERR_LOG("AudioStream::StartAudioStream get system elapsed time failed: %d", retCode);
    }

    isFirstRead_ = true;
    isFirstWrite_ = true;
    state_ = RUNNING;

    if (renderMode_ == RENDER_MODE_CALLBACK) {
        if (writeThread_ && writeThread_->joinable()) {
            writeThread_->join();
        }
        isReadyToWrite_ = true;
        writeThread_ = std::make_unique<std::thread>(&AudioStream::WriteCbTheadLoop, this);
        pthread_setname_np(writeThread_->native_handle(), "OS_AudioWriteCb");
    } else if (captureMode_ == CAPTURE_MODE_CALLBACK) {
        isReadyToRead_ = true;
        readThread_ = std::make_unique<std::thread>(&AudioStream::ReadCbThreadLoop, this);
        pthread_setname_np(readThread_->native_handle(), "OS_AudioReadCb");
    }

    AUDIO_INFO_LOG("StartAudioStream SUCCESS, sessionId: %{public}d", sessionId_);

    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("AudioStream:Calling Update tracker for Running");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, GetClientPid(), rendererInfo_, capturerInfo_);
    }

    OpenDumpFile();
    return true;
}

void AudioStream::OpenDumpFile()
{
    int32_t dumpMode = system::GetIntParameter("persist.multimedia.audiostream.blenddatadump", 0);
    switch (dumpMode) {
        case BIN_TEST_MODE:
            pfd_ = fopen("/data/local/tmp/audiostream_dump.pcm", "wb+");
            break;
        case JS_TEST_MODE:
            pfd_ = fopen("/data/storage/el2/base/haps/entry/files/audiostream_dump.pcm", "wb+");
            break;
        default:
            break;
    }

    if (dumpMode != 0 && pfd_ == nullptr) {
        AUDIO_ERR_LOG("Error opening pcm test file!");
    }
}

int32_t AudioStream::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead)
{
    CHECK_AND_RETURN_RET_LOG(userSize > 0, ERR_INVALID_PARAM,
        "Invalid userSize:%{public}zu", userSize);

    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE,
        "Read: State is not RUNNNIG. Illegal  state:%{public}u", state_);

    if (isFirstRead_) {
        FlushAudioStream();
        isFirstRead_ = false;
    }

    StreamBuffer stream;
    stream.buffer = &buffer;
    stream.bufferLen = userSize;
    int32_t readLen = ReadStream(stream, isBlockingRead);
    CHECK_AND_RETURN_RET_LOG(readLen >= 0, ERR_INVALID_READ,
        "ReadStream fail,ret:%{public}d", readLen);

    return readLen;
}

int32_t AudioStream::Write(uint8_t *buffer, size_t bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(renderMode_ != RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "Write not supported. RenderMode is callback");

    CHECK_AND_RETURN_RET_LOG((buffer != nullptr) && (bufferSize > 0),
        ERR_INVALID_PARAM, "Invalid buffer size:%{public}zu", bufferSize);

    if (state_ != RUNNING) {
        AUDIO_ERR_LOG("Write: Illegal  state:%{public}u", state_);
        // To allow context switch for APIs running in different thread contexts
        std::this_thread::sleep_for(std::chrono::microseconds(WRITE_RETRY_DELAY_IN_US));
        return ERR_ILLEGAL_STATE;
    }

    if (streamParams_.encoding != ENCODING_PCM) {
        AUDIO_ERR_LOG("Write: Write not supported. encoding doesnot match");
        AUDIO_ERR_LOG("encoding = %{public}d, excepted %{public}d", streamParams_.encoding, ENCODING_PCM);
        return ERR_NOT_SUPPORTED;
    }

    WriteMuteDataSysEvent(buffer, bufferSize);

    int32_t writeError;
    StreamBuffer stream;
    stream.buffer = buffer;
    stream.bufferLen = bufferSize;

    isFirstWrite_ = isFirstWrite_ ? !offloadEnable_ :isFirstWrite_;
    if (isFirstWrite_) {
        int32_t ret = RenderPrebuf(stream.bufferLen);
        CHECK_AND_RETURN_RET_LOG(!ret, ERR_WRITE_FAILED, "ERR_WRITE_FAILED");
        isFirstWrite_ = false;
    }

    ProcessDataByAudioBlend(buffer, bufferSize);
    ProcessDataByVolumeRamp(buffer, bufferSize);
    size_t bytesWritten = WriteStream(stream, writeError);
    CHECK_AND_RETURN_RET_LOG(writeError == 0, ERR_WRITE_FAILED,
        "WriteStream fail,writeError:%{public}d", writeError);
    return bytesWritten;
}

int32_t AudioStream::Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize)
{
    CHECK_AND_RETURN_RET_LOG(renderMode_ != RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "Write not supported. RenderMode is callback");

    if (state_ != RUNNING) {
        AUDIO_ERR_LOG("Write: Illegal state:%{public}u", state_);
        // To allow context switch for APIs running in different thread contexts
        std::this_thread::sleep_for(std::chrono::microseconds(WRITE_RETRY_DELAY_IN_US));
        return ERR_ILLEGAL_STATE;
    }

    if (streamParams_.encoding != ENCODING_AUDIOVIVID) {
        AUDIO_ERR_LOG("Write: Write not supported. encoding doesnot match");
        AUDIO_ERR_LOG("encoding = %{public}d, excepted %{public}d", streamParams_.encoding, ENCODING_AUDIOVIVID);
        return ERR_NOT_SUPPORTED;
    }

    BufferDesc pcmDesc = {pcmBuffer, pcmBufferSize};
    BufferDesc metaDesc = {metaBuffer, metaBufferSize};

    CHECK_AND_RETURN_RET_LOG(converter_ != nullptr, ERR_WRITE_FAILED,
        "Write: converter isn't init.");

    bool ret = converter_->CheckInputValid(pcmDesc, metaDesc);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_PARAM, "Write: Invalid input.");

    int32_t writeError;
    StreamBuffer stream;

    converter_->Process(pcmDesc, metaDesc);

    converter_->GetOutputBufferStream(stream.buffer, stream.bufferLen);

    if (isFirstWrite_) {
        int32_t res = RenderPrebuf(stream.bufferLen);
        CHECK_AND_RETURN_RET_LOG(!res, ERR_WRITE_FAILED, "ERR_WRITE_FAILED");
        isFirstWrite_ = false;
    }

    ProcessDataByVolumeRamp(stream.buffer, stream.bufferLen);

    size_t bytesWritten = WriteStream(stream, writeError);
    stream.buffer += bytesWritten;
    stream.bufferLen -= bytesWritten;
    bytesWritten += WriteStream(stream, writeError);
    CHECK_AND_RETURN_RET_LOG(writeError == 0, ERR_WRITE_FAILED,
        "WriteStream fail,writeError:%{public}d", writeError);
    return bytesWritten;
}

bool AudioStream::PauseAudioStream(StateChangeCmdType cmdType)
{
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, false,
        "State is not RUNNING. Illegal state:%{public}u", state_);
    State oldState = state_;
    // Update state to stop write thread
    state_ = PAUSED;
    isPausing_ = true;

    if (captureMode_ == CAPTURE_MODE_CALLBACK) {
        isReadyToRead_ = false;
        if (readThread_ && readThread_->joinable()) {
            readThread_->join();
        }
    }

    // Ends the WriteCb thread
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        isReadyToWrite_ = false;
        CheckOffloadBreakWaitWrite();
        // wake write thread to make pause faster
        bufferQueueCV_.notify_all();
        if (writeThread_ && writeThread_->joinable()) {
            writeThread_->join();
        }
    }

    AUDIO_DEBUG_LOG("renderMode_ : %{public}d state_: %{public}d", renderMode_, state_);
    int32_t ret = PauseStream(cmdType);
    if (ret != SUCCESS) {
        AUDIO_DEBUG_LOG("StreamPause fail,ret:%{public}d", ret);
        state_ = oldState;
        isPausing_ = false;
        return false;
    }

    AUDIO_INFO_LOG("PauseAudioStream SUCCESS, sessionId: %{public}d", sessionId_);

    // flush stream after stream paused
    if (!offloadEnable_) {
        FlushAudioStream();
    }

    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("Calling Update tracker for Pause");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, GetClientPid(), rendererInfo_, capturerInfo_);
    }
    isPausing_ = false;
    return true;
}

bool AudioStream::StopAudioStream()
{
    AUDIO_INFO_LOG("begin StopAudioStream for sessionId %{public}d", sessionId_);
    CHECK_AND_RETURN_RET_LOG((state_ == RUNNING) || (state_ == PAUSED), false,
        "State is not RUNNING. Illegal state:%{public}u", state_);
    startMuteTime_ = 0;
    State oldState = state_;
    state_ = STOPPED; // Set it before stopping as Read/Write and Stop can be called from different threads

    if (captureMode_ == CAPTURE_MODE_CALLBACK) {
        isReadyToRead_ = false;
        if (readThread_ && readThread_->joinable()) {
            readThread_->join();
        }
    }

    if (renderMode_ == RENDER_MODE_CALLBACK) {
        isReadyToWrite_ = false;
        CheckOffloadBreakWaitWrite();
        if (writeThread_ && writeThread_->joinable()) {
            writeThread_->join();
        }
    }

    int32_t ret = StopStream();
    if (ret != SUCCESS) {
        AUDIO_DEBUG_LOG("StreamStop fail,ret:%{public}d", ret);
        state_ = oldState;
        return false;
    }

    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("AudioStream:Calling Update tracker for stop");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, GetClientPid(), rendererInfo_, capturerInfo_);
    }

    if (pfd_ != nullptr) {
        fclose(pfd_);
        pfd_ = nullptr;
    }

    return true;
}

bool AudioStream::FlushAudioStream()
{
    Trace trace("AudioStream::FlushAudioStream");
    CHECK_AND_RETURN_RET_LOG((state_ == RUNNING) || (state_ == PAUSED) || (state_ == STOPPED),
        false, "State is not RUNNING. Illegal state:%{public}u", state_);

    if (converter_ != nullptr && streamParams_.encoding == ENCODING_AUDIOVIVID) {
        CHECK_AND_RETURN_RET_LOG(converter_->Flush(), false, "Flush stream fail, failed in converter");
    }

    int32_t ret = FlushStream();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Flush stream fail,ret:%{public}d", ret);

    AUDIO_DEBUG_LOG("Flush stream SUCCESS, sessionId: %{public}d", sessionId_);
    return true;
}

bool AudioStream::DrainAudioStream()
{
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, false,
        "State is not RUNNING. Illegal  state:%{public}u", state_);

    int32_t ret = DrainStream();
    if (ret != SUCCESS) {
        AUDIO_DEBUG_LOG("Drain stream fail,ret:%{public}d", ret);
        return false;
    }

    AUDIO_INFO_LOG("Drain stream SUCCESS");
    return true;
}

bool AudioStream::ReleaseAudioStream(bool releaseRunner)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED && state_ != NEW,
        false, "Illegal state: state = %{public}u", state_);
    // If state_ is RUNNING try to Stop it first and Release
    if (state_ == RUNNING) {
        StopAudioStream();
    }

    ReleaseStream(releaseRunner);
    state_ = RELEASED;
    AUDIO_INFO_LOG("ReleaseAudiostream SUCCESS, sessionId: %{public}d", sessionId_);

    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("AudioStream:Calling Update tracker for release");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, GetClientPid(), rendererInfo_, capturerInfo_);
    }

    audioSpeed_.reset();
    audioSpeed_ = nullptr;
    return true;
}

int32_t AudioStream::SetAudioStreamType(AudioStreamType audioStreamType)
{
    return SetStreamType(audioStreamType);
}

int32_t AudioStream::SetVolume(float volume)
{
    if (volumeRamp_.IsActive()) {
        volumeRamp_.Terminate();
    }
    return SetStreamVolume(volume);
}

float AudioStream::GetVolume()
{
    return GetStreamVolume();
}

int32_t AudioStream::SetRenderRate(AudioRendererRate renderRate)
{
    return SetStreamRenderRate(renderRate);
}

AudioRendererRate AudioStream::GetRenderRate()
{
    return GetStreamRenderRate();
}

int32_t AudioStream::SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "failed. callback == nullptr");

    SaveStreamCallback(callback);

    return SUCCESS;
}

void AudioStream::SetPreferredFrameSize(int32_t frameSize)
{
    AUDIO_INFO_LOG("Not Supported Yet");
}

int32_t AudioStream::SetRenderMode(AudioRenderMode renderMode)
{
    int32_t ret = SetAudioRenderMode(renderMode);
    CHECK_AND_RETURN_RET_LOG(!ret, ERR_OPERATION_FAILED, "renderMode: %{public}d failed", renderMode);
    renderMode_ = renderMode;

    lock_guard<mutex> lock(bufferQueueLock_);

    for (int32_t i = 0; i < MAX_WRITECB_NUM_BUFFERS; ++i) {
        size_t length;
        GetMinimumBufferSize(length);
        AUDIO_INFO_LOG("AudioServiceClient:: GetMinimumBufferSize: %{public}zu", length);

        writeBufferPool_[i] = std::make_unique<uint8_t[]>(max(length,
            (size_t)(0.2 * GetFormatSize(streamParams_) * streamParams_.samplingRate))); // 0.2: 200ms is init size
        CHECK_AND_RETURN_RET_LOG(writeBufferPool_[i] != nullptr, ERR_OPERATION_FAILED,
            "AudioServiceClient::GetBufferDescriptor writeBufferPool_[i]==nullptr. Allocate memory failed.");

        BufferDesc bufDesc {};
        bufDesc.buffer = writeBufferPool_[i].get();
        bufDesc.bufLength = length;
        freeBufferQ_.emplace(bufDesc);
    }

    return SUCCESS;
}

AudioRenderMode AudioStream::GetRenderMode()
{
    return GetAudioRenderMode();
}

int32_t AudioStream::SetCaptureMode(AudioCaptureMode captureMode)
{
    int32_t ret = SetAudioCaptureMode(captureMode);
    CHECK_AND_RETURN_RET_LOG(!ret, ERR_OPERATION_FAILED, "captureMode: %{public}d failed", captureMode);
    captureMode_ = captureMode;

    lock_guard<mutex> lock(bufferQueueLock_);

    for (int32_t i = 0; i < MAX_READCB_NUM_BUFFERS; ++i) {
        size_t length;
        GetMinimumBufferSize(length);
        AUDIO_INFO_LOG("length %{public}zu", length);

        readBufferPool_[i] = std::make_unique<uint8_t[]>(length);
        CHECK_AND_RETURN_RET_LOG(readBufferPool_[i] != nullptr, ERR_OPERATION_FAILED,
            "readBufferPool_[i]==nullptr. Allocate memory failed.");

        BufferDesc bufDesc {};
        bufDesc.buffer = readBufferPool_[i].get();
        bufDesc.bufLength = length;
        freeBufferQ_.emplace(bufDesc);
    }

    return SUCCESS;
}

AudioCaptureMode AudioStream::GetCaptureMode()
{
    return GetAudioCaptureMode();
}

int32_t AudioStream::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(renderMode_ == RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "not supported. Render mode is not callback.");

    CHECK_AND_RETURN_RET_LOG(callback, ERR_INVALID_PARAM,
        "SetRendererWriteCallback callback is nullptr");
    return AudioServiceClient::SetRendererWriteCallback(callback);
}

int32_t AudioStream::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(captureMode_ == CAPTURE_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "not supported. Capture mode is not callback.");

    CHECK_AND_RETURN_RET_LOG(callback, ERR_INVALID_PARAM, "callback is nullptr");
    return AudioServiceClient::SetCapturerReadCallback(callback);
}

int32_t AudioStream::GetBufferDesc(BufferDesc &bufDesc)
{
    CHECK_AND_RETURN_RET_LOG((renderMode_ == RENDER_MODE_CALLBACK) || (captureMode_ == CAPTURE_MODE_CALLBACK),
        ERR_INCORRECT_MODE, "not supported. Render or Capture mode is not callback.");

    lock_guard<mutex> lock(bufferQueueLock_);

    if (renderMode_ == RENDER_MODE_CALLBACK) {
        if (!freeBufferQ_.empty()) {
            bufDesc.buffer = freeBufferQ_.front().buffer;
            bufDesc.bufLength = freeBufferQ_.front().bufLength;
            bufDesc.dataLength = freeBufferQ_.front().dataLength;
            freeBufferQ_.pop();
        } else {
            bufDesc.buffer = nullptr;
            AUDIO_ERR_LOG("freeBufferQ_.empty()");
            return ERR_OPERATION_FAILED;
        }
    }

    if (captureMode_ == CAPTURE_MODE_CALLBACK) {
        if (!filledBufferQ_.empty()) {
            bufDesc.buffer = filledBufferQ_.front().buffer;
            bufDesc.bufLength = filledBufferQ_.front().bufLength;
            bufDesc.dataLength = filledBufferQ_.front().dataLength;
            filledBufferQ_.pop();
        } else {
            bufDesc.buffer = nullptr;
            AUDIO_ERR_LOG("filledBufferQ_.empty()");
            return ERR_OPERATION_FAILED;
        }
    }
    return SUCCESS;
}

int32_t AudioStream::GetBufQueueState(BufferQueueState &bufState)
{
    CHECK_AND_RETURN_RET_LOG((renderMode_ == RENDER_MODE_CALLBACK) || (captureMode_ == CAPTURE_MODE_CALLBACK),
        ERR_INCORRECT_MODE, "not supported. Render or Capture mode is not callback.");

    lock_guard<mutex> lock(bufferQueueLock_);

    if (renderMode_ == RENDER_MODE_CALLBACK) {
        bufState.numBuffers = filledBufferQ_.size();
    }

    if (captureMode_ == CAPTURE_MODE_CALLBACK) {
        bufState.numBuffers = freeBufferQ_.size();
    }

    return SUCCESS;
}

int32_t AudioStream::Enqueue(const BufferDesc &bufDesc)
{
    CHECK_AND_RETURN_RET_LOG((renderMode_ == RENDER_MODE_CALLBACK) || (captureMode_ == CAPTURE_MODE_CALLBACK),
        ERR_INCORRECT_MODE, "not supported. Render or Capture mode is not callback.");

    CHECK_AND_RETURN_RET_LOG(bufDesc.buffer != nullptr, ERR_INVALID_PARAM,
        "failed. bufDesc.buffer == nullptr.");

    unique_lock<mutex> lock(bufferQueueLock_);

    if (renderMode_ == RENDER_MODE_CALLBACK) {
        AUDIO_DEBUG_LOG("AudioStream::Enqueue: filledBuffer length: %{public}zu.", bufDesc.bufLength);
        filledBufferQ_.emplace(bufDesc);
    }

    if (captureMode_ == CAPTURE_MODE_CALLBACK) {
        AUDIO_DEBUG_LOG("AudioStream::Enqueue: freeBuffer length: %{public}zu.", bufDesc.bufLength);
        freeBufferQ_.emplace(bufDesc);
    }

    bufferQueueCV_.notify_all();

    return SUCCESS;
}

int32_t AudioStream::Clear()
{
    CHECK_AND_RETURN_RET_LOG((renderMode_ == RENDER_MODE_CALLBACK) || (captureMode_ == CAPTURE_MODE_CALLBACK),
        ERR_INCORRECT_MODE, "not supported. Render or Capture mode is not callback.");

    lock_guard<mutex> lock(bufferQueueLock_);

    while (!filledBufferQ_.empty()) {
        freeBufferQ_.emplace(filledBufferQ_.front());
        filledBufferQ_.pop();
    }

    return SUCCESS;
}

int32_t AudioStream::GetStreamBufferCB(StreamBuffer &stream,
    std::unique_ptr<uint8_t[]> &speedBuffer, int32_t &speedBufferSize)
{
    stream.buffer = filledBufferQ_.front().buffer;
    stream.bufferLen = filledBufferQ_.front().bufLength;

    if (stream.buffer == nullptr) {
        AUDIO_ERR_LOG("WriteCb stream.buffer is nullptr return");
        return ERR_INVALID_PARAM;
    }
    if (!isEqual(speed_, 1.0f)) {
        int32_t ret = audioSpeed_->ChangeSpeedFunc(stream.buffer, stream.bufferLen, speedBuffer, speedBufferSize);
        if (ret == 0 || speedBufferSize == 0) {
            freeBufferQ_.emplace(filledBufferQ_.front());
            filledBufferQ_.pop();
            SendWriteBufferRequestEvent();
            return ERR_INVALID_WRITE; // Continue writing when the sonic is not full
        }
        stream.buffer = speedBuffer.get();
        stream.bufferLen = speedBufferSize;
        return SUCCESS;
    }
    return SUCCESS;
}

void AudioStream::WriteCbTheadLoop()
{
    AUDIO_INFO_LOG("WriteCb thread start");
    StreamBuffer stream;
    std::unique_ptr<uint8_t[]> speedBuffer = std::make_unique<uint8_t[]>(MAX_BUFFER_SIZE);
    int32_t speedBufferSize;
    size_t bytesWritten;
    int32_t writeError;

    if (isReadyToWrite_) {
        // send write events for application to fill all free buffers at the beginning
        SubmitAllFreeBuffers();

        while (true) {
            if (state_ != RUNNING) {
                AUDIO_INFO_LOG("Write: not running state: %{public}u", state_);
                isReadyToWrite_ = false;
                break;
            }

            unique_lock<mutex> lock(bufferQueueLock_);

            if (filledBufferQ_.empty()) {
                // wait signal with timeout
                bufferQueueCV_.wait_for(lock, chrono::milliseconds(CB_WRITE_BUFFERS_WAIT_IN_MS));
                continue;
            }

            int32_t ret = GetStreamBufferCB(stream, speedBuffer, speedBufferSize);
            if (ret == ERR_INVALID_PARAM) break;
            if (ret == ERR_INVALID_WRITE) continue; // Continue writing when the sonic is not full

            ProcessDataByAudioBlend(stream.buffer, stream.bufferLen);
            ProcessDataByVolumeRamp(stream.buffer, stream.bufferLen);

            bytesWritten = WriteStreamInCb(stream, writeError);
            if (writeError != 0) {
                AUDIO_ERR_LOG("WriteStreamInCb fail, writeError:%{public}d", writeError);
            } else {
                AUDIO_DEBUG_LOG("WriteCb WriteStream, bytesWritten:%{public}zu", bytesWritten);
            }

            freeBufferQ_.emplace(filledBufferQ_.front());
            filledBufferQ_.pop();
            SendWriteBufferRequestEvent();
        }
    }
}

void AudioStream::ReadCbThreadLoop()
{
    AUDIO_INFO_LOG("ReadCb thread start");
    StreamBuffer stream;
    if (isReadyToRead_) {
        int32_t readLen;
        bool isBlockingRead = true;
        while (true) {
            if (state_ != RUNNING) {
                AUDIO_INFO_LOG("Read: not running state: %{public}u", state_);
                isReadyToRead_ = false;
                break;
            }

            unique_lock<mutex> lock(bufferQueueLock_);

            if (freeBufferQ_.empty()) {
                // wait signal with timeout
                bufferQueueCV_.wait_for(lock, chrono::milliseconds(CB_READ_BUFFERS_WAIT_IN_MS));
                continue;
            }
            stream.buffer = freeBufferQ_.front().buffer;
            stream.bufferLen = freeBufferQ_.front().bufLength;

            CHECK_AND_BREAK_LOG(stream.buffer != nullptr, "ReadCb stream.buffer == nullptr");

            readLen = ReadStream(stream, isBlockingRead);
            if (readLen < 0) {
                AUDIO_ERR_LOG("ReadCb ReadStream fail, ret: %{public}d", readLen);
            } else {
                AUDIO_DEBUG_LOG("ReadCb ReadStream, bytesRead:%{public}d", readLen);
                freeBufferQ_.front().dataLength = readLen;
                filledBufferQ_.emplace(freeBufferQ_.front());
                freeBufferQ_.pop();
                SendReadBufferRequestEvent();
            }
        }
    }
}

int32_t AudioStream::SetLowPowerVolume(float volume)
{
    return SetStreamLowPowerVolume(volume);
}

float AudioStream::GetLowPowerVolume()
{
    return GetStreamLowPowerVolume();
}

int32_t AudioStream::SetOffloadMode(int32_t state, bool isAppBack)
{
    return SetStreamOffloadMode(state, isAppBack);
}

int32_t AudioStream::UnsetOffloadMode()
{
    return UnsetStreamOffloadMode();
}

float AudioStream::GetSingleStreamVolume()
{
    return GetSingleStreamVol();
}

void AudioStream::SubmitAllFreeBuffers()
{
    lock_guard<mutex> lock(bufferQueueLock_);
    for (size_t i = 0; i < freeBufferQ_.size(); ++i) {
        SendWriteBufferRequestEvent();
    }
}

int32_t AudioStream::SetAudioEffectMode(AudioEffectMode effectMode)
{
    return SetStreamAudioEffectMode(effectMode);
}

AudioEffectMode AudioStream::GetAudioEffectMode()
{
    return GetStreamAudioEffectMode();
}

void AudioStream::SetInnerCapturerState(bool isInnerCapturer)
{
    SetStreamInnerCapturerState(isInnerCapturer);
}

void AudioStream::SetPrivacyType(AudioPrivacyType privacyType)
{
    SetStreamPrivacyType(privacyType);
}

int64_t AudioStream::GetFramesWritten()
{
    return GetStreamFramesWritten();
}

int64_t AudioStream::GetFramesRead()
{
    return GetStreamFramesRead();
}

#ifdef SONIC_ENABLE
int32_t AudioStream::ChangeSpeed(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    return audioSpeed_->ChangeSpeedFunc(buffer, bufferSize, outBuffer, outBufferSize);
}

int32_t AudioStream::WriteSpeedBuffer(int32_t bufferSize, uint8_t *speedBuffer, size_t speedBufferSize)
{
    int32_t writeIndex = 0;
    int32_t writeSize = bufferSize_;
    while (speedBufferSize > 0) {
        if (speedBufferSize < bufferSize_) {
            writeSize = speedBufferSize;
        }
        int32_t writtenSize = Write(speedBuffer + writeIndex, writeSize);
        if (writtenSize < 0) return writtenSize;
        if (writtenSize == 0) {
            AUDIO_WARNING_LOG("writeen size is zero");
            continue;
        }
        writeIndex += writtenSize;
        speedBufferSize -= writtenSize;
    }

    return bufferSize;
}
#endif

int32_t AudioStream::SetSpeed(float speed)
{
    if (audioSpeed_ == nullptr) {
        audioSpeed_ = make_unique<AudioSpeed>(streamParams_.samplingRate,
            streamParams_.format, streamParams_.channels);
    }
    audioSpeed_->SetSpeed(speed);
    speed_ = speed;
    return SUCCESS;
}

float AudioStream::GetSpeed()
{
    return speed_;
}

int32_t AudioStream::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    if ((state_ != PREPARED) && (state_ != NEW)) {
        return ERR_ILLEGAL_STATE;
    }
    audioBlend_.SetParams(blendMode, streamParams_.format, streamParams_.channels);
    return SUCCESS;
}

void AudioStream::SetStreamTrackerState(bool trackerRegisteredState)
{
    streamTrackerRegistered_ = trackerRegisteredState;
}

void AudioStream::GetSwitchInfo(SwitchInfo& info)
{
    info.params = streamOriginParams_;

    info.rendererInfo = rendererInfo_;
    info.capturerInfo = capturerInfo_;
    info.eStreamType = eStreamType_;
    info.renderMode = renderMode_;
    info.state = state_;
    info.sessionId = sessionId_;
    info.streamTrackerRegistered = streamTrackerRegistered_;
    GetStreamSwitchInfo(info);
}

void AudioStream::ProcessDataByAudioBlend(uint8_t *buffer, size_t bufferSize)
{
    audioBlend_.Process(buffer, bufferSize);
    if (pfd_ != nullptr) {
        size_t writeResult = fwrite(reinterpret_cast<void *>(buffer), 1, bufferSize, pfd_);
        if (writeResult != bufferSize) {
            AUDIO_ERR_LOG("Failed to write the file.");
        }
    }
}

int32_t AudioStream::SetVolumeWithRamp(float targetVolume, int32_t duration)
{
    CHECK_AND_RETURN_RET((state_ != RELEASED) && (state_ != INVALID) && (state_ != STOPPED),
        ERR_ILLEGAL_STATE);

    float currStreamVol = GetVolume();
    CHECK_AND_RETURN_RET(currStreamVol != targetVolume, SUCCESS);

    volumeRamp_.SetVolumeRampConfig(targetVolume, currStreamVol, duration);
    return SUCCESS;
}

void AudioStream::ProcessDataByVolumeRamp(uint8_t *buffer, size_t bufferSize)
{
    if (volumeRamp_.IsActive()) {
        SetStreamVolume(volumeRamp_.GetRampVolume());
    }
}

void AudioStream::WriteMuteDataSysEvent(uint8_t *buffer, size_t bufferSize)
{
    std::string name = gBundleInfo_.name;
    int32_t versionCode = gBundleInfo_.versionCode;
    if (buffer[0] == 0) {
        if (startMuteTime_ == 0) {
            startMuteTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if ((currentTime - startMuteTime_ >= ONE_MINUTE) && !isUpEvent_) {
            AUDIO_WARNING_LOG("write silent data for some time");
            isUpEvent_ = true;
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "BACKGROUND_SILENT_PLAYBACK",
                HiviewDFX::HiSysEvent::EventType::STATISTIC,
                "APP_NAME", name,
                "APP_VERSION_CODE", versionCode);
        }
    } else if (buffer[0] != 0 && startMuteTime_ != 0) {
        startMuteTime_ = 0;
    }
}

int32_t AudioStream::InitFromParams(AudioStreamParams &info)
{
    int32_t ret = 0;
    static std::mutex connectServerMutex;
    std::lock_guard<std::mutex> lockConnect(connectServerMutex);
    Trace trace("AudioStream::Initialize");
    if (eMode_ == AUDIO_MODE_PLAYBACK) {
        AUDIO_DEBUG_LOG("Initialize playback");
        if (!IsPlaybackChannelRelatedInfoValid(info.channels, info.channelLayout)) {
            return ERR_NOT_SUPPORTED;
        }
        ret = Initialize(AUDIO_SERVICE_CLIENT_PLAYBACK);

        if (info.encoding == ENCODING_AUDIOVIVID) {
            converter_ = std::make_unique<AudioFormatConverter3DA>();
            if (converter_ == nullptr ||
                converter_->Init(info) != SUCCESS ||
                !converter_->AllocateMem()) {
                AUDIO_ERR_LOG("AudioStream: converter construct error");
                return ERR_NOT_SUPPORTED;
            }
            converter_->ConverterChannels(info.channels, info.channelLayout);
        }
    } else if (eMode_ == AUDIO_MODE_RECORD) {
        AUDIO_DEBUG_LOG("Initialize recording");
        bool res = IsCapturerChannelValid(info.channels);
        CHECK_AND_RETURN_RET_LOG(res, ERR_NOT_SUPPORTED, "Invalid source channel %{public}d", info.channels);
        ret = Initialize(AUDIO_SERVICE_CLIENT_RECORD);
    } else {
        AUDIO_ERR_LOG("error eMode.");
        return ERR_INVALID_OPERATION;
    }
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "Error initializing!");
    return SUCCESS;
}

int32_t AudioStream::RegisterAudioStreamPolicyServerDiedCb()
{
    AUDIO_DEBUG_LOG("RegisterAudioStreamPolicyServerDiedCb enter");
    CHECK_AND_RETURN_RET_LOG(audioStreamPolicyServiceDiedCB_ == nullptr,
        ERROR, "audioStreamPolicyServiceDiedCB_ existence, do not create duplicate");

    audioStreamPolicyServiceDiedCB_ = std::make_shared<AudioStreamPolicyServiceDiedCallbackImpl>();
    CHECK_AND_RETURN_RET_LOG(audioStreamPolicyServiceDiedCB_ != nullptr,
        ERROR, "create audioStreamPolicyServiceDiedCB_ failed");

    return AudioPolicyManager::GetInstance().RegisterAudioStreamPolicyServerDiedCb(audioStreamPolicyServiceDiedCB_);
}

int32_t AudioStream::UnregisterAudioStreamPolicyServerDiedCb()
{
    AUDIO_DEBUG_LOG("UnregisterAudioStreamPolicyServerDiedCb enter");
    CHECK_AND_RETURN_RET_LOG(audioStreamPolicyServiceDiedCB_ != nullptr,
        ERROR, "audioStreamPolicyServiceDiedCB_ is null");
    return AudioPolicyManager::GetInstance().UnregisterAudioStreamPolicyServerDiedCb(audioStreamPolicyServiceDiedCB_);
}

int32_t AudioStream::RegisterRendererOrCapturerPolicyServiceDiedCB(
    const std::shared_ptr<RendererOrCapturerPolicyServiceDiedCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR, "Callback is null");

    int32_t ret = RegisterAudioStreamPolicyServerDiedCb();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "RegisterAudioStreamPolicyServerDiedCb failed");

    CHECK_AND_RETURN_RET_LOG(audioStreamPolicyServiceDiedCB_ != nullptr,
        ERROR, "audioStreamPolicyServiceDiedCB_ is null");
    audioStreamPolicyServiceDiedCB_->SaveRendererOrCapturerPolicyServiceDiedCB(callback);
    return SUCCESS;
}

int32_t AudioStream::RemoveRendererOrCapturerPolicyServiceDiedCB()
{
    CHECK_AND_RETURN_RET_LOG(audioStreamPolicyServiceDiedCB_ != nullptr,
        ERROR, "audioStreamPolicyServiceDiedCB_ is null");
    audioStreamPolicyServiceDiedCB_->RemoveRendererOrCapturerPolicyServiceDiedCB();

    int32_t ret = UnregisterAudioStreamPolicyServerDiedCb();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "RemoveRendererOrCapturerPolicyServiceDiedCB failed");

    return SUCCESS;
}

bool AudioStream::RestoreAudioStream()
{
    CHECK_AND_RETURN_RET_LOG(proxyObj_ != nullptr, false, "proxyObj_ is null");
    CHECK_AND_RETURN_RET_LOG(state_ != NEW && state_ != INVALID, true,
        "state_ is NEW/INVALID, no need for restore");
    bool result = false;
    State oldState = state_;
    state_ = NEW;
    SetStreamTrackerState(false);

    int32_t ret = SetAudioStreamInfo(streamOriginParams_, proxyObj_);
    if (ret != SUCCESS) {
        goto error;
    }
    switch (oldState) {
        case RUNNING:
            result = StartAudioStream();
            break;
        case PAUSED:
            result = StartAudioStream() && PauseAudioStream();
            break;
        case STOPPED:
        case STOPPING:
            result = StartAudioStream() && StopAudioStream();
            break;
        default:
            break;
    }
    if (!result) {
        goto error;
    }
    return result;

error:
    AUDIO_ERR_LOG("RestoreAudioStream failed");
    state_ = oldState;
    return false;
}

int32_t AudioStream::NotifyCapturerAdded(uint32_t sessionID)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = static_cast<AudioSamplingRate> (streamParams_.samplingRate);
    streamInfo.channels = static_cast<AudioChannel> (streamParams_.channels);
    return AudioPolicyManager::GetInstance().NotifyCapturerAdded(capturerInfo_, streamInfo, sessionID);
}

AudioStreamPolicyServiceDiedCallbackImpl::AudioStreamPolicyServiceDiedCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioStreamPolicyServiceDiedCallbackImpl instance create");
}

AudioStreamPolicyServiceDiedCallbackImpl::~AudioStreamPolicyServiceDiedCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioStreamPolicyServiceDiedCallbackImpl instance destory");
}

void AudioStreamPolicyServiceDiedCallbackImpl::OnAudioPolicyServiceDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("AudioStreamPolicyServiceDiedCallbackImpl OnAudioPolicyServiceDied");
    if (policyServiceDiedCallback_ != nullptr) {
        policyServiceDiedCallback_->OnAudioPolicyServiceDied();
    }
}

void AudioStreamPolicyServiceDiedCallbackImpl::SaveRendererOrCapturerPolicyServiceDiedCB(
    const std::shared_ptr<RendererOrCapturerPolicyServiceDiedCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback != nullptr) {
        policyServiceDiedCallback_ = callback;
    }
}

void AudioStreamPolicyServiceDiedCallbackImpl::RemoveRendererOrCapturerPolicyServiceDiedCB()
{
    std::lock_guard<std::mutex> lock(mutex_);
    policyServiceDiedCallback_ = nullptr;
}
} // namespace AudioStandard
} // namespace OHOS
