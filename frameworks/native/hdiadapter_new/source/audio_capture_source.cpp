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

#ifndef LOG_TAG
#define LOG_TAG "AudioCaptureSource"
#endif

#include "source/audio_capture_source.h"
#include <climits>
#include <future>
#include "parameters.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "audio_schedule.h"
#include "media_monitor_manager.h"
#include "audio_enhance_chain_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "manager/hdi_monitor.h"
#include "capturer_clock_manager.h"
#include "audio_setting_provider.h"

namespace OHOS {
namespace AudioStandard {

static constexpr uint32_t DECIMAL_BASE = 10;

AudioCaptureSource::AudioCaptureSource(const uint32_t captureId, const std::string &halName)
    : captureId_(captureId), halName_(halName)
{
    audioSrcClock_ = std::make_shared<AudioCapturerSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(captureId, audioSrcClock_);
}

AudioCaptureSource::~AudioCaptureSource()
{
    isCaptureThreadRunning_ = false;
    AUDIO_INFO_LOG("[%{public}s] volumeDataCount: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
    CapturerClockManager::GetInstance().DeleteAudioSourceClock(captureId_);
}

int32_t AudioCaptureSource::Init(const IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(statusMutex_);
    if (attr.sourceType == SOURCE_TYPE_MIC_REF || attr.sourceType == SOURCE_TYPE_EC) {
        InitEcOrMicRefAttr(attr);
    } else {
        attr_ = attr;
    }
    adapterNameCase_ = attr_.adapterName;
    if (adapterNameCase_ == "" && halName_ == "primary") {
        adapterNameCase_ = "primary";
    }
    openMic_ = attr_.openMicSpeaker;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);

    int32_t ret = InitCapture();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    if (muteState_.load() && IsFormalSourceType(attr_.sourceType)) {
        SetMute(true);
    }
    sourceInited_ = true;
    if ((attr.sourceType == SOURCE_TYPE_MIC_REF || attr.sourceType == SOURCE_TYPE_EC) &&
        IsNonblockingSource(attr.adapterName)) {
        ringBufferHandler_ = std::make_shared<RingBufferHandler>();
        ringBufferHandler_->Init(attr.sampleRate, attr.channel, GetByteSizeByFormat(attr.format));
    }

    if (audioSrcClock_ != nullptr) {
        audioSrcClock_->Init(attr.sampleRate, attr.format, attr.channel);
    }

    return SUCCESS;
}

void AudioCaptureSource::DeInit(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    Trace trace("AudioCaptureSource::DeInit");
    AudioXCollie audioXCollie("AudioCaptureSource::DeInit", TIMEOUT_SECONDS_5,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);

    AUDIO_INFO_LOG("halName: %{public}s, sourceType: %{public}d", halName_.c_str(), attr_.sourceType);
    sourceInited_ = false;
    started_.store(false);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN(deviceManager != nullptr);
    captureInited_ = false;
    deviceManager->DestroyCapture(adapterNameCase_, hdiCaptureId_);
    audioCapture_ = nullptr;

    IAudioSourceCallback *callback = nullptr;
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback = &callback_;
    }
    if (callback != nullptr) {
        callback->OnWakeupClose();
    }
    currentActiveDevice_ = DEVICE_TYPE_INVALID;
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

bool AudioCaptureSource::IsInited(void)
{
    return sourceInited_;
}

void AudioCaptureSource::InitRunningLock(void)
{
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        switch (attr_.sourceType) {
            case SOURCE_TYPE_WAKEUP:
                runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME_WAKEUP));
                break;
            case SOURCE_TYPE_MIC:
            case SOURCE_TYPE_CAMCORDER:
            case SOURCE_TYPE_UNPROCESSED:
            case SOURCE_TYPE_LIVE:
            default:
                runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME));
        }
        guard.CheckCurrTimeout();
    }
    if (runningLock_ != nullptr) {
        runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    } else {
        AUDIO_ERR_LOG("running lock is null, playback can not work well");
    }
#endif
}

int32_t AudioCaptureSource::Start(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("halName: %{public}s, sourceType: %{public}d", halName_.c_str(), attr_.sourceType);
    Trace trace("AudioCaptureSource::Start");
    if (audioSrcClock_ != nullptr) {
        audioSrcClock_->Reset();
    }

    if (IsNonblockingSource(adapterNameCase_)) {
        return NonblockingStart();
    }

    InitLatencyMeasurement();
    InitRunningLock();

    // eg: primary_source_0_20240527202236189_44100_2_1.pcm
    dumpFileName_ = halName_ + "_source_" + std::to_string(attr_.sourceType) + "_" + GetTime() + "_" +
        std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel) + "_" +
        std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
    logUtilsTag_ = "AudioSource";

    if (started_.load()) {
        return SUCCESS;
    }
    callback_.OnCaptureState(true);
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Start(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail");
    started_.store(true);

    if (halName_ == HDI_ID_INFO_ACCESSORY && dmDeviceTypeMap_[DEVICE_TYPE_ACCESSORY] == DM_DEVICE_TYPE_PENCIL) {
        SetAccessoryDeviceState(true);
    }

    return SUCCESS;
}

