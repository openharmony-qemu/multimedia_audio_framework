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

#ifndef LOG_TAG
#define LOG_TAG "RemoteMultichannelAudioRenderSink"
#endif

#include "sink/remote_multichannel_audio_render_sink.h"
#include <sstream>
#include <climits>
#include <utility>
#include <algorithm>
#include "parameters.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "audio_performance_monitor.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "audio_stream_enum.h"
#include "securec.h"

using namespace OHOS::HDI::DistributedAudio::Audio::V2_0;

namespace OHOS {
namespace AudioStandard {
RemoteMultichannelAudioRenderSink::RemoteMultichannelAudioRenderSink(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
    AUDIO_INFO_LOG("construction");
}

RemoteMultichannelAudioRenderSink::~RemoteMultichannelAudioRenderSink()
{
    if (sinkInited_.load()) {
        DeInit();
    }
    AUDIO_INFO_LOG("destruction");
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(ADAPTER_TYPE_REMOTE_NM);
}

int32_t RemoteMultichannelAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    attr_ = attr;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);

    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");

    InitLatencyMeasurement();
    renderInited_.store(true);
    sinkInited_.store(true);
    InitPipeInfo(hdiRenderId_, HDI_ADAPTER_TYPE_REMOTE, AUDIO_OUTPUT_FLAG_MULTICHANNEL);
    return SUCCESS;
}

void RemoteMultichannelAudioRenderSink::DeInit(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("RemoteMultichannelAudioRenderSink::DeInit");
    AUDIO_INFO_LOG("in");

    sinkInited_.store(false);
    renderInited_.store(false);
    started_.store(false);
    paused_.store(false);

    DestroyRender();
    DeinitPipeInfo();
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    AUDIO_INFO_LOG("end");
}

bool RemoteMultichannelAudioRenderSink::IsInited(void)
{
    return sinkInited_.load();
}

int32_t RemoteMultichannelAudioRenderSink::Start(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteMultichannelAudioRenderSink::Start");
    if (!renderInited_.load() || !IsValidState()) {
        int32_t ret = CreateRender();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
        renderInited_.store(true);
    }
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME_BASE) + deviceNetworkId_);
        guard.CheckCurrTimeout();
    }
    runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
#endif
    CHECK_AND_RETURN_RET_LOG(!started_.load(), SUCCESS, "already started");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(IsValidState(), ERR_NOT_STARTED, "invalid state");

    int32_t ret = audioRender_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail");
    UpdateSinkState(true);
    ChangePipeStatus(PIPE_STATUS_RUNNING);
    InitDumpFile();
    started_.store(true);

    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_REMOTE_NM, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteMultichannelAudioRenderSink::Stop");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    if (!started_.load()) {
        AUDIO_INFO_LOG("already stopped");
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail ret: %{public}d", ret);

    UpdateSinkState(false);
    ChangePipeStatus(PIPE_STATUS_STANDBY);
    started_.store(false);
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::Resume(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteMultichannelAudioRenderSink::Resume");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");
    CHECK_AND_RETURN_RET_LOG(paused_.load(), SUCCESS, "already resumed");

    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Resume();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "resume fail, ret: %{public}d", ret);

    paused_.store(false);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_REMOTE_NM, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::Pause(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteMultichannelAudioRenderSink::Pause");

    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");
    CHECK_AND_RETURN_RET_LOG(!paused_.load(), SUCCESS, "already paused");

    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "pause fail, ret: %{public}d", ret);

    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::Flush(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteMultichannelAudioRenderSink::Flush");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "flush fail, ret: %{public}d", ret);

    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::Reset(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("RemoteMultichannelAudioRenderSink::Reset");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "reset fail, ret: %{public}d", ret);

    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load() && IsValidState(), ERR_ILLEGAL_STATE, "not create, invalid state");
    int64_t stamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    JUDGE_AND_WARNING_LOG(!started_.load(), "not start, invalid state");

    Trace trace("RemoteMultichannelAudioRenderSink::RenderFrame");
    CheckLatencySignal(reinterpret_cast<uint8_t *>(&data), len);

    if (switchDeviceMute_) {
        Trace trace("RemoteMultichannelAudioRenderSink::RenderFrame::switch");
        if (memset_s(reinterpret_cast<void *>(&data), static_cast<size_t>(len), 0, static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s fail");
        }
    }

    std::vector<int8_t> bufferVec(len);
    int32_t ret = memcpy_s(bufferVec.data(), len, &data, len);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "copy fail, error code: %{public}d", ret);

    BufferDesc buffer = { reinterpret_cast<uint8_t *>(&data), len, len };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_);
    Trace::CountVolume("RemoteMultichannelAudioRenderSink::RenderFrame", static_cast<uint8_t>(data));
    ret = audioRender_->RenderFrame(bufferVec, writeLen);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_REMOTE_NM, ClockTime::GetCurNano());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "fail, ret: %{public}x", ret);
    writeLen = len;
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), len);
    }
    CheckUpdateState(&data, len);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    int64_t threshold = 50; // 50ms
    JUDGE_AND_WARNING_LOG(stamp >= threshold, "len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    if (logMode_) {
        AUDIO_DEBUG_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    }

    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::GetVolumeDataCount(int64_t &volumeData)
{
    volumeData = volumeDataCount_;
    return SUCCESS;
}

void RemoteMultichannelAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");
    CHECK_AND_RETURN(IsValidState());
    int32_t ret = audioRender_->SetExtraParams(value.c_str());
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "set parameter fail, error code: %{public}d", ret);
}

std::string RemoteMultichannelAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_INFO_LOG("not support");
    return "";
}

int32_t RemoteMultichannelAudioRenderSink::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load() && IsValidState(), ERR_ILLEGAL_STATE, "not create, invalid state");
    leftVolume_ = left;
    rightVolume_ = right;
    float volume;
    if (leftVolume_ == 0) {
        volume = rightVolume_;
    } else if (rightVolume_ == 0) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->SetVolume(volume);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set volume fail, ret: %{public}d", ret);

    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::SetVolumeWithRamp(float left, float right, uint32_t durationMs)
{
    AUDIO_INFO_LOG("RemoteMultichannelAudioRenderSink::SetVolumeWithRamp in");
    return SetVolume(left, right);
}

int32_t RemoteMultichannelAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::GetLatency(uint32_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load() && IsValidState(), ERR_ILLEGAL_STATE, "not create, invalid state");
    uint32_t hdiLatency = 0;
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->GetLatency(hdiLatency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail, ret: %{public}d", ret);

    latency = hdiLatency;
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec,
    int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float RemoteMultichannelAudioRenderSink::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

void RemoteMultichannelAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void RemoteMultichannelAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteMultichannelAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set multichannel mute %{public}d", mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("multichannel already muted");
            return SUCCESS;
        }
        switchDeviceMute_ = true;
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("multichannel not all unmuted");
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
    }

    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag,
    int32_t phoneCallSceneType)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load() && IsValidState(), ERR_ILLEGAL_STATE, "not create, invalid state");
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");

    int32_t ret = DoSetOutputRoute();
    JUDGE_AND_WARNING_LOG(ret != SUCCESS, "update route fail, ret: %{public}d", ret);
    CHECK_AND_RETURN_RET(audioScene != currentAudioScene_, SUCCESS);

    struct AudioSceneDescriptor sceneDesc = {
        .scene.id = GetAudioCategory(audioScene),
        .desc.pins = AudioPortPin::PIN_OUT_SPEAKER,
    };

    AUDIO_DEBUG_LOG("start");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    ret = audioRender_->SelectScene(sceneDesc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
    currentAudioScene_ = audioScene;
    AUDIO_DEBUG_LOG("end, audioScene: %{public}d", audioScene);
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::GetAudioScene(void)
{
    return currentAudioScene_;
}

int32_t RemoteMultichannelAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    CHECK_AND_RETURN_RET_LOG(!outputDevices.empty() && outputDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "invalid device");
    if (currentActiveDevice_ == outputDevices[0]) {
        AUDIO_INFO_LOG("output device not change, device: %{public}d", outputDevices[0]);
        return SUCCESS;
    }
    AUDIO_INFO_LOG("device: %{public}d", outputDevices[0]);
    currentActiveDevice_ = outputDevices[0];
    return DoSetOutputRoute();
}

