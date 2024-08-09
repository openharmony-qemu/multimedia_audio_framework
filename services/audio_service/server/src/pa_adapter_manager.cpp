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
#ifndef LOG_TAG
#define LOG_TAG "PaAdapterManager"
#endif

#include "pa_adapter_manager.h"
#include <sstream>
#include <atomic>
#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_schedule.h"
#include "pa_adapter_tools.h"
#include "pa_renderer_stream_impl.h"
#include "pa_capturer_stream_impl.h"
#include "audio_utils.h"
#include "audio_info.h"
#include "policy_handler.h"

namespace OHOS {
namespace AudioStandard {
const uint32_t CHECK_UTIL_SUCCESS = 0;
const uint64_t BUF_LENGTH_IN_MSEC = 20;
static const uint32_t PA_RECORD_MAX_LENGTH_NORMAL = 4;
static const uint32_t PA_RECORD_MAX_LENGTH_WAKEUP = 30;
static const int32_t CONNECT_STREAM_TIMEOUT_IN_SEC = 8; // 8S
static const std::unordered_map<AudioStreamType, std::string> STREAM_TYPE_ENUM_STRING_MAP = {
    {STREAM_VOICE_CALL, "voice_call"},
    {STREAM_MUSIC, "music"},
    {STREAM_RING, "ring"},
    {STREAM_MEDIA, "media"},
    {STREAM_VOICE_ASSISTANT, "voice_assistant"},
    {STREAM_SYSTEM, "system"},
    {STREAM_ALARM, "alarm"},
    {STREAM_NOTIFICATION, "notification"},
    {STREAM_BLUETOOTH_SCO, "bluetooth_sco"},
    {STREAM_ENFORCED_AUDIBLE, "enforced_audible"},
    {STREAM_DTMF, "dtmf"},
    {STREAM_TTS, "tts"},
    {STREAM_ACCESSIBILITY, "accessibility"},
    {STREAM_RECORDING, "recording"},
    {STREAM_MOVIE, "movie"},
    {STREAM_GAME, "game"},
    {STREAM_SPEECH, "speech"},
    {STREAM_SYSTEM_ENFORCED, "system_enforced"},
    {STREAM_ULTRASONIC, "ultrasonic"},
    {STREAM_WAKEUP, "wakeup"},
    {STREAM_VOICE_MESSAGE, "voice_message"},
    {STREAM_NAVIGATION, "navigation"},
    {STREAM_VOICE_COMMUNICATION, "voice_call"},
    {STREAM_VOICE_RING, "ring"},
    {STREAM_VOICE_CALL_ASSISTANT, "voice_call_assistant"},
};

static int32_t CheckReturnIfinvalid(bool expr, const int32_t retVal)
{
    do {
        if (!(expr)) {
            return retVal;
        }
    } while (false);
    return CHECK_UTIL_SUCCESS;
}

static bool IsEnhanceMode(SourceType sourceType)
{
    if (sourceType == SOURCE_TYPE_MIC || sourceType == SOURCE_TYPE_VOICE_COMMUNICATION ||
        sourceType == SOURCE_TYPE_VOICE_CALL) {
        return true;
    }
    return false;
}

PaAdapterManager::PaAdapterManager(ManagerType type)
{
    AUDIO_INFO_LOG("Constructor with type:%{public}d", type);
    mainLoop_ = nullptr;
    api_ = nullptr;
    context_ = nullptr;
    isContextConnected_ = false;
    isMainLoopStarted_ = false;
    managerType_ = type;
}

int32_t PaAdapterManager::CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream)
{
    AUDIO_DEBUG_LOG("Create renderer start");
    int32_t ret = InitPaContext();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to init pa context");
    uint32_t sessionId = PolicyHandler::GetInstance().GenerateSessionId(processConfig.appInfo.appUid);
    AUDIO_DEBUG_LOG("Create [%{public}d] type renderer:[%{public}u]", managerType_, sessionId);

    // PaAdapterManager is solely responsible for creating paStream objects
    // while the PaRendererStreamImpl has full authority over the subsequent management of the paStream
    pa_stream *paStream = InitPaStream(processConfig, sessionId, false);
    CHECK_AND_RETURN_RET_LOG(paStream != nullptr, ERR_OPERATION_FAILED, "Failed to init render");
    std::shared_ptr<IRendererStream> rendererStream = CreateRendererStream(processConfig, paStream);
    CHECK_AND_RETURN_RET_LOG(rendererStream != nullptr, ERR_DEVICE_INIT, "Failed to init pa stream");
    rendererStream->SetStreamIndex(sessionId);
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    rendererStreamMap_[sessionId] = rendererStream;
    stream = rendererStream;
    return SUCCESS;
}

int32_t PaAdapterManager::ReleaseRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Release [%{public}d] type render:[%{public}u]", managerType_, streamIndex);
    std::unique_lock<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    std::shared_ptr<IRendererStream> currentRender = rendererStreamMap_[streamIndex];
    rendererStreamMap_[streamIndex] = nullptr;
    rendererStreamMap_.erase(streamIndex);
    lock.unlock();