int32_t AudioCaptureSource::Stop(void)
{
    Trace trace("AudioCaptureSource::Stop");
    std::promise<void> promiseEnsureLock;
    auto futurePromiseEnsureLock = promiseEnsureLock.get_future();
    std::thread stopThread([&promiseEnsureLock, this] {
        std::lock_guard<std::mutex> lock(statusMutex_);
        promiseEnsureLock.set_value();
        DoStop();
    });
    futurePromiseEnsureLock.get();
    stopThread.detach();

    if (halName_ == HDI_ID_INFO_ACCESSORY && dmDeviceTypeMap_[DEVICE_TYPE_ACCESSORY] == DM_DEVICE_TYPE_PENCIL) {
        SetAccessoryDeviceState(false);
    }

    return SUCCESS;
}

int32_t AudioCaptureSource::Resume(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    Trace trace("AudioCaptureSource::Resume");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    if (!paused_) {
        return SUCCESS;
    }
    int32_t ret = audioCapture_->Resume(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail");
    paused_ = false;
    return SUCCESS;
}

int32_t AudioCaptureSource::Pause(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("halName: %{public}s, sourceType: %{public}d", halName_.c_str(), attr_.sourceType);
    Trace trace("AudioCaptureSource::Pause");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioCapture_->Pause(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail");
    paused_ = true;
    return SUCCESS;
}

int32_t AudioCaptureSource::Flush(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("halName: %{public}s, sourceType: %{public}d", halName_.c_str(), attr_.sourceType);
    Trace trace("AudioCaptureSource::Flush");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioCapture_->Flush(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    return SUCCESS;
}

int32_t AudioCaptureSource::Reset(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("halName: %{public}s, sourceType: %{public}d", halName_.c_str(), attr_.sourceType);
    Trace trace("AudioCaptureSource::Reset");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioCapture_->Flush(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail");
    return SUCCESS;
}

static uint64_t GetFirstTimeStampFromAlgo(const std::string &adapterNameCase)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, 0, "GetDeviceManager fail!");

    AudioParamKey key = NONE;
    std::string value = deviceManager->GetAudioParameter(adapterNameCase, key, "record_algo_first_ts");
    CHECK_AND_RETURN_RET_LOG(value != "", 0, "record_algo_first_ts fail!");

    uint64_t firstTimeStamp = std::strtoull(value.c_str(), nullptr, DECIMAL_BASE);
    AUDIO_INFO_LOG("record_algo_first_ts:%{public}" PRIu64, firstTimeStamp);
    return firstTimeStamp;
}

int32_t AudioCaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    if (!started_.load()) {
        AUDIO_WARNING_LOG("not start, invalid state");
        return ERR_ILLEGAL_STATE;
    }

    Trace trace("AudioCaptureSource::CaptureFrame");
    AudioCapturerSourceTsRecorder recorder(replyBytes, audioSrcClock_);

    // only mic ref
    if (attr_.sourceType == SOURCE_TYPE_MIC_REF) {
        if (ringBufferHandler_ != nullptr) {
            Trace micRefTrace("AudioCaptureSource::CaptureFrame::micRef");
            int32_t ret = ringBufferHandler_->ReadDataFromRingBuffer(reinterpret_cast<uint8_t *>(frame), requestBytes);
            if (ret == SUCCESS) {
                replyBytes = requestBytes;
            } else {
                AUDIO_ERR_LOG("read data from ring buffer fail");
                replyBytes = 0;
            }
        }
        return SUCCESS;
    }

    int64_t stamp = ClockTime::GetCurNano();
    uint32_t frameLen = static_cast<uint32_t>(requestBytes);
    int32_t ret = audioCapture_->CaptureFrame(audioCapture_, reinterpret_cast<int8_t *>(frame), &frameLen, &replyBytes);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_READ_FAILED, "fail, ret: %{public}x", ret);
    if (audioSrcClock_ != nullptr && audioSrcClock_->GetFrameCnt() == 0) {
        audioSrcClock_->SetFirstTimestampFromHdi(GetFirstTimeStampFromAlgo(adapterNameCase_));
    }
    CheckLatencySignal(reinterpret_cast<uint8_t *>(frame), replyBytes);

    DumpData(frame, replyBytes);
    CheckUpdateState(frame, requestBytes);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    int64_t stampThreshold = 50; // 50ms
    if (logMode_ || stamp >= stampThreshold) {
        AUDIO_WARNING_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", requestBytes, stamp);
    }
    return SUCCESS;
}

