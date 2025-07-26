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
#define LOG_TAG "RemoteAudioRenderSink"
#endif

#include "sink/remote_audio_render_sink.h"
#include <sstream>
#include <climits>
#include <utility>
#include <algorithm>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "audio_performance_monitor.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

namespace OHOS {
namespace AudioStandard {
const std::unordered_map<std::string, AudioCategory> RemoteAudioRenderSink::SPLIT_STREAM_MAP = {
    { std::string(MEDIA_STREAM_TYPE), AudioCategory::AUDIO_IN_MEDIA },
    { std::string(NAVIGATION_STREAM_TYPE), AudioCategory::AUDIO_IN_NAVIGATION },
    { std::string(COMMUNICATION_STREAM_TYPE), AudioCategory::AUDIO_IN_COMMUNICATION },
};

RemoteAudioRenderSink::RemoteAudioRenderSink(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId)
{
    AUDIO_DEBUG_LOG("construction");
}

RemoteAudioRenderSink::~RemoteAudioRenderSink()
{
    if (sinkInited_.load()) {
        DeInit();
    }
    AUDIO_DEBUG_LOG("destruction");
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(ADAPTER_TYPE_REMOTE);
}

int32_t RemoteAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    attr_ = attr;
    std::vector<AudioCategory> splitStreamVector;
    InitSplitStream(attr_.aux.c_str(), splitStreamVector);
    std::unique_lock<std::shared_mutex> wrapperLock(renderWrapperMutex_);
    for (auto &splitStream : splitStreamVector) {
        audioRenderWrapperMap_[splitStream] = {};
    }
    sinkInited_.store(true);
    return SUCCESS;
}

void RemoteAudioRenderSink::DeInit(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("RemoteAudioRenderSink::DeInit");
    AUDIO_INFO_LOG("in");

    JoinStartThread();

    sinkInited_.store(false);
    renderInited_.store(false);
    started_.store(false);
    paused_.store(false);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN(deviceManager != nullptr);
    std::unique_lock<std::shared_mutex> wrapperLock(renderWrapperMutex_);
    for (auto &it : audioRenderWrapperMap_) {
        deviceManager->DestroyRender(deviceNetworkId_, it.second.hdiRenderId_);
        deviceManager->UnRegistRenderSinkCallback(deviceNetworkId_, it.second.hdiRenderId_);
        it.second.audioRender_.ForceSetRefPtr(nullptr);
        DumpFileUtil::CloseDumpFile(&it.second.dumpFile_);
    }
    audioRenderWrapperMap_.clear();
    AUDIO_INFO_LOG("end");
}

bool RemoteAudioRenderSink::IsInited(void)
{
    return sinkInited_.load();
}

void RemoteAudioRenderSink::JoinStartThread()
{
    std::lock_guard<std::mutex> lock(threadMutex_);
    if (startThread_ != nullptr) {
        if (startThread_->joinable()) {
            startThread_->join();
        }
        startThread_ = nullptr;
    }
}

int32_t RemoteAudioRenderSink::Start(void)
{
    Trace trace("RemoteAudioRenderSink::Start");
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(createRenderMutex_);

    std::shared_lock<std::shared_mutex> wrapperLock(renderWrapperMutex_);
    for (auto &it : audioRenderWrapperMap_) {
        it.second.dumpFileName_ = std::string(DUMP_REMOTE_RENDER_SINK_FILENAME) + "_" + std::to_string(it.first) +
            '_' + GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel) + "_" +
            std::to_string(attr_.format) + ".pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, it.second.dumpFileName_ + std::to_string(it.first) +
            ".pcm", &(it.second.dumpFile_));
    }
    wrapperLock.unlock();

    CHECK_AND_RETURN_RET_LOG(sinkInited_.load(), ERR_ILLEGAL_STATE, "not inited");

    if (isThreadRunning_.load()) {
        AUDIO_INFO_LOG("SubThread is already running");
        return SUCCESS;
    }

    JoinStartThread();
    isThreadRunning_.store(true);

    std::lock_guard<std::mutex> threadLock(threadMutex_);
    startThread_ = std::make_shared<std::thread>([this]() {
        AUDIO_INFO_LOG("SubThread Start");
        if (!renderInited_.load()) {
            std::unique_lock<std::shared_mutex> wrapperLock(renderWrapperMutex_);
            for (auto &it : audioRenderWrapperMap_) {
                int32_t ret = CreateRender(it.first);
                CHECK_AND_RETURN_LOG(ret == SUCCESS, "create render fail");
                renderInited_.store(true);
            }
        }
    
        if (started_.load()) {
            AUDIO_INFO_LOG("already started");
            isThreadRunning_.store(false);
            return;
        }
    
        std::shared_lock<std::shared_mutex> wrapperLock(renderWrapperMutex_);
        for (auto &it : audioRenderWrapperMap_) {
            CHECK_AND_RETURN_LOG(it.second.audioRender_ != nullptr,
                "render is nullptr, type: %{public}d", it.first);
            int32_t ret = it.second.audioRender_->Start();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "start fail, type: %{public}d, ret: %{public}d", it.first, ret);
        }
        started_.store(true);
        isThreadRunning_.store(false);
        AUDIO_INFO_LOG("SubThread End");
    });

    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_REMOTE, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::Stop(void)
{
    Trace trace("RemoteAudioRenderSink::Stop");
    AUDIO_INFO_LOG("in");

    JoinStartThread();

    if (!started_.load()) {
        AUDIO_INFO_LOG("already stopped");
        return SUCCESS;
    }

    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (auto &it : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(it.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", it.first);
        int32_t ret = it.second.audioRender_->Stop();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail, type: %{public}d, ret: %{public}d",
            it.first, ret);
    }
    started_.store(false);
    isThreadRunning_.store(false);
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::Resume(void)
{
    Trace trace("RemoteAudioRenderSink::Resume");
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("already resumed");
        return SUCCESS;
    }

    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (auto &it : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(it.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", it.first);
        int32_t ret = it.second.audioRender_->Resume();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "resume fail, type: %{public}d, ret: %{public}d",
            it.first, ret);
    }
    paused_.store(false);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_REMOTE, INIT_LASTWRITTEN_TIME);
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::Pause(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    if (paused_.load()) {
        AUDIO_INFO_LOG("already paused");
        return SUCCESS;
    }

    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (auto &it : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(it.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", it.first);
        int32_t ret = it.second.audioRender_->Pause();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "pause fail, type: %{public}d, ret: %{public}d",
            it.first, ret);
    }
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::Flush(void)
{
    Trace trace("RemoteAudioRenderSink::Flush");
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (auto &it : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(it.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", it.first);
        int32_t ret = it.second.audioRender_->Flush();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "flush fail, type: %{public}d, ret: %{public}d",
            it.first, ret);
    }
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::Reset(void)
{
    Trace trace("RemoteAudioRenderSink::Reset");
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "not start, invalid state");

    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (auto &it : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(it.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", it.first);
        int32_t ret = it.second.audioRender_->Flush();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "reset fail, type: %{public}d, ret: %{public}d",
            it.first, ret);
    }
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    Trace trace("RemoteAudioRenderSink::RenderFrame");
    AUDIO_DEBUG_LOG("in");
    return RenderFrame(data, len, writeLen, AUDIO_IN_MEDIA);
}

int64_t RemoteAudioRenderSink::GetVolumeDataCount()
{
    return volumeDataCount_;
}

int32_t RemoteAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void RemoteAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
}