    if (currentRender->Release() < 0) {
        AUDIO_WARNING_LOG("Release stream %{public}d failed", streamIndex);
        return ERR_OPERATION_FAILED;
    }

    if (isHighResolutionExist_ == true && highResolutionIndex_ == streamIndex) {
        isHighResolutionExist_ = false;
    }
    AUDIO_INFO_LOG("rendererStreamMap_.size() : %{public}zu", rendererStreamMap_.size());
    if (rendererStreamMap_.size() == 0) {
        AUDIO_INFO_LOG("Release the last stream");
    }
    return SUCCESS;
}

int32_t PaAdapterManager::StartRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter StartRender");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    return rendererStreamMap_[streamIndex]->Start();
}

int32_t PaAdapterManager::StopRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter StopRender");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    return rendererStreamMap_[streamIndex]->Stop();
}

int32_t PaAdapterManager::PauseRender(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter PauseRender");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = rendererStreamMap_.find(streamIndex);
    if (it == rendererStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }
    rendererStreamMap_[streamIndex]->Pause();
    return SUCCESS;
}

int32_t PaAdapterManager::TriggerStartIfNecessary()
{
    return SUCCESS;
}

int32_t PaAdapterManager::GetStreamCount() const noexcept
{
    if (managerType_ == RECORDER) {
        return capturerStreamMap_.size();
    } else {
        return rendererStreamMap_.size();
    }
}

int32_t PaAdapterManager::CreateCapturer(AudioProcessConfig processConfig, std::shared_ptr<ICapturerStream> &stream)
{
    AUDIO_DEBUG_LOG("Create capturer start");
    CHECK_AND_RETURN_RET_LOG(managerType_ == RECORDER, ERROR, "Invalid managerType:%{public}d", managerType_);
    int32_t ret = InitPaContext();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to init pa context");
    uint32_t sessionId = PolicyHandler::GetInstance().GenerateSessionId(processConfig.appInfo.appUid);

    // PaAdapterManager is solely responsible for creating paStream objects
    // while the PaCapturerStreamImpl has full authority over the subsequent management of the paStream
    pa_stream *paStream = InitPaStream(processConfig, sessionId, true);
    CHECK_AND_RETURN_RET_LOG(paStream != nullptr, ERR_OPERATION_FAILED, "Failed to init capture");
    std::shared_ptr<ICapturerStream> capturerStream = CreateCapturerStream(processConfig, paStream);
    CHECK_AND_RETURN_RET_LOG(capturerStream != nullptr, ERR_DEVICE_INIT, "Failed to init pa stream");
    capturerStream->SetStreamIndex(sessionId);
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    capturerStreamMap_[sessionId] = capturerStream;
    stream = capturerStream;
    return SUCCESS;
}

int32_t PaAdapterManager::ReleaseCapturer(uint32_t streamIndex)
{
    AUDIO_DEBUG_LOG("Enter ReleaseCapturer");
    std::lock_guard<std::mutex> lock(streamMapMutex_);
    auto it = capturerStreamMap_.find(streamIndex);
    if (it == capturerStreamMap_.end()) {
        AUDIO_WARNING_LOG("No matching stream");
        return SUCCESS;
    }

    if (capturerStreamMap_[streamIndex]->Release() < 0) {
        AUDIO_WARNING_LOG("Release stream %{public}d failed", streamIndex);
        return ERR_OPERATION_FAILED;
    }

    capturerStreamMap_[streamIndex] = nullptr;
    capturerStreamMap_.erase(streamIndex);
    if (capturerStreamMap_.size() == 0) {
        AUDIO_INFO_LOG("Release the last stream");
    }
    return SUCCESS;
}

int32_t PaAdapterManager::ResetPaContext()
{
    AUDIO_DEBUG_LOG("Enter ResetPaContext");
    if (context_) {
        pa_context_set_state_callback(context_, nullptr, nullptr);
        if (isContextConnected_ == true) {
            PaLockGuard lock(mainLoop_);
            pa_context_disconnect(context_);
            pa_context_unref(context_);
            isContextConnected_ = false;
            context_ = nullptr;
        }
    }

    if (mainLoop_) {
        pa_threaded_mainloop_free(mainLoop_);
        isMainLoopStarted_  = false;
        mainLoop_ = nullptr;
    }

    api_ = nullptr;
    return SUCCESS;
}