int32_t AudioCaptureSource::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
    uint64_t &replyBytesEc)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    if (attr_.sourceType != SOURCE_TYPE_EC) {
        CHECK_AND_RETURN_RET_LOG(fdesc != nullptr && fdesc->frame != nullptr && fdescEc != nullptr &&
            fdescEc->frame != nullptr, ERR_INVALID_PARAM, "desc frame is nullptr");
    } else { // only check ec frame
        CHECK_AND_RETURN_RET_LOG(fdescEc != nullptr && fdescEc->frame != nullptr, ERR_INVALID_PARAM,
            "desc frame is nullptr");
    }

    if (IsNonblockingSource(adapterNameCase_)) {
        return NonblockingCaptureFrameWithEc(fdescEc, replyBytesEc);
    }

    struct AudioFrameLen frameLen = { fdesc->frameLen, fdescEc->frameLen };
    struct AudioCaptureFrameInfo frameInfo = {};
    int32_t ret = audioCapture_->CaptureFrameEc(audioCapture_, &frameLen, &frameInfo);
    if (ret < 0) {
        AUDIO_ERR_LOG("fail, ret: %{public}x", ret);
        AudioCaptureFrameInfoFree(&frameInfo, false);
        return ERR_READ_FAILED;
    }

    if (attr_.sourceType != SOURCE_TYPE_EC && frameInfo.frame != nullptr) {
        if (frameInfo.replyBytes - fdescEc->frameLen < fdesc->frameLen) {
            replyBytes = 0;
            return ERR_INVALID_READ;
        }
        if (memcpy_s(fdesc->frame, fdesc->frameLen, frameInfo.frame, fdesc->frameLen) != EOK) {
            AUDIO_ERR_LOG("copy desc fail");
        } else {
            replyBytes = (attr_.sourceType == SOURCE_TYPE_EC) ? 0 : fdesc->frameLen;
            DumpData(fdesc->frame, replyBytes);
        }
    }
    if (frameInfo.frameEc != nullptr) {
        if (memcpy_s(fdescEc->frame, fdescEc->frameLen, frameInfo.frameEc, fdescEc->frameLen) != EOK) {
            AUDIO_ERR_LOG("copy desc ec fail");
        } else {
            replyBytesEc = (attr_.sourceType == SOURCE_TYPE_EC) ? frameInfo.replyBytesEc : fdescEc->frameLen;
        }
    }
    CheckUpdateState(fdesc->frame, replyBytes);
    AudioCaptureFrameInfoFree(&frameInfo, false);

    return SUCCESS;
}

std::string AudioCaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t AudioCaptureSource::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    leftVolume_ = left;
    rightVolume_ = right;
    float volume;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    int32_t ret = audioCapture_->SetVolume(audioCapture_, volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }

    return ret;
}

int32_t AudioCaptureSource::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    float val = 0.0;
    audioCapture_->GetVolume(audioCapture_, &val);
    left = val;
    right = val;
    return SUCCESS;
}

int32_t AudioCaptureSource::SetMute(bool isMute)
{
    AUDIO_INFO_LOG("halName: %{public}s, isMute: %{public}d", halName_.c_str(), isMute);

    muteState_.store(isMute);
    if (audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->SetMute(audioCapture_, isMute);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("set mute fail");
        } else {
            AUDIO_INFO_LOG("set mute succ");
        }
    }

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    if (deviceManager != nullptr) {
        deviceManager->SetMicMute(adapterNameCase_, isMute);
    }
    AUDIO_INFO_LOG("end");
    return SUCCESS;
}

int32_t AudioCaptureSource::GetMute(bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    bool hdiMuteState = false;
    int32_t ret = audioCapture_->GetMute(audioCapture_, &hdiMuteState);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("get mute fail");
    }
    AUDIO_DEBUG_LOG("hdiMuteState: %{public}d, muteState: %{public}d", hdiMuteState, muteState_.load());
    isMute = muteState_.load();
    return SUCCESS;
}

uint64_t AudioCaptureSource::GetTransactionId(void)
{
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t AudioCaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");

    struct AudioTimeStamp stamp = {};
    int32_t ret = audioCapture_->GetCapturePosition(audioCapture_, &frames, &stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get capture position fail, ret: %{public}d", ret);
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;
    return ret;
}

float AudioCaptureSource::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t AudioCaptureSource::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");
    AUDIO_INFO_LOG("scene: %{public}d, current scene : %{public}d, scoExcludeFlag: %{public}d",
        audioScene, currentAudioScene_, scoExcludeFlag);

    if (audioScene != currentAudioScene_ && !scoExcludeFlag) {
        struct AudioSceneDescriptor sceneDesc;
        InitSceneDesc(sceneDesc, audioScene);

        CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
        int32_t ret = audioCapture_->SelectScene(audioCapture_, &sceneDesc);
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
    }
    if (audioScene != currentAudioScene_) {
        currentAudioScene_ = audioScene;
    }
    return SUCCESS;
}

int32_t AudioCaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    return UpdateActiveDeviceWithoutLock(inputDevice);
}