std::string RemoteAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t RemoteAudioRenderSink::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load(), ERR_ILLEGAL_STATE, "not create, invalid state");
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

    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (const auto &wrapper : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(wrapper.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", wrapper.first);
        int32_t ret = wrapper.second.audioRender_->SetVolume(volume);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "set volume fail, ret: %{public}d", ret);
    }
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::GetLatency(uint32_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load(), ERR_ILLEGAL_STATE, "not create, invalid state");
    uint32_t hdiLatency = 0;
    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (const auto &wrapper : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(wrapper.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", wrapper.first);
        int32_t ret = wrapper.second.audioRender_->GetLatency(hdiLatency);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail, ret: %{public}d", ret);
    }
    latency = hdiLatency;
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float RemoteAudioRenderSink::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

void RemoteAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void RemoteAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load(), ERR_ILLEGAL_STATE, "not create, invalid state");
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");

    int32_t ret = DoSetOutputRoute();
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("update route fail, ret: %{public}d", ret);
    }

    struct AudioSceneDescriptor sceneDesc = {
        .scene.id = GetAudioCategory(audioScene),
        .desc.pins = AudioPortPin::PIN_OUT_SPEAKER,
    };
    AUDIO_DEBUG_LOG("start");
    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    for (const auto &wrapper : audioRenderWrapperMap_) {
        CHECK_AND_RETURN_RET_LOG(wrapper.second.audioRender_ != nullptr, ERR_INVALID_HANDLE,
            "render is nullptr, type: %{public}d", wrapper.first);
        ret = wrapper.second.audioRender_->SelectScene(sceneDesc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
    }
    AUDIO_DEBUG_LOG("end, audioScene: %{public}d", audioScene);
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void RemoteAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

void RemoteAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRenderSink::SplitRenderFrame(char &data, uint64_t len, uint64_t &writeLen, const char *streamType)
{
    Trace trace("RemoteAudioRenderSink::SplitRenderFrame");
    AUDIO_DEBUG_LOG("in, type: %{public}s", streamType);
    auto it = SPLIT_STREAM_MAP.find(streamType);
    CHECK_AND_RETURN_RET_LOG(it != SPLIT_STREAM_MAP.end(), ERR_INVALID_PARAM, "invalid stream type");
    return RenderFrame(data, len, writeLen, it->second);
}

void RemoteAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: RemoteSink\tstarted: " + std::string(started_.load() ? "true" : "false") +
        "\tdeviceNetworkId: " + deviceNetworkId_ + "\n";
}