int32_t PaAdapterManager::InitPaContext()
{
    AUDIO_DEBUG_LOG("Enter InitPaContext");
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    if (context_ != nullptr) {
        AUDIO_INFO_LOG("Context is not null, return");
        return SUCCESS;
    }
    mainLoop_ = pa_threaded_mainloop_new();
    CHECK_AND_RETURN_RET_LOG(mainLoop_ != nullptr, ERR_DEVICE_INIT, "Failed to init pa mainLoop");
    api_ = pa_threaded_mainloop_get_api(mainLoop_);
    if (managerType_ == PLAYBACK) {
        pa_threaded_mainloop_set_name(mainLoop_, "OS_RendererML");
    } else if (managerType_ == DUP_PLAYBACK) {
        pa_threaded_mainloop_set_name(mainLoop_, "OS_DRendererML");
    } else if (managerType_ == DUAL_PLAYBACK) {
        pa_threaded_mainloop_set_name(mainLoop_, "OS_DualRendererML");
    } else if (managerType_ == RECORDER) {
        pa_threaded_mainloop_set_name(mainLoop_, "OS_CapturerML");
    } else {
        AUDIO_ERR_LOG("Not supported managerType:%{public}d", managerType_);
    }
    if (api_ == nullptr) {
        AUDIO_ERR_LOG("Get api from mainLoop failed");
        pa_threaded_mainloop_free(mainLoop_);
        return ERR_DEVICE_INIT;
    }

    std::stringstream ss;
    ss << "app-pid<" << getpid() << ">-uid<" << getuid() << ">";
    std::string packageName = "";
    ss >> packageName;

    context_ = pa_context_new(api_, packageName.c_str());
    if (context_ == nullptr) {
        AUDIO_ERR_LOG("New context failed");
        pa_threaded_mainloop_free(mainLoop_);
        return ERR_DEVICE_INIT;
    }

    pa_context_set_state_callback(context_, PAContextStateCb, mainLoop_);
    if (pa_context_connect(context_, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        int error = pa_context_errno(context_);
        AUDIO_ERR_LOG("Context connect error: %{public}s", pa_strerror(error));
        return ERR_DEVICE_INIT;
    }
    isContextConnected_ = true;
    CHECK_AND_RETURN_RET_LOG(HandleMainLoopStart() == SUCCESS, ERR_DEVICE_INIT, "Failed to start pa mainLoop");

    return SUCCESS;
}

int32_t PaAdapterManager::HandleMainLoopStart()
{
    PaLockGuard lock(mainLoop_);
    if (pa_threaded_mainloop_start(mainLoop_) < 0) {
        return ERR_DEVICE_INIT;
    }
    isMainLoopStarted_ = true;

    while (true) {
        pa_context_state_t state = pa_context_get_state(context_);
        if (state == PA_CONTEXT_READY) {
            AUDIO_INFO_LOG("pa context is ready");
            break;
        }

        if (!PA_CONTEXT_IS_GOOD(state)) {
            int error = pa_context_errno(context_);
            AUDIO_ERR_LOG("Context bad state error: %{public}s", pa_strerror(error));
            lock.Unlock();
            ResetPaContext();
            return ERR_DEVICE_INIT;
        }
        pa_threaded_mainloop_wait(mainLoop_);
    }
    return SUCCESS;
}

int32_t PaAdapterManager::GetDeviceNameForConnect(AudioProcessConfig processConfig, uint32_t sessionId,
    std::string &deviceName)
{
    deviceName = "";
    if (processConfig.audioMode == AUDIO_MODE_RECORD) {
        if (processConfig.isWakeupCapturer) {
            int32_t ret = PolicyHandler::GetInstance().SetWakeUpAudioCapturerFromAudioServer(processConfig);
            if (ret < 0) {
                AUDIO_ERR_LOG("ErrorCode: %{public}d", ret);
                return ERROR;
            }
            deviceName = PRIMARY_WAKEUP;
        }
        if (processConfig.isInnerCapturer) {
            if (processConfig.innerCapMode == MODERN_INNER_CAP) {
                AUDIO_INFO_LOG("Create the modern inner-cap.");
                deviceName = NEW_INNER_CAPTURER_SOURCE;
            } else {
                deviceName = INNER_CAPTURER_SOURCE;
            }
        } else if (processConfig.capturerInfo.sourceType == SOURCE_TYPE_REMOTE_CAST) {
            deviceName = REMOTE_CAST_INNER_CAPTURER_SINK_NAME + MONITOR_SOURCE_SUFFIX;
        }
        return PolicyHandler::GetInstance().NotifyCapturerAdded(processConfig.capturerInfo,
            processConfig.streamInfo, sessionId);
    }
    return SUCCESS;
}

pa_stream *PaAdapterManager::InitPaStream(AudioProcessConfig processConfig, uint32_t sessionId, bool isRecording)
{
    AUDIO_DEBUG_LOG("Enter InitPaStream");
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    PaLockGuard palock(mainLoop_);
    if (CheckReturnIfinvalid(mainLoop_ && context_, ERR_ILLEGAL_STATE) < 0) {
        AUDIO_ERR_LOG("CheckReturnIfinvalid failed");
        return nullptr;
    }

    // Use struct to save spec size
    pa_sample_spec sampleSpec = ConvertToPAAudioParams(processConfig);
    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        return nullptr;
    }
    const std::string streamName = GetStreamName(processConfig.streamType);
    pa_channel_map map;
    CHECK_AND_RETURN_RET_LOG(SetPaProplist(propList, map, processConfig, streamName, sessionId) == 0, nullptr,
        "set pa proplist failed");

    pa_stream *paStream = pa_stream_new_with_proplist(context_, streamName.c_str(), &sampleSpec,
        isRecording ? nullptr : &map, propList);
    if (!paStream) {
        int32_t error = pa_context_errno(context_);
        AUDIO_ERR_LOG("pa_stream_new_with_proplist failed, error: %{public}d", error);
        pa_proplist_free(propList);
        return nullptr;
    }

    pa_proplist_free(propList);
    pa_stream_set_state_callback(paStream, PAStreamStateCb, reinterpret_cast<void *>(this));
    palock.Unlock();

    std::string deviceName;
    int32_t errorCode = GetDeviceNameForConnect(processConfig, sessionId, deviceName);
    if (errorCode != SUCCESS) {
        AUDIO_ERR_LOG("getdevicename err: %{public}d", errorCode);
        ReleasePaStream(paStream);
        return nullptr;
    }

    int32_t ret = ConnectStreamToPA(paStream, sampleSpec, processConfig.capturerInfo.sourceType, deviceName);
    if (ret < 0) {
        AUDIO_ERR_LOG("ConnectStreamToPA Failed");
        ReleasePaStream(paStream);
        return nullptr;
    }
    return paStream;
}