DeviceType RemoteMultichannelAudioRenderSink::GetCurrentOutputDevice()
{
    return currentActiveDevice_;
}

void RemoteMultichannelAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    currentActiveDevice_ = currentActiveDevice_ == device ? DEVICE_TYPE_NONE : currentActiveDevice_;
}

int32_t RemoteMultichannelAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t RemoteMultichannelAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    Trace trace("RemoteMultichannelAudioRenderSink:UpdateAppsUid");
    std::unordered_set<int32_t> lastAppsUid = appsUid_;
    std::unordered_set<int32_t> appsUidSet(appsUid.cbegin(), appsUid.cend());
    appsUid_ = std::move(appsUidSet);
    CHECK_AND_RETURN_RET(appsUid_ != lastAppsUid, SUCCESS);

    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, SUCCESS);
    std::string appInfoStr = GenerateAppsUidStr(appsUid_);
    int32_t ret = audioRender_->SetExtraParams(appInfoStr.c_str());
    AUDIO_INFO_LOG("set parameter: %{public}s, ret: %{public}d", appInfoStr.c_str(), ret);
    return SUCCESS;
}

void RemoteMultichannelAudioRenderSink::SetInvalidState(void)
{
    AUDIO_INFO_LOG("update validState:false, adapterName: %{public}s", GetEncryptStr(deviceNetworkId_).c_str());
    std::lock_guard<std::mutex> lock(sinkMutex_);
    validState_.store(false);
    sinkInited_.store(false);
    renderInited_.store(false);
    started_.store(false);
    paused_.store(false);
}

void RemoteMultichannelAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: RemoteMultichannelSink\tstarted: " + std::string(started_.load() ? "true" : "false") +
        "\tdeviceNetworkId: " + deviceNetworkId_ + "\n";
}

void RemoteMultichannelAudioRenderSink::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        DeInit();
    }

    callback_.OnRenderSinkParamChange(adapterName, key, condition, value);
}

uint32_t RemoteMultichannelAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
{
    AudioFormat hdiFormat = ConvertToHdiFormat(format);
    switch (hdiFormat) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            return PCM_8_BIT;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            return PCM_16_BIT;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            return PCM_24_BIT;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            return PCM_32_BIT;
        default:
            AUDIO_DEBUG_LOG("unknown format type, set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat RemoteMultichannelAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
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

AudioSampleFormat RemoteMultichannelAudioRenderSink::ParseAudioFormat(const std::string &format)
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

AudioCategory RemoteMultichannelAudioRenderSink::GetAudioCategory(AudioScene audioScene)
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

void RemoteMultichannelAudioRenderSink::InitAudioSampleAttr(AudioSampleAttributes &param)
{
    param.channelCount = STEREO_CHANNEL_COUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = 0;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE,
        HDI_RENDER_OFFSET_REMOTE_MULTICHANNEL));
    param.type = AUDIO_MULTI_CHANNEL;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.channelLayout = attr_.channelLayout;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBit(attr_.format) * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
}

void RemoteMultichannelAudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = AudioPortPin::PIN_OUT_SPEAKER;
    deviceDesc.desc = "";
}

void RemoteMultichannelAudioRenderSink::InitDumpFile()
{
    dumpFileName_ = std::string(DUMP_REMOTE_MCH_RENDER_SINK_FILENAME) + "_" +
        GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" +
        std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
}