void RemoteAudioRenderSink::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        DeInit();
    }

    callback_.OnRenderSinkParamChange(adapterName, key, condition, value);
}

void RemoteAudioRenderSink::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

AudioFormat RemoteAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
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

AudioCategory RemoteAudioRenderSink::GetAudioCategory(AudioScene audioScene)
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

void RemoteAudioRenderSink::InitSplitStream(const char *splitStreamStr, std::vector<AudioCategory> &splitStreamVector)
{
    AUDIO_INFO_LOG("splitStreamStr: %{public}s", splitStreamStr);
    if (splitStreamStr == nullptr || strlen(splitStreamStr) == 0) {
        splitStreamVector.push_back(AudioCategory::AUDIO_IN_MEDIA);
        AUDIO_INFO_LOG("split stream use default 1");
        return;
    }

    std::istringstream iss(splitStreamStr);
    std::string currentSplitStream;
    std::vector<std::string> splitStreamStrVector;
    while (getline(iss, currentSplitStream, ':')) {
        splitStreamStrVector.push_back(currentSplitStream);
        AUDIO_INFO_LOG("current split stream type is %{public}s", currentSplitStream.c_str());
    }
    sort(splitStreamStrVector.begin(), splitStreamStrVector.end());
    for (auto &splitStream : splitStreamStrVector) {
        auto it = SPLIT_STREAM_MAP.find(splitStream);
        if (it == SPLIT_STREAM_MAP.end()) {
            AUDIO_ERR_LOG("invalid stream type %{public}s", splitStream.c_str());
            continue;
        }
        splitStreamVector.push_back(it->second);
    }
}