void PaAdapterManager::ReleasePaStream(pa_stream *paStream)
{
    if (!paStream) {
        AUDIO_INFO_LOG("paStream is nullptr. No need to release.");
        return;
    }
    if (!mainLoop_) {
        AUDIO_ERR_LOG("mainLoop_ is nullptr!");
        return;
    }

    PaLockGuard palock(mainLoop_);
    pa_stream_set_state_callback(paStream, nullptr, nullptr);

    pa_stream_state_t state = pa_stream_get_state(paStream);
    if (state == PA_STREAM_UNCONNECTED) {
        AUDIO_INFO_LOG("Release paStream unconnected");
        pa_stream_terminate(paStream);
    }
    pa_stream_unref(paStream);
    AUDIO_INFO_LOG("Release paStream because of errs");
}

bool PaAdapterManager::IsEffectNone(StreamUsage streamUsage)
{
    if (streamUsage == STREAM_USAGE_SYSTEM || streamUsage == STREAM_USAGE_DTMF ||
        streamUsage == STREAM_USAGE_ENFORCED_TONE || streamUsage == STREAM_USAGE_ULTRASONIC ||
        streamUsage == STREAM_USAGE_NAVIGATION || streamUsage == STREAM_USAGE_NOTIFICATION) {
        return true;
    }
    return false;
}

bool PaAdapterManager::CheckHighResolution(const AudioProcessConfig &processConfig)
{
    DeviceType deviceType = processConfig.deviceType;
    AudioStreamType streamType = processConfig.streamType;
    AudioSamplingRate sampleRate = processConfig.streamInfo.samplingRate;
    AudioSampleFormat sampleFormat = processConfig.streamInfo.format;

    AUDIO_DEBUG_LOG("deviceType:%{public}d, streamType:%{public}d, sampleRate:%{public}d, sampleFormat:%{public}d",
        deviceType, streamType, sampleRate, sampleFormat);

    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP && streamType == STREAM_MUSIC &&
        sampleRate >= SAMPLE_RATE_48000 && sampleFormat >= SAMPLE_S24LE) {
        return true;
    }
    return false;
}

void PaAdapterManager::SetHighResolution(pa_proplist *propList, AudioProcessConfig &processConfig, uint32_t sessionId)
{
    bool spatializationEnabled = processConfig.rendererInfo.spatializationEnabled;
    AUDIO_DEBUG_LOG("spatializationEnabled : %{public}d, isHighResolutionExist_ : %{public}d",
        spatializationEnabled, isHighResolutionExist_);

    if (spatializationEnabled == false && isHighResolutionExist_ == false && CheckHighResolution(processConfig)) {
        AUDIO_INFO_LOG("current stream marked as high resolution");
        pa_proplist_sets(propList, "stream.highResolution", "1");
        isHighResolutionExist_ = true;
        highResolutionIndex_ = sessionId;
    } else {
        AUDIO_INFO_LOG("current stream marked as non-high resolution");
        pa_proplist_sets(propList, "stream.highResolution", "0");
    }
}