int32_t AudioCaptureSource::UpdateSourceType(SourceType sourceType)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    if (attr_.sourceType == sourceType) {
        AUDIO_INFO_LOG("not change, currentActiveDevice: %{public}d, sourceType: %{public}d", currentActiveDevice_,
            attr_.sourceType);
        return SUCCESS;
    }
    AUDIO_INFO_LOG("sourceType: %{public}d", sourceType);
    attr_.sourceType = sourceType;
    return DoSetInputRoute(currentActiveDevice_);
}

void AudioCaptureSource::RegistCallback(uint32_t type, IAudioSourceCallback *callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

void AudioCaptureSource::RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

int32_t AudioCaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

int32_t AudioCaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

void AudioCaptureSource::SetAddress(const std::string &address)
{
    address_ = address;
}

void AudioCaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: PrimarySource\tstarted: " + std::string(started_.load() ? "true" : "false") + "\thalName: " +
        halName_ + "\tcurrentActiveDevice: " + std::to_string(currentActiveDevice_) + "\tsourceType: " +
        std::to_string(attr_.sourceType) + "\n";
}

AudioFormat AudioCaptureSource::ConvertToHdiFormat(AudioSampleFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

uint64_t AudioCaptureSource::GetChannelLayoutByChannelCount(uint32_t channelCount)
{
    uint64_t channelLayout = 0;
    switch (channelCount) {
        case MONO:
            channelLayout = CH_LAYOUT_MONO;
            break;
        case STEREO:
            channelLayout = CH_LAYOUT_STEREO;
            break;
        case CHANNEL_4:
            channelLayout = CH_LAYOUT_QUAD;
            break;
        case CHANNEL_8:
            channelLayout = CH_LAYOUT_7POINT1;
            break;
        default:
            channelLayout = CH_LAYOUT_STEREO;
            break;
    }
    return channelLayout;
}

enum AudioInputType AudioCaptureSource::ConvertToHDIAudioInputType(int32_t sourceType)
{
    enum AudioInputType hdiAudioInputType;
    switch (sourceType) {
        case SOURCE_TYPE_INVALID:
            hdiAudioInputType = AUDIO_INPUT_DEFAULT_TYPE;
            break;
        case SOURCE_TYPE_MIC:
        case SOURCE_TYPE_PLAYBACK_CAPTURE:
        case SOURCE_TYPE_ULTRASONIC:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
        case SOURCE_TYPE_WAKEUP:
            hdiAudioInputType = AUDIO_INPUT_SPEECH_WAKEUP_TYPE;
            break;
        case SOURCE_TYPE_VOICE_TRANSCRIPTION:
        case SOURCE_TYPE_VOICE_COMMUNICATION:
            hdiAudioInputType = AUDIO_INPUT_VOICE_COMMUNICATION_TYPE;
            break;
        case SOURCE_TYPE_VOICE_RECOGNITION:
            hdiAudioInputType = AUDIO_INPUT_VOICE_RECOGNITION_TYPE;
            break;
        case SOURCE_TYPE_VOICE_CALL:
            hdiAudioInputType = AUDIO_INPUT_VOICE_CALL_TYPE;
            break;
        case SOURCE_TYPE_CAMCORDER:
            hdiAudioInputType = AUDIO_INPUT_CAMCORDER_TYPE;
            break;
        case SOURCE_TYPE_EC:
            hdiAudioInputType = AUDIO_INPUT_EC_TYPE;
            break;
        case SOURCE_TYPE_MIC_REF:
            hdiAudioInputType = AUDIO_INPUT_NOISE_REDUCTION_TYPE;
            break;
        case SOURCE_TYPE_UNPROCESSED:
            hdiAudioInputType = AUDIO_INPUT_RAW_TYPE;
            break;
        case SOURCE_TYPE_LIVE:
            hdiAudioInputType = AUDIO_INPUT_LIVE_TYPE;
            break;
        default:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
    }
    return hdiAudioInputType;
}

void AudioCaptureSource::CheckAcousticEchoCancelerSupported(int32_t sourceType, int32_t &hdiAudioInputType)
{
    CHECK_AND_RETURN(sourceType == SOURCE_TYPE_LIVE);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "local device manager is nullptr");
    std::string value = deviceManager->GetAudioParameter("primary", AudioParamKey::PARAM_KEY_STATE,
        "source_type_live_aec_supported");
    if (value != "true") {
        HILOG_COMM_INFO("SOURCE_TYPE_LIVE not supported will be changed to SOURCE_TYPE_MIC");
        hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
    }
}

AudioSampleFormat AudioCaptureSource::ParseAudioFormat(const std::string &format)
{
    if (format == "AUDIO_FORMAT_PCM_16_BIT") {
        return SAMPLE_S16LE;
    } else if (format == "AUDIO_FORMAT_PCM_24_BIT" || format == "AUDIO_FORMAT_PCM_24_BIT_PACKED") {
        return SAMPLE_S24LE;
    } else if (format == "AUDIO_FORMAT_PCM_32_BIT") {
        return SAMPLE_S32LE;
    } else {
        return SAMPLE_S16LE;
    }
}