void RemoteAudioRenderSink::InitAudioSampleAttr(AudioSampleAttributes &param, AudioCategory type)
{
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = 0;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE));
    param.type = AUDIO_IN_MEDIA;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }

    param.type = type;
}

void RemoteAudioRenderSink::InitDeviceDesc(AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = AudioPortPin::PIN_OUT_SPEAKER;
    deviceDesc.desc = "";
}

int32_t RemoteAudioRenderSink::CreateRender(AudioCategory type)
{
    int64_t stamp = ClockTime::GetCurNano();

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param, type);
    InitDeviceDesc(deviceDesc);
    struct RenderWrapper &wrapper = audioRenderWrapperMap_[type];

    AUDIO_INFO_LOG("create render, format: %{public}u", param.format);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(deviceNetworkId_, &param, &deviceDesc, wrapper.hdiRenderId_);
    wrapper.audioRender_.ForceSetRefPtr(static_cast<IAudioRender *>(render));
    CHECK_AND_RETURN_RET(wrapper.audioRender_ != nullptr, ERR_NOT_STARTED);
    deviceManager->RegistRenderSinkCallback(deviceNetworkId_, wrapper.hdiRenderId_, this);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("create render success, cost: [%{public}" PRId64 "]ms", stamp);
    return SUCCESS;
}

int32_t RemoteAudioRenderSink::DoSetOutputRoute(void)
{
    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    if (audioRenderWrapperMap_.find(AUDIO_IN_MEDIA) == audioRenderWrapperMap_.end()) {
        AUDIO_WARNING_LOG("render not include AUDIO_IN_MEDIA");
        return ERR_INVALID_HANDLE;
    }
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    int32_t ret = deviceManager->SetOutputRoute(deviceNetworkId_, { DEVICE_TYPE_SPEAKER }, static_cast<int32_t>(
        GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_REMOTE)));
    return ret;
}

void RemoteAudioRenderSink::CheckUpdateState(char *data, uint64_t len)
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
                maxAmplitude_ = 0;
            }
        }
    }
}

int32_t RemoteAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen, AudioCategory type)
{
    CHECK_AND_RETURN_RET_LOG(renderInited_.load(), ERR_ILLEGAL_STATE, "not create, invalid state");
    AUDIO_DEBUG_LOG("type: %{public}d", type);
    int64_t stamp = ClockTime::GetCurNano();
    std::shared_lock<std::shared_mutex> lock(renderWrapperMutex_);
    sptr<IAudioRender> audioRender = audioRenderWrapperMap_[type].audioRender_;
    CHECK_AND_RETURN_RET_LOG(audioRender != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    if (!started_.load()) {
        AUDIO_WARNING_LOG("not start, invalid state");
    }

    std::vector<int8_t> bufferVec(len);
    int32_t ret = memcpy_s(bufferVec.data(), len, &data, len);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "copy fail, error code: %{public}d", ret);

    BufferDesc buffer = { reinterpret_cast<uint8_t *>(&data), len, len };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_ + std::to_string(type), volumeDataCount_);
    Trace trace("RemoteAudioRenderSink::RenderFrame inner renderFrame");
    Trace::CountVolume("RemoteAudioRenderSink::RenderFrame", static_cast<uint8_t>(data));
    ret = audioRender->RenderFrame(bufferVec, writeLen);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(ADAPTER_TYPE_REMOTE, ClockTime::GetCurNano());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "fail, ret: %{public}x", ret);
    writeLen = len;
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        FILE *dumpFile = audioRenderWrapperMap_[type].dumpFile_;
        DumpFileUtil::WriteDumpFile(dumpFile, static_cast<void *>(&data), len);
        std::string dumpFileName = audioRenderWrapperMap_[type].dumpFileName_;
        AudioCacheMgr::GetInstance().CacheData(dumpFileName, static_cast<void *>(&data), len);
    }
    CheckUpdateState(&data, len);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    int64_t stampThreshold = 50; // 50ms
    if (stamp >= stampThreshold) {
        AUDIO_WARNING_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    }
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