void PaAdapterManager::SetRecordProplist(pa_proplist *propList, AudioProcessConfig &processConfig)
{
    pa_proplist_sets(propList, "stream.isInnerCapturer", std::to_string(processConfig.isInnerCapturer).c_str());
    pa_proplist_sets(propList, "stream.isWakeupCapturer", std::to_string(processConfig.isWakeupCapturer).c_str());
    pa_proplist_sets(propList, "stream.isIpcCapturer", std::to_string(true).c_str());
    pa_proplist_sets(propList, "stream.capturerSource",
        std::to_string(processConfig.capturerInfo.sourceType).c_str());
    const std::string sceneType = GetEnhanceSceneName(processConfig.capturerInfo.sourceType);
    AudioEnhanceMode enhanceMode = IsEnhanceMode(processConfig.capturerInfo.sourceType) ?
        ENHANCE_DEFAULT : ENHANCE_NONE;
    const std::string sceneMode = GetEnhanceModeName(enhanceMode);
    std::string upDevice = "DEVICE_TYPE_MIC";
    std::string downDevice = "DEVICE_TYPE_SPEAKER";
    pa_proplist_sets(propList, "scene.type", sceneType.c_str());
    pa_proplist_sets(propList, "scene.mode", sceneMode.c_str());
    pa_proplist_sets(propList, "device.up", upDevice.c_str());
    pa_proplist_sets(propList, "device.down", downDevice.c_str());
}

int32_t PaAdapterManager::SetPaProplist(pa_proplist *propList, pa_channel_map &map, AudioProcessConfig &processConfig,
    const std::string &streamName, uint32_t sessionId)
{
    // for remote audio device router filter
    pa_proplist_sets(propList, "stream.sessionID", std::to_string(sessionId).c_str());
    pa_proplist_sets(propList, "stream.client.uid", std::to_string(processConfig.appInfo.appUid).c_str());
    pa_proplist_sets(propList, "stream.client.pid", std::to_string(processConfig.appInfo.appPid).c_str());
    pa_proplist_sets(propList, "stream.type", streamName.c_str());
    pa_proplist_sets(propList, "media.name", streamName.c_str());

    float mVolumeFactor = 1.0f;
    float mPowerVolumeFactor = 1.0f;
    float mDuckVolumeFactor = 1.0f;
    pa_proplist_sets(propList, "stream.volumeFactor", std::to_string(mVolumeFactor).c_str());
    pa_proplist_sets(propList, "stream.powerVolumeFactor", std::to_string(mPowerVolumeFactor).c_str());
    pa_proplist_sets(propList, "stream.duckVolumeFactor", std::to_string(mDuckVolumeFactor).c_str());
    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::string streamStartTime = ctime(&timenow);
    pa_proplist_sets(propList, "stream.startTime", streamStartTime.c_str());

    if (processConfig.audioMode == AUDIO_MODE_PLAYBACK) {
        pa_proplist_sets(propList, "scene.mode",
            IsEffectNone(processConfig.rendererInfo.streamUsage) ? "EFFECT_NONE" : "EFFECT_DEFAULT");
        // mark dup stream for dismissing volume handle
        std::string streamMode = managerType_ == DUP_PLAYBACK ? DUP_STREAM
            : (managerType_ == DUAL_PLAYBACK ? DUAL_TONE_STREAM : NORMAL_STREAM);
        pa_proplist_sets(propList, "stream.mode", streamMode.c_str());
        pa_proplist_sets(propList, "stream.flush", "false");
        pa_proplist_sets(propList, "fadeoutPause", "0");
        pa_proplist_sets(propList, "stream.privacyType", std::to_string(processConfig.privacyType).c_str());
        pa_proplist_sets(propList, "stream.usage", std::to_string(processConfig.rendererInfo.streamUsage).c_str());
        pa_proplist_sets(propList, "scene.type", processConfig.rendererInfo.sceneType.c_str());
        pa_proplist_sets(propList, "spatialization.enabled",
            std::to_string(processConfig.rendererInfo.spatializationEnabled).c_str());
        pa_proplist_sets(propList, "headtracking.enabled",
            std::to_string(processConfig.rendererInfo.headTrackingEnabled).c_str());
        SetHighResolution(propList, processConfig, sessionId);
    } else if (processConfig.audioMode == AUDIO_MODE_RECORD) {
        SetRecordProplist(propList, processConfig);
    }

    AUDIO_INFO_LOG("Creating stream of channels %{public}d", processConfig.streamInfo.channels);
    if (processConfig.streamInfo.channelLayout == 0) {
        processConfig.streamInfo.channelLayout = defaultChCountToLayoutMap[processConfig.streamInfo.channels];
    }
    pa_proplist_sets(propList, "stream.channelLayout", std::to_string(processConfig.streamInfo.channelLayout).c_str());

    pa_channel_map_init(&map);
    map.channels = processConfig.streamInfo.channels;
    uint32_t channelsInLayout = ConvertChLayoutToPaChMap(processConfig.streamInfo.channelLayout, map);
    CHECK_AND_RETURN_RET_LOG(channelsInLayout == processConfig.streamInfo.channels && channelsInLayout != 0,
        ERR_INVALID_PARAM, "Invalid channel Layout");
    return SUCCESS;
}