AudioCategory AudioCaptureSource::GetAudioCategory(AudioScene audioScene)
{
    AudioCategory audioCategory;
    switch (audioScene) {
        case AUDIO_SCENE_DEFAULT:
            audioCategory = AUDIO_IN_MEDIA;
            break;
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_VOICE_RINGING:
            audioCategory = AUDIO_IN_RINGTONE;
            break;
        case AUDIO_SCENE_PHONE_CALL:
            audioCategory = AUDIO_IN_CALL;
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            audioCategory = AUDIO_IN_COMMUNICATION;
            break;
        default:
            audioCategory = AUDIO_IN_MEDIA;
            break;
    }
    AUDIO_DEBUG_LOG("audioCategory: %{public}d", audioCategory);

    return audioCategory;
}

int32_t AudioCaptureSource::GetByteSizeByFormat(AudioSampleFormat format)
{
    int32_t byteSize = 0;
    switch (format) {
        case SAMPLE_U8:
            byteSize = BYTE_SIZE_SAMPLE_U8;
            break;
        case SAMPLE_S16LE:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
        case SAMPLE_S24LE:
            byteSize = BYTE_SIZE_SAMPLE_S24;
            break;
        case SAMPLE_S32LE:
            byteSize = BYTE_SIZE_SAMPLE_S32;
            break;
        default:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
    }

    return byteSize;
}

bool AudioCaptureSource::IsFormalSourceType(int32_t sourceType)
{
    if (sourceType == SOURCE_TYPE_EC || sourceType == SOURCE_TYPE_MIC_REF) {
        return false;
    }
    return true;
}

uint32_t AudioCaptureSource::GetUniqueId(void) const
{
    if (halName_ == HDI_ID_INFO_USB) {
        return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_USB);
    } else if (halName_ == HDI_ID_INFO_ACCESSORY) {
        return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_ACCESSORY);
    }
    return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_PRIMARY);
}

uint32_t AudioCaptureSource::GetUniqueIdBySourceType(void) const
{
    switch (attr_.sourceType) {
        case SOURCE_TYPE_EC:
            return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_EC);
        case SOURCE_TYPE_MIC_REF:
            return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_MIC_REF);
        case SOURCE_TYPE_WAKEUP:
            return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_WAKEUP);
        default:
            return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_PRIMARY);
    }
}

void AudioCaptureSource::InitEcOrMicRefAttr(const IAudioSourceAttr &attr)
{
    attr_.adapterName = attr.adapterName;
    attr_.openMicSpeaker = attr.openMicSpeaker;
    attr_.format = attr.format;
    attr_.sampleRate = attr.sampleRate;
    attr_.channel = attr.channel;
    attr_.bufferSize = USB_DEFAULT_BUFFER_SIZE;
    attr_.isBigEndian = attr.isBigEndian;
    attr_.filePath = "";
    attr_.deviceNetworkId = "LocalDevice";
    attr_.deviceType = attr.deviceType;
    attr_.sourceType = attr.sourceType;
    if (attr_.sourceType == SOURCE_TYPE_EC) {
        attr_.formatEc = attr.format;
        attr_.sampleRateEc = attr.sampleRate;
        attr_.channelEc = attr.channel;
    }
}

void AudioCaptureSource::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = true;
    param.streamId = static_cast<int32_t>(GetUniqueIdBySourceType());
    param.type = AUDIO_IN_MEDIA;
    param.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    param.isBigEndian = false;
    param.isSignedData = true;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    }
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = AUDIO_BUFFER_SIZE;
    param.sourceType = SOURCE_TYPE_MIC;

    param.sampleRate = attr_.sampleRate;
    param.format = ConvertToHdiFormat(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    param.channelLayout = attr_.channelLayout;
    if (param.channelLayout == CH_LAYOUT_UNKNOWN) {
        param.channelLayout = GetChannelLayoutByChannelCount(attr_.channel);
    }
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    }
    param.sourceType = static_cast<int32_t>(ConvertToHDIAudioInputType(attr_.sourceType));
    CheckAcousticEchoCancelerSupported(attr_.sourceType, param.sourceType);

    if ((attr_.hasEcConfig || attr_.sourceType == SOURCE_TYPE_EC) && attr_.channelEc != 0) {
        param.ecSampleAttributes.ecInterleaved = true;
        param.ecSampleAttributes.ecFormat = ConvertToHdiFormat(attr_.formatEc);
        param.ecSampleAttributes.ecSampleRate = attr_.sampleRateEc;
        param.ecSampleAttributes.ecChannelCount = attr_.channelEc;
        param.ecSampleAttributes.ecChannelLayout = GetChannelLayoutByChannelCount(attr_.channelEc);
        param.ecSampleAttributes.ecPeriod = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
        param.ecSampleAttributes.ecFrameSize = PCM_16_BIT * param.ecSampleAttributes.ecChannelCount / PCM_8_BIT;
        param.ecSampleAttributes.ecIsBigEndian = false;
        param.ecSampleAttributes.ecIsSignedData = true;
        param.ecSampleAttributes.ecStartThreshold =
            DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.ecSampleAttributes.ecFrameSize);
        param.ecSampleAttributes.ecStopThreshold = INT_MAX;
        param.ecSampleAttributes.ecSilenceThreshold = AUDIO_BUFFER_SIZE;
    }
}