int32_t RemoteMultichannelAudioRenderSink::CreateRender(void)
{
    int64_t stamp = ClockTime::GetCurNano();

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, rate: %{public}u, channel: %{public}u, format: %{public}u, channelLayout: %{public}"
        PRIu64, param.sampleRate, param.channelCount, param.format, param.channelLayout);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(deviceNetworkId_, &param, &deviceDesc, hdiRenderId_);
    audioRender_.ForceSetRefPtr(static_cast<IAudioRender *>(render));
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);
    std::stringstream val;
    val << "multichannelParams=" << attr_.sampleRate << "," << std::to_string(attr_.format) << ","
        << attr_.channel << "," << attr_.channelLayout;
    SetAudioParameter(AudioParamKey::NONE, "", val.str());
    deviceManager->RegistRenderSinkCallback(deviceNetworkId_, hdiRenderId_, shared_from_this());
    int32_t ret = audioRender_->SetExtraParams(GenerateAppsUidStr(appsUid_).c_str());
    AUDIO_INFO_LOG("set app info params, ret: %{public}d", ret);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    validState_.store(true);
    AUDIO_INFO_LOG("create render success, cost: [%{public}" PRId64 "]ms", stamp);
    return SUCCESS;
}

void RemoteMultichannelAudioRenderSink::DestroyRender()
{
    AUDIO_INFO_LOG("destroy render");
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "deviceManager is nullptr");
    CHECK_AND_RETURN(IsValidState());
    deviceManager->DestroyRender(deviceNetworkId_, hdiRenderId_);
    deviceManager->UnRegistRenderSinkCallback(deviceNetworkId_, hdiRenderId_);
    audioRender_.ForceSetRefPtr(nullptr);

    validState_.store(true);
    switchDeviceMute_ = false;
}

int32_t RemoteMultichannelAudioRenderSink::DoSetOutputRoute()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    int32_t ret = deviceManager->SetOutputRoute(deviceNetworkId_, { DEVICE_TYPE_SPEAKER }, static_cast<int32_t>(
        GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE_MULTICHANNEL)));
    return ret;
}

void RemoteMultichannelAudioRenderSink::InitLatencyMeasurement(void)
{
    CHECK_AND_RETURN(AudioLatencyMeasurement::CheckIfEnabled());

    AUDIO_INFO_LOG("in");
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "signalDetectAgent is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    signalDetected_ = false;
}

void RemoteMultichannelAudioRenderSink::CheckLatencySignal(uint8_t *data, size_t len)
{
    CHECK_AND_RETURN(signalDetectAgent_ != nullptr);
    uint32_t byteSize = static_cast<uint32_t>(GetFormatByteSize(attr_.format));
    size_t newlyCheckedTime = len / (attr_.sampleRate / MILLISECOND_PER_SECOND) /
        (byteSize * sizeof(uint8_t) * attr_.channel);
    signalDetectedTime_ += newlyCheckedTime;
    if (signalDetectedTime_ >= MILLISECOND_PER_SECOND && signalDetectAgent_->signalDetected_) {
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true, signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().ShowTimestamp(true);
        signalDetectAgent_->signalDetected_ = false;
    }
    signalDetected_ = signalDetectAgent_->CheckAudioData(data, len);
    if (signalDetected_) {
        AUDIO_INFO_LOG("signal detected");
        signalDetectedTime_ = 0;
    }
}

bool RemoteMultichannelAudioRenderSink::IsValidState()
{
    JUDGE_AND_WARNING_LOG(!validState_.load(), "disconnected, render invalid");
    return validState_.load();
}

void RemoteMultichannelAudioRenderSink::CheckUpdateState(char *data, uint64_t len)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), data, len);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
            }
        }
    }
}

// must be called with sinkMutex_ held
void RemoteMultichannelAudioRenderSink::UpdateSinkState(bool started)
{
    callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE_MULTICHANNEL),
        started);
}

} // namespace AudioStandard
} // namespace OHOS