std::shared_ptr<IRendererStream> PaAdapterManager::CreateRendererStream(AudioProcessConfig processConfig,
    pa_stream *paStream)
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    std::shared_ptr<PaRendererStreamImpl> rendererStream =
        std::make_shared<PaRendererStreamImpl>(paStream, processConfig, mainLoop_);
    if (rendererStream->InitParams() != SUCCESS) {
        int32_t error = pa_context_errno(context_);
        AUDIO_ERR_LOG("Create rendererStream Failed, error: %{public}d", error);
        return nullptr;
    }
    return rendererStream;
}

std::shared_ptr<ICapturerStream> PaAdapterManager::CreateCapturerStream(AudioProcessConfig processConfig,
    pa_stream *paStream)
{
    std::lock_guard<std::mutex> lock(paElementsMutex_);
    std::shared_ptr<PaCapturerStreamImpl> capturerStream =
        std::make_shared<PaCapturerStreamImpl>(paStream, processConfig, mainLoop_);
    if (capturerStream->InitParams() != SUCCESS) {
        int32_t error = pa_context_errno(context_);
        AUDIO_ERR_LOG("Create capturerStream Failed, error: %{public}d", error);
        return nullptr;
    }
    return capturerStream;
}

int32_t PaAdapterManager::ConnectStreamToPA(pa_stream *paStream, pa_sample_spec sampleSpec,
    SourceType source, const std::string &deviceName)
{
    AUDIO_DEBUG_LOG("Enter PaAdapterManager::ConnectStreamToPA");
    if (CheckReturnIfinvalid(mainLoop_ && context_ && paStream, ERROR) < 0) {
        return ERR_ILLEGAL_STATE;
    }

    PaLockGuard lock(mainLoop_);
    int32_t XcollieFlag = 1; // flag 1 generate log file
    if (managerType_ == PLAYBACK || managerType_ == DUP_PLAYBACK || managerType_ == DUAL_PLAYBACK) {
        int32_t rendererRet = ConnectRendererStreamToPA(paStream, sampleSpec);
        CHECK_AND_RETURN_RET_LOG(rendererRet == SUCCESS, rendererRet, "ConnectRendererStreamToPA failed");
    }
    if (managerType_ == RECORDER) {
        XcollieFlag = (1 | 2); // flag 1 generate log file, flag 2 die when timeout, restart server
        int32_t capturerRet = ConnectCapturerStreamToPA(paStream, sampleSpec, source, deviceName);
        CHECK_AND_RETURN_RET_LOG(capturerRet == SUCCESS, capturerRet, "ConnectCapturerStreamToPA failed");
    }
    while (waitConnect_) {
        pa_stream_state_t state = pa_stream_get_state(paStream);
        if (state == PA_STREAM_READY) {
            AUDIO_INFO_LOG("PaStream is ready");
            break;
        }
        if (!PA_STREAM_IS_GOOD(state)) {
            int32_t error = pa_context_errno(context_);
            AUDIO_ERR_LOG("connection to stream error: %{public}d", error);
            return ERR_INVALID_OPERATION;
        }
        AudioXCollie audioXCollie("PaAdapterManager::ConnectStreamToPA", CONNECT_STREAM_TIMEOUT_IN_SEC,
            [this](void *) {
                AUDIO_ERR_LOG("ConnectStreamToPA timeout, trigger signal");
                waitConnect_ = false;
                pa_threaded_mainloop_signal(this->mainLoop_, 0);
            }, nullptr, XcollieFlag);
        pa_threaded_mainloop_wait(mainLoop_);
    }
    return SUCCESS;
}