void AudioCaptureSource::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_IN_MIC;
    if (halName_ == HDI_ID_INFO_USB) {
        deviceDesc.pins = PIN_IN_USB_HEADSET;
    } else if (halName_ == HDI_ID_INFO_ACCESSORY) {
        if (dmDeviceTypeMap_[DEVICE_TYPE_ACCESSORY] == DM_DEVICE_TYPE_PENCIL) {
            deviceDesc.pins = PIN_IN_PENCIL;
        } else if (dmDeviceTypeMap_[DEVICE_TYPE_ACCESSORY] == DM_DEVICE_TYPE_UWB) {
            deviceDesc.pins = PIN_IN_UWB;
        }
    }
    deviceDesc.desc = const_cast<char *>(address_.c_str());
}

void AudioCaptureSource::InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene)
{
    sceneDesc.scene.id = GetAudioCategory(audioScene);

    AudioPortPin port = PIN_IN_MIC;
    if (halName_ == HDI_ID_INFO_USB) {
        port = PIN_IN_USB_HEADSET;
    } else if (halName_ == HDI_ID_INFO_ACCESSORY) {
        if (dmDeviceTypeMap_[DEVICE_TYPE_ACCESSORY] == DM_DEVICE_TYPE_PENCIL) {
            port = PIN_IN_PENCIL;
        } else if (dmDeviceTypeMap_[DEVICE_TYPE_ACCESSORY] == DM_DEVICE_TYPE_UWB) {
            port = PIN_IN_UWB;
        }
    }
    AUDIO_DEBUG_LOG("port: %{public}d", port);
    sceneDesc.desc.pins = port;
    sceneDesc.desc.desc = const_cast<char *>("");
}

// LCOV_EXCL_START
void AudioCaptureSource::SetAudioRouteInfoForEnhanceChain(void)
{
    if (IsNonblockingSource(attr_.adapterName)) {
        AUDIO_ERR_LOG("non blocking source not support");
        return;
    }
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag != 1) {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_LOG(audioEnhanceChainManager != nullptr, "audioEnhanceChainManager is nullptr");
        if (halName_ == HDI_ID_INFO_USB) {
            audioEnhanceChainManager->SetInputDevice(captureId_, DEVICE_TYPE_USB_ARM_HEADSET, "");
        } else {
            audioEnhanceChainManager->SetInputDevice(captureId_, currentActiveDevice_, "");
        }
    }
}
// LCOV_EXCL_STOP

bool AudioCaptureSource::IsCaptureInvalid(void)
{
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("audioCapture_ is nullptr!");
        std::string errorMsg = attr_.adapterName + " load adapter fail, ret: " + std::to_string(ERR_NOT_STARTED);
        HdiMonitor::ReportHdiException(HdiType::LOCAL, ErrorCase::CALL_HDI_FAILED, ERR_NOT_STARTED, errorMsg);
        return false;
    }
    return true;
}

int32_t AudioCaptureSource::CreateCapture(void)
{
    Trace trace("AudioCaptureSource::CreateCapture");

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create capture, halName: %{public}s, hdiSourceType: %{public}d, rate: %{public}u, "
        "channel: %{public}u, format: %{public}u, devicePin: %{public}u, desc: %{public}s", halName_.c_str(),
        param.sourceType, param.sampleRate, param.channelCount, param.format, deviceDesc.pins, deviceDesc.desc);
    if (attr_.hasEcConfig || attr_.sourceType == SOURCE_TYPE_EC) {
        AUDIO_INFO_LOG("config ec, rate: %{public}d, channel: %{public}u, format: %{public}u",
            param.ecSampleAttributes.ecSampleRate, param.ecSampleAttributes.ecChannelCount,
            param.ecSampleAttributes.ecFormat);
    }
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *capture = deviceManager->CreateCapture(adapterNameCase_, &param, &deviceDesc, hdiCaptureId_);
    audioCapture_ = static_cast<struct IAudioCapture *>(capture);
    CHECK_AND_RETURN_RET(IsCaptureInvalid(), ERR_NOT_STARTED);

    AUDIO_INFO_LOG("create capture success, hdiCaptureId: %{public}u, desc: %{public}s", hdiCaptureId_,
        deviceDesc.desc);
    return SUCCESS;
}

int32_t AudioCaptureSource::DoSetInputRoute(DeviceType inputDevice)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    int32_t streamId = static_cast<int32_t>(GetUniqueIdBySourceType());
    int32_t inputType = static_cast<int32_t>(ConvertToHDIAudioInputType(attr_.sourceType));
    CheckAcousticEchoCancelerSupported(attr_.sourceType, inputType);
    AUDIO_INFO_LOG("adapterName: %{public}s, inputDevice: %{public}d, streamId: %{public}d, inputType: %{public}d",
        attr_.adapterName.c_str(), inputDevice, streamId, inputType);
    int32_t ret = deviceManager->SetInputRoute(adapterNameCase_, inputDevice, streamId, inputType);
    return ret;
}