int32_t PaAdapterManager::ConnectRendererStreamToPA(pa_stream *paStream, pa_sample_spec sampleSpec)
{
    uint32_t tlength = 4; // 4 is tlength of playback
    uint32_t maxlength = 4; // 4 is max buffer length of playback
    uint32_t prebuf = 1; // 1 is prebuf of playback

    if (managerType_ == DUP_PLAYBACK || managerType_ == DUAL_PLAYBACK) {
        maxlength = 20; // 20 for cover offload
        prebuf = 2; // 2 is double of normal, use more prebuf for dup stream
    }
    AUDIO_INFO_LOG("Create ipc playback stream tlength: %{public}u, maxlength: %{public}u prebuf: %{public}u", tlength,
        maxlength, prebuf);
    pa_buffer_attr bufferAttr;
    bufferAttr.fragsize = static_cast<uint32_t>(-1);
    bufferAttr.prebuf = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * prebuf, &sampleSpec);
    bufferAttr.maxlength = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * maxlength, &sampleSpec);
    bufferAttr.tlength = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * tlength, &sampleSpec);
    bufferAttr.minreq = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC, &sampleSpec);

    const char *sinkName = managerType_ == DUP_PLAYBACK ? INNER_CAPTURER_SINK.c_str() :
        (managerType_ == DUAL_PLAYBACK ? "Speaker" : nullptr);
    uint32_t flags = PA_STREAM_ADJUST_LATENCY | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_START_CORKED |
        PA_STREAM_VARIABLE_RATE;
    if (managerType_ == DUP_PLAYBACK || managerType_ == DUAL_PLAYBACK) {
        flags |= PA_STREAM_DONT_MOVE; // should not move dup streams
    }
    int32_t result = pa_stream_connect_playback(paStream, sinkName, &bufferAttr, static_cast<pa_stream_flags_t>(flags),
        nullptr, nullptr);
    if (result < 0) {
        int32_t error = pa_context_errno(context_);
        AUDIO_ERR_LOG("connection to stream error: %{public}d -- %{public}s", error, pa_strerror(error));
        return ERR_INVALID_OPERATION;
    }
    return SUCCESS;
}

int32_t PaAdapterManager::ConnectCapturerStreamToPA(pa_stream *paStream, pa_sample_spec sampleSpec,
    SourceType source, const std::string &deviceName)
{
    uint32_t fragsize = 1; // 1 is frag size of recorder
    uint32_t maxlength = (source == SOURCE_TYPE_WAKEUP) ? PA_RECORD_MAX_LENGTH_WAKEUP : PA_RECORD_MAX_LENGTH_NORMAL;
    pa_buffer_attr bufferAttr;
    bufferAttr.maxlength = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * maxlength, &sampleSpec);
    bufferAttr.fragsize = pa_usec_to_bytes(BUF_LENGTH_IN_MSEC * PA_USEC_PER_MSEC * fragsize, &sampleSpec);
    AUDIO_INFO_LOG("bufferAttr, maxLength: %{public}d, fragsize: %{public}d",
        bufferAttr.maxlength, bufferAttr.fragsize);

    const char *cDeviceName = (deviceName == "") ? nullptr : deviceName.c_str();
    int32_t result = pa_stream_connect_record(paStream, cDeviceName, &bufferAttr,
        (pa_stream_flags_t)(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_START_CORKED |
        PA_STREAM_VARIABLE_RATE));
    // PA_STREAM_ADJUST_LATENCY exist, return peek length from server;
    if (result < 0) {
        int32_t error = pa_context_errno(context_);
        AUDIO_ERR_LOG("connection to stream error: %{public}d", error);
        return ERR_INVALID_OPERATION;
    }
    return SUCCESS;
}

int32_t PaAdapterManager::SetStreamAudioEnhanceMode(pa_stream *paStream, AudioEnhanceMode mode)
{
    PaLockGuard lock(mainLoop_);
    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed.");
        return ERROR;
    }
    std::string enhanceModeName = AUDIO_ENHANCE_SUPPORTED_SCENE_MODES.find(mode)->second;
    std::string upDevice = "DEVICE_TYPE_MIC";
    std::string downDevice = "DEVICE_TYPE_SPEAKER";
    pa_proplist_sets(propList, "device.up", upDevice.c_str());
    pa_proplist_sets(propList, "device.down", downDevice.c_str());
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    if (updatePropOperation == nullptr) {
        AUDIO_ERR_LOG("pa_stream_proplist_update failed.");
        return ERROR;
    }
    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);
    return SUCCESS;
}

const std::string PaAdapterManager::GetEnhanceModeName(AudioEnhanceMode mode)
{
    std::string name;
    switch (mode) {
        case AudioEnhanceMode::ENHANCE_NONE:
            name = "ENHANCE_NONE";
            break;
        case AudioEnhanceMode::ENHANCE_DEFAULT:
            name = "ENHANCE_DEFAULT";
            break;
        default:
            name = "ENHANCE_NONE";
            break;
    }
    const std::string modeName = name;
    return modeName;
}

void PaAdapterManager::PAStreamUpdateStreamIndexSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    AUDIO_DEBUG_LOG("PAStreamUpdateStreamIndexSuccessCb in");
}

void PaAdapterManager::PAContextStateCb(pa_context *context, void *userdata)
{
    pa_threaded_mainloop *mainLoop = reinterpret_cast<pa_threaded_mainloop *>(userdata);
    AUDIO_INFO_LOG("Current Context State: %{public}d", pa_context_get_state(context));
    ScheduleReportData(getpid(), gettid(), "audio_server");

    switch (pa_context_get_state(context)) {
        case PA_CONTEXT_READY:
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
    }
}

void PaAdapterManager::PAStreamStateCb(pa_stream *stream, void *userdata)
{
    if (!userdata) {
        AUDIO_ERR_LOG("PAStreamStateCb: userdata is null");
        return;
    }
    PaAdapterManager *adapterManger = reinterpret_cast<PaAdapterManager *>(userdata);
    AUDIO_INFO_LOG("Current Stream State: %{public}d", pa_stream_get_state(stream));
    switch (pa_stream_get_state(stream)) {
        case PA_STREAM_READY:
        case PA_STREAM_FAILED:
        case PA_STREAM_TERMINATED:
            pa_threaded_mainloop_signal(adapterManger->mainLoop_, 0);
            break;
        case PA_STREAM_UNCONNECTED:
        case PA_STREAM_CREATING:
        default:
            break;
    }
}

const std::string PaAdapterManager::GetStreamName(AudioStreamType audioType)
{
    std::string name = "unknown";
    if (STREAM_TYPE_ENUM_STRING_MAP.find(audioType) != STREAM_TYPE_ENUM_STRING_MAP.end()) {
        name = STREAM_TYPE_ENUM_STRING_MAP.at(audioType);
    } else {
        AUDIO_ERR_LOG("GetStreamName: Invalid stream type [%{public}d], return unknown", audioType);
    }
    const std::string streamName = name;
    return streamName;
}

pa_sample_spec PaAdapterManager::ConvertToPAAudioParams(AudioProcessConfig processConfig)
{
    pa_sample_spec paSampleSpec;
    paSampleSpec.channels = processConfig.streamInfo.channels;
    paSampleSpec.rate = processConfig.streamInfo.samplingRate;
    switch (processConfig.streamInfo.format) {
        case SAMPLE_U8:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_U8;
            break;
        case SAMPLE_S16LE:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_S16LE;
            break;
        case SAMPLE_S24LE:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_S24LE;
            break;
        case SAMPLE_S32LE:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_S32LE;
            break;
        default:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_INVALID;
            break;
    }
    return paSampleSpec;
}


uint32_t PaAdapterManager::ConvertChLayoutToPaChMap(const uint64_t &channelLayout, pa_channel_map &paMap)
{
    if (channelLayout == CH_LAYOUT_MONO) {
        pa_channel_map_init_mono(&paMap);
        return AudioChannel::MONO;
    }
    uint32_t channelNum = 0;
    uint64_t mode = (channelLayout & CH_MODE_MASK) >> CH_MODE_OFFSET;
    switch (mode) {
        case 0: {
            for (auto bit = chSetToPaPositionMap.begin(); bit != chSetToPaPositionMap.end(); ++bit) {
                if (channelNum >= PA_CHANNELS_MAX) {
                    return 0;
                }
                if ((channelLayout & (bit->first)) != 0) {
                    paMap.map[channelNum++] = bit->second;
                }
            }
            break;
        }
        case 1: {
            uint64_t order = (channelLayout & CH_HOA_ORDNUM_MASK) >> CH_HOA_ORDNUM_OFFSET;
            channelNum = (order + 1) * (order + 1);
            if (channelNum > PA_CHANNELS_MAX) {
                return 0;
            }
            for (uint32_t i = 0; i < channelNum; ++i) {
                paMap.map[i] = chSetToPaPositionMap[FRONT_LEFT];
            }
            break;
        }
        default:
            channelNum = 0;
            break;
    }
    return channelNum;
}

const std::string PaAdapterManager::GetEnhanceSceneName(SourceType sourceType)
{
    std::string name;
    switch (sourceType) {
        case SOURCE_TYPE_MIC:
            name = "SCENE_RECORD";
            break;
        case SOURCE_TYPE_VOICE_CALL:
        case SOURCE_TYPE_VOICE_COMMUNICATION:
            name = "SCENE_VOIP_3A";
            break;
        default:
            name = "SCENE_OTHERS";
    }
    const std::string sceneName = name;
    return sceneName;
}

} // namespace AudioStandard
} // namespace OHOS