int32_t AudioCaptureSource::InitCapture(void)
{
    if (captureInited_) {
        AUDIO_INFO_LOG("capture already inited");
        return SUCCESS;
    }
    AUDIO_INFO_LOG("In, openMicL %{public}u, halName: %{public}s", openMic_, halName_.c_str());
    int32_t ret = CreateCapture();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create capture fail");
    if (openMic_) {
        ret = SUCCESS;
        DeviceType inputDevice;
        if (halName_ == HDI_ID_INFO_USB) {
            inputDevice = DEVICE_TYPE_USB_ARM_HEADSET;
        } else {
            inputDevice = static_cast<DeviceType>(attr_.deviceType);
        }
        ret = UpdateActiveDeviceWithoutLock(inputDevice);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("update route fail, ret: %{public}d", ret);
        }
        SetAudioRouteInfoForEnhanceChain();
    }
    captureInited_ = true;
    return SUCCESS;
}

void AudioCaptureSource::InitLatencyMeasurement(void)
{
    std::lock_guard<std::mutex> lock(signalDetectMutex_);

    CHECK_AND_RETURN(AudioLatencyMeasurement::CheckIfEnabled());
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "signalDetectAgent is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
}

void AudioCaptureSource::DeInitLatencyMeasurement(void)
{
    std::lock_guard<std::mutex> lock(signalDetectMutex_);

    signalDetected_ = false;
    signalDetectAgent_ = nullptr;
}

void AudioCaptureSource::CheckLatencySignal(uint8_t *frame, size_t replyBytes)
{
    std::lock_guard<std::mutex> lock(signalDetectMutex_);
    CHECK_AND_RETURN(signalDetectAgent_ != nullptr);
    signalDetected_ = signalDetectAgent_->CheckAudioData(frame, replyBytes);
    if (signalDetected_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN(deviceManager != nullptr);
        std::string value = deviceManager->GetAudioParameter(adapterNameCase_, key, condition);

        LatencyMonitor::GetInstance().UpdateDspTime(value.c_str());
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(false, signalDetectAgent_->lastPeakBufferTime_);
        AUDIO_INFO_LOG("signal detected");
        signalDetected_ = false;
    }
}

void AudioCaptureSource::CheckUpdateState(char *frame, size_t replyBytes)
{
    if (startUpdate_) {
        std::lock_guard<std::mutex> lock(statusMutex_);
        if (captureFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        captureFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), frame, replyBytes);
        if (captureFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            captureFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

bool AudioCaptureSource::IsNonblockingSource(const std::string &adapterName)
{
    return (attr_.sourceType == SOURCE_TYPE_EC && adapterName != "dp") ||
        (attr_.sourceType == SOURCE_TYPE_MIC_REF);
}

int32_t AudioCaptureSource::NonblockingStart(void)
{
    if (started_.load()) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Start(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail");
    started_.store(true);

    isCaptureThreadRunning_ = true;
    captureThread_ = std::make_unique<std::thread>(&AudioCaptureSource::CaptureThreadLoop, this);
    std::string threadName = "OS_Capture";
    threadName += (attr_.sourceType == SOURCE_TYPE_EC) ? "Ec" : "MicRef";
    pthread_setname_np(captureThread_->native_handle(), threadName.c_str());
    return SUCCESS;
}

int32_t AudioCaptureSource::NonblockingStop(void)
{
    isCaptureThreadRunning_ = false;
    if (captureThread_ != nullptr && captureThread_->joinable()) {
        captureThread_->join();
    }

    if (!started_.load()) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Stop(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    started_.store(false);
    return SUCCESS;
}

int32_t AudioCaptureSource::NonblockingCaptureFrameWithEc(FrameDesc *fdescEc, uint64_t &replyBytesEc)
{
    if (ringBufferHandler_ != nullptr) {
        Trace trace("AudioCaptureSource::NonblockingCaptureFrameWithEc");
        int32_t ret = ringBufferHandler_->ReadDataFromRingBuffer(reinterpret_cast<uint8_t *>(fdescEc->frame),
            fdescEc->frameLen);
        if (ret == SUCCESS) {
            replyBytesEc = fdescEc->frameLen;
        } else {
            AUDIO_ERR_LOG("read ec data from ringBuffer fail");
            replyBytesEc = 0;
        }
    }
    return SUCCESS;
}

void AudioCaptureSource::CaptureFrameOnlyEc(std::vector<uint8_t> &ecData)
{
    struct AudioFrameLen frameLen = {};
    frameLen.frameLen = static_cast<uint64_t>(ecData.size());
    frameLen.frameEcLen = static_cast<uint64_t>(ecData.size());
    struct AudioCaptureFrameInfo frameInfo = {};
    CHECK_AND_RETURN_LOG(audioCapture_ != nullptr, "capture is nullptr");
    int32_t ret = audioCapture_->CaptureFrameEc(audioCapture_, &frameLen, &frameInfo);
    if (ret >= 0 && frameInfo.frameEc != nullptr) {
        if (memcpy_s(ecData.data(), ecData.size(), frameInfo.frameEc, frameInfo.replyBytesEc) != EOK) {
            AUDIO_ERR_LOG("copy ec fail");
        }
    }
    AudioCaptureFrameInfoFree(&frameInfo, false);
}

void AudioCaptureSource::CaptureThreadLoop(void)
{
    CHECK_AND_RETURN_LOG(ringBufferHandler_ != nullptr, "ringBufferHandler is nullptr");

    uint32_t captureDataLen = FRAME_TIME_LEN_MS * attr_.sampleRate / SECOND_TO_MILLISECOND *
        static_cast<uint32_t>(GetByteSizeByFormat(attr_.format)) * attr_.channel;
    AUDIO_INFO_LOG("start, sourceType: %{public}d, captureDataLen: %{public}u", attr_.sourceType, captureDataLen);
    std::vector<uint8_t> buffer;
    buffer.resize(captureDataLen);
    ScheduleThreadInServer(getpid(), gettid());
    while (isCaptureThreadRunning_) {
        Trace trace("CaptureRefInput");
        uint64_t replyBytes = 0;
        uint32_t requestBytes = static_cast<uint32_t>(buffer.size());
        int32_t ret = SUCCESS;
        if (attr_.sourceType == SOURCE_TYPE_MIC_REF) {
            CHECK_AND_RETURN_LOG(audioCapture_ != nullptr, "capture is nullptr");
            ret = audioCapture_->CaptureFrame(audioCapture_, reinterpret_cast<int8_t *>(buffer.data()), &requestBytes,
                &replyBytes);
        } else {
            CaptureFrameOnlyEc(buffer);
        }
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("capture frame fail");
        }
        ringBufferHandler_->WriteDataToRingBuffer(buffer.data(), buffer.size());
    }
    UnscheduleThreadInServer(getpid(), gettid());
    AUDIO_INFO_LOG("exit, sourceType: %{public}d", attr_.sourceType);
}

int32_t AudioCaptureSource::UpdateActiveDeviceWithoutLock(DeviceType inputDevice)
{
    AUDIO_INFO_LOG("current active device: %{public}d, inputDevice: %{public}d", currentActiveDevice_, inputDevice);
    if (currentActiveDevice_ == inputDevice) {
        AUDIO_INFO_LOG("input device not change, device: %{public}d, sourceType: %{public}d", inputDevice,
            attr_.sourceType);
        return SUCCESS;
    }

    int32_t ret = DoSetInputRoute(inputDevice);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    currentActiveDevice_ = inputDevice;
    if (inputDevice == DEVICE_TYPE_ACCESSORY) {
        SetAudioRouteInfoForEnhanceChain();
    }
    return SUCCESS;
}

int32_t AudioCaptureSource::SetAccessoryDeviceState(bool state)
{
    ErrCode ret;
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(settingProvider.CheckOsAccountReady(), ERROR, "os account not ready");
    if (state) {
        ret = settingProvider.PutStringValue("hw.pencil.mic_ack.state", "1", "global");
    } else {
        ret = settingProvider.PutStringValue("hw.pencil.mic_ack.state", "0", "global");
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Write Accessory state to Database failed");
    AUDIO_INFO_LOG("success write hw.pencil.mic_ack.state %{public}d to Database", state);
    return SUCCESS;
}

int32_t AudioCaptureSource::DoStop(void)
{
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    Trace trace("AudioCaptureSource::DoStop");

    if (IsNonblockingSource(adapterNameCase_)) {
        return NonblockingStop();
    }

    DeInitLatencyMeasurement();
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    int32_t ret = audioCapture_->Stop(audioCapture_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    started_.store(false);
    callback_.OnCaptureState(false);
    return SUCCESS;
}

void AudioCaptureSource::DumpData(char *frame, uint64_t &replyBytes)
{
    BufferDesc buffer = { reinterpret_cast<uint8_t*>(frame), replyBytes, replyBytes };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, frame, replyBytes);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(frame), replyBytes);
    }
}

void AudioCaptureSource::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    bool isDmDeviceTypeUpdated = (deviceType == currentActiveDevice_ && dmDeviceTypeMap_[deviceType] != dmDeviceType);
    dmDeviceTypeMap_[deviceType] = dmDeviceType;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "deviceManager is nullptr");
    deviceManager->SetDmDeviceType(dmDeviceType, deviceType);

    if (isDmDeviceTypeUpdated) {
        AUDIO_INFO_LOG("dm deviceType update, need update input port pin");
        int32_t ret = DoSetInputRoute(currentActiveDevice_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "DoSetInputRoute fails");
    }
}

} // namespace AudioStandard
} // namespace OHOS
