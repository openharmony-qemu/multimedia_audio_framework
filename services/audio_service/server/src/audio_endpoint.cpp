/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEndpointInner"
#endif

#include "audio_endpoint.h"
#include "audio_endpoint_private.h"


#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_schedule.h"
#include "audio_qosmanager.h"
#include "audio_utils.h"
#include "manager/hdi_adapter_manager.h"
#include "format_converter.h"
#include "policy_handler.h"
#include "media_monitor_manager.h"
#include "volume_tools.h"
#include "audio_dump_pcm.h"
#include "audio_performance_monitor.h"
#include "audio_service.h"
#ifdef RESSCHE_ENABLE
#include "res_type.h"
#include "res_sched_client.h"
#endif
#include "audio_volume.h"
#include "audio_stream_monitor.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
    static constexpr int64_t RECORD_DELAY_TIME_NS = 4000000; // 4ms = 4 * 1000 * 1000ns
    static constexpr int64_t RECORD_VOIP_DELAY_TIME_NS = 20000000; // 20ms = 20 * 1000 * 1000ns
    static constexpr int64_t MAX_SPAN_DURATION_NS = 100000000; // 100ms = 100 * 1000 * 1000ns
    static constexpr int64_t PLAYBACK_DELAY_STOP_HDI_TIME_NS = 3000000000; // 3s = 3 * 1000 * 1000 * 1000ns
    static constexpr int64_t RECORDER_DELAY_STOP_HDI_TIME_NS = 200000000; // 200ms = 200 * 1000 * 1000ns
    static constexpr int64_t LINK_RECORDER_DELAY_STOP_HDI_TIME_NS = 2000000000; // 2000ms = 2000 * 1000 * 1000ns
    static constexpr int64_t WAIT_CLIENT_STANDBY_TIME_NS = 1000000000; // 1s = 1000 * 1000 * 1000ns
    static constexpr int64_t DELAY_STOP_HDI_TIME_WHEN_NO_RUNNING_NS = 1000000000; // 1s
    static constexpr int32_t SLEEP_TIME_IN_DEFAULT = 400; // 400ms
    static constexpr int64_t DELTA_TO_REAL_READ_START_TIME = 0; // 0ms
    const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 40;
    static const int32_t START_DEVICE_TIMEOUT = 10; // 10s
    static const int32_t ONE_MINUTE = 60;
    const int32_t DUP_COMMON_LEN = 40; // 40 -> 40ms
    const int32_t DUP_DEFAULT_LEN = 20; // 20 -> 20ms
}

std::string AudioEndpoint::GenerateEndpointKey(AudioDeviceDescriptor &deviceInfo, int32_t endpointFlag)
{
    // All primary sinks share one endpoint
    int32_t endpointId = 0;
    if (deviceInfo.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        endpointId = deviceInfo.deviceId_;
    }
    return deviceInfo.networkId_ + "_" + std::to_string(endpointId) + "_" +
        std::to_string(deviceInfo.deviceRole_) + "_" + std::to_string(endpointFlag);
}

std::shared_ptr<AudioEndpoint> AudioEndpoint::CreateEndpoint(EndpointType type, uint64_t id,
    const AudioProcessConfig &clientConfig, const AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo)
{
    std::shared_ptr<AudioEndpoint> audioEndpoint = nullptr;
    audioEndpoint = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    CHECK_AND_RETURN_RET_LOG(audioEndpoint != nullptr, nullptr, "Create AudioEndpoint failed.");

    if (!audioEndpoint->Config(deviceInfo, streamInfo)) {
        AUDIO_ERR_LOG("Config AudioEndpoint failed!");
        audioEndpoint = nullptr;
    }
    return audioEndpoint;
}

AudioDeviceDescriptor &AudioEndpoint::GetDeviceInfo()
{
    return deviceInfo_;
}

DeviceRole AudioEndpoint::GetDeviceRole()
{
    return deviceInfo_.deviceRole_;
}

AudioStreamInfo &AudioEndpoint::GetAudioStreamInfo()
{
    return dstStreamInfo_;
}

AudioEndpointInner::AudioEndpointInner(EndpointType type, uint64_t id,
    const AudioProcessConfig &clientConfig) : endpointType_(type), id_(id), clientConfig_(clientConfig)
{
    AUDIO_INFO_LOG("AudioEndpoint type:%{public}d", endpointType_);
    if (clientConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        logUtilsTag_ = "AudioEndpoint::Play";
    } else {
        logUtilsTag_ = "AudioEndpoint::Rec";
    }
}

std::string AudioEndpointInner::GetEndpointName()
{
    return GenerateEndpointKey(deviceInfo_, id_);
}

int32_t AudioEndpointInner::SetVolume(AudioStreamType streamType, float volume)
{
    if (streamType == AudioStreamType::STREAM_VOICE_CALL && endpointType_ == TYPE_VOIP_MMAP) {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        if (sink != nullptr) {
            AUDIO_INFO_LOG("SetVolume:%{public}f, streamType:%{public}d", volume, streamType);
            sink->SetVolume(volume, volume);
        }
    }
    return SUCCESS;
}

MockCallbacks::MockCallbacks(uint32_t streamIndex) : streamIndex_(streamIndex)
{
    AUDIO_INFO_LOG("DupStream %{public}u create MockCallbacks", streamIndex_);
    if (GetEngineFlag() == 1) {
        dumpDupOutFileName_ = std::to_string(streamIndex_) + "_endpoint_dup_out_" + ".pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpDupOutFileName_, &dumpDupOut_);
    }
}

MockCallbacks::~MockCallbacks()
{
    if (GetEngineFlag() == 1) {
        DumpFileUtil::CloseDumpFile(&dumpDupOut_);
    }
}

void MockCallbacks::OnStatusUpdate(IOperation operation)
{
    AUDIO_INFO_LOG("DupStream %{public}u recv operation: %{public}d", streamIndex_, operation);
}

int32_t MockCallbacks::OnWriteData(size_t length)
{
    Trace trace("DupStream::OnWriteData length " + std::to_string(length));
    return SUCCESS;
}

int32_t MockCallbacks::OnWriteData(int8_t *inputData, size_t requestDataLen)
{
    Trace trace("DupStream::OnWriteData length " + std::to_string(requestDataLen));
    if (GetEngineFlag() == 1 && dupRingBuffer_ != nullptr) {
        OptResult result = dupRingBuffer_->GetReadableSize();
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
            "dupBuffer get readable size failed, size is:%{public}zu", result.size);
        CHECK_AND_RETURN_RET_LOG((result.size != 0) && (result.size >= requestDataLen), ERROR,
            "Readable size is invaild, result.size:%{public}zu, requstDataLen:%{public}zu",
            result.size, requestDataLen);
        AUDIO_DEBUG_LOG("requstDataLen is:%{public}zu readSize is:%{public}zu", requestDataLen, result.size);
        result = dupRingBuffer_->Dequeue({reinterpret_cast<uint8_t *>(inputData), requestDataLen});
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "dupBuffer dequeue failed!");\
        DumpFileUtil::WriteDumpFile(dumpDupOut_, static_cast<void *>(inputData), requestDataLen);
    }
    return SUCCESS;
}

int32_t MockCallbacks::GetAvailableSize(size_t &length)
{
    return ERR_NOT_SUPPORTED;
}

std::unique_ptr<AudioRingCache>& MockCallbacks::GetDupRingBuffer()
{
    return dupRingBuffer_;
}

bool AudioEndpointInner::ShouldInnerCap(int32_t innerCapId)
{
    bool shouldBecapped = false;
    std::lock_guard<std::mutex> lock(listLock_);
    for (uint32_t i = 0; i < processList_.size(); i++) {
        if (processList_[i]->GetInnerCapState(innerCapId)) {
            shouldBecapped = true;
            break;
        }
    }
    AUDIO_INFO_LOG("find endpoint inner-cap state: %{public}s", shouldBecapped ? "true" : "false");
    return shouldBecapped;
}

AudioProcessConfig AudioEndpointInner::GetInnerCapConfig()
{
    AudioProcessConfig processConfig;

    processConfig.appInfo.appPid = static_cast<int32_t>(getpid());
    processConfig.appInfo.appUid = static_cast<int32_t>(getuid());

    processConfig.streamInfo = dstStreamInfo_;

    processConfig.audioMode = AUDIO_MODE_PLAYBACK;

    // processConfig.rendererInfo ?

    processConfig.streamType = STREAM_MUSIC;

    return processConfig;
}

int32_t AudioEndpointInner::InitDupStream(int32_t innerCapId)
{
    std::lock_guard<std::mutex> lock(dupMutex_);
    bool hasEnabled = (fastCaptureInfos_.count(innerCapId) && fastCaptureInfos_[innerCapId].isInnerCapEnabled);
    CHECK_AND_RETURN_RET_LOG((hasEnabled == false), SUCCESS, "already enabled");

    AudioProcessConfig processConfig = GetInnerCapConfig();
    processConfig.innerCapId = innerCapId;
    auto &captureInfo = fastCaptureInfos_[innerCapId];
    int32_t ret = IStreamManager::GetDupPlaybackManager().CreateRender(processConfig, captureInfo.dupStream);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && captureInfo.dupStream != nullptr,
        ERR_OPERATION_FAILED, "Failed: %{public}d", ret);
    uint32_t dupStreamIndex = captureInfo.dupStream->GetStreamIndex();

    innerCapIdToDupStreamCallbackMap_[innerCapId] = std::make_shared<MockCallbacks>(dupStreamIndex);
    captureInfo.dupStream->RegisterStatusCallback(innerCapIdToDupStreamCallbackMap_[innerCapId]);
    captureInfo.dupStream->RegisterWriteCallback(innerCapIdToDupStreamCallbackMap_[innerCapId]);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        ret = InitDupBuffer(processConfig, innerCapId, dupStreamIndex); // buffer init
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "InitProAudioDupBuffer failed");
    }

    // eg: /data/local/tmp/LocalDevice6_0_c2s_dup_48000_2_1.pcm
    AudioStreamInfo tempInfo = processConfig.streamInfo;
    dupDumpName_ = GetEndpointName() + "_c2s_dup_" + std::to_string(tempInfo.samplingRate) + "_" +
        std::to_string(tempInfo.channels) + "_" + std::to_string(tempInfo.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dupDumpName_, &dumpC2SDup_);

    AUDIO_INFO_LOG("Dup Renderer %{public}d with Endpoint status: %{public}s", dupStreamIndex,
        GetStatusStr(endpointStatus_).c_str());
    CHECK_AND_RETURN_RET_LOG(endpointStatus_ != INVALID, ERR_ILLEGAL_STATE, "Endpoint is invalid!");

    // buffer init
    dupBufferSize_ = dstSpanSizeInframe_ * dstByteSizePerFrame_; // each
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, ERR_OPERATION_FAILED, "DstAudioBuffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(dupBufferSize_ < dstAudioBuffer_->GetDataSize(), ERR_OPERATION_FAILED, "Init buffer fail");
    dupBuffer_ = std::make_unique<uint8_t []>(dupBufferSize_);
    ret = memset_s(reinterpret_cast<void *>(dupBuffer_.get()), dupBufferSize_, 0, dupBufferSize_);
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memset buffer fail, ret %{public}d", ret);
    }

    if (endpointStatus_ == RUNNING || (endpointStatus_ == IDEL && isDeviceRunningInIdel_)) {
        int32_t audioId = deviceInfo_.deviceId_;
        AUDIO_INFO_LOG("Endpoint %{public}d is already running, let's start the dup stream", audioId);
        captureInfo.dupStream->Start();
    }
    captureInfo.isInnerCapEnabled = true;
    float clientVolume = dstAudioBuffer_->GetStreamVolume();
    float duckFactor = dstAudioBuffer_->GetDuckFactor();
    if (engineFlag == 1 && AudioVolume::GetInstance() != nullptr) {
        AudioVolume::GetInstance()->SetStreamVolume(dupStreamIndex, clientVolume);
        AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(dupStreamIndex, duckFactor);
    }

    return SUCCESS;
}

int32_t AudioEndpointInner::InitDupBuffer(AudioProcessConfig processConfig, int32_t innerCapId,
    uint32_t dupStreamIndex)
{
    int32_t ret = CreateDupBufferInner(innerCapId);

    dumpDupInFileName_ = std::to_string(dupStreamIndex) + "_endpoint_dup_in_" + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpDupInFileName_, &dumpDupIn_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Config dup buffer failed");

    bool isSystemApp = CheckoutSystemAppUtil::CheckoutSystemApp(processConfig.appInfo.appUid);
    if (AudioVolume::GetInstance() != nullptr) {
        StreamVolumeParams streamVolumeParams = { dupStreamIndex, processConfig.streamType,
            processConfig.rendererInfo.streamUsage, processConfig.appInfo.appUid, processConfig.appInfo.appPid,
            isSystemApp, processConfig.rendererInfo.volumeMode, processConfig.rendererInfo.isVirtualKeyboard };
        AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
    }
        
    return SUCCESS;
}

int32_t AudioEndpointInner::EnableFastInnerCap(int32_t innerCapId)
{
    CHECK_AND_RETURN_RET_LOG(deviceInfo_.deviceRole_ == OUTPUT_DEVICE, ERR_INVALID_OPERATION, "Not output device!");
    int32_t ret = InitDupStream(innerCapId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Init dup stream failed!");
    return SUCCESS;
}

int32_t AudioEndpointInner::DisableFastInnerCap()
{
    if (deviceInfo_.deviceRole_ != OUTPUT_DEVICE) {
            return SUCCESS;
    }
    std::lock_guard<std::mutex> lock(dupMutex_);
    for (auto &capInfo : fastCaptureInfos_) {
        HandleDisableFastCap(capInfo.second);
    }
    fastCaptureInfos_.clear();
    return SUCCESS;
}

int32_t AudioEndpointInner::DisableFastInnerCap(int32_t innerCapId)
{
    if (deviceInfo_.deviceRole_ != OUTPUT_DEVICE) {
        return SUCCESS;
    }
    std::lock_guard<std::mutex> lock(dupMutex_);
    if (!fastCaptureInfos_.count(innerCapId)) {
        AUDIO_INFO_LOG("InnerCap is already disabled.");
        return SUCCESS;
    }
    HandleDisableFastCap(fastCaptureInfos_[innerCapId]);
    fastCaptureInfos_.erase(innerCapId);
    return SUCCESS;
}

int32_t AudioEndpointInner::HandleDisableFastCap(CaptureInfo &captureInfo)
{
    if (!captureInfo.isInnerCapEnabled) {
        captureInfo.dupStream = nullptr;
        AUDIO_INFO_LOG("InnerCap is already disabled.");
        return SUCCESS;
    }
    if (captureInfo.dupStream == nullptr) {
        captureInfo.isInnerCapEnabled = false;
        AUDIO_INFO_LOG("dupStream is nullptr.");
        return SUCCESS;
    }
    captureInfo.isInnerCapEnabled = false;
    AUDIO_INFO_LOG("Disable dup renderer %{public}d with Endpoint status: %{public}s",
        captureInfo.dupStream->GetStreamIndex(), GetStatusStr(endpointStatus_).c_str());

    if (GetEngineFlag() == 1) {
        uint32_t dupStreamIndex = captureInfo.dupStream->GetStreamIndex();
        if (AudioVolume::GetInstance() != nullptr) {
        AudioVolume::GetInstance()->RemoveStreamVolume(dupStreamIndex);
        }
    }
    IStreamManager::GetDupPlaybackManager().ReleaseRender(captureInfo.dupStream->GetStreamIndex());
    captureInfo.dupStream = nullptr;
    return SUCCESS;
}

AudioEndpoint::EndpointStatus AudioEndpointInner::GetStatus()
{
    AUDIO_INFO_LOG("AudioEndpoint get status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    return endpointStatus_.load();
}

void AudioEndpointInner::Release()
{
    // Wait for thread end and then clear other data to avoid using any cleared data in thread.
    AUDIO_INFO_LOG("Release enter.");
    if (!isInited_.load()) {
        AUDIO_WARNING_LOG("already released");
        return;
    }

    isInited_.store(false);
    workThreadCV_.notify_all();
    if (endpointWorkThread_.joinable()) {
        AUDIO_DEBUG_LOG("AudioEndpoint join work thread start");
        endpointWorkThread_.join();
        AUDIO_DEBUG_LOG("AudioEndpoint join work thread end");
    }
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(ADAPTER_TYPE_FAST);

    stopUpdateThread_.store(true);
    updateThreadCV_.notify_all();
    if (updatePosTimeThread_.joinable()) {
        AUDIO_DEBUG_LOG("AudioEndpoint join update thread start");
        updatePosTimeThread_.join();
        AUDIO_DEBUG_LOG("AudioEndpoint join update thread end");
    }

    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
    if (sink != nullptr) {
        sink->DeInit();
    }
    HdiAdapterManager::GetInstance().ReleaseId(fastRenderId_);

    if (source != nullptr) {
        source->DeInit();
    }
    HdiAdapterManager::GetInstance().ReleaseId(fastCaptureId_);

    endpointStatus_.store(INVALID);

    if (dstAudioBuffer_ != nullptr) {
        AUDIO_INFO_LOG("Set device buffer null");
        dstAudioBuffer_ = nullptr;
    }

    if (deviceInfo_.deviceRole_ == OUTPUT_DEVICE) {
        DisableFastInnerCap();
    }

    DumpFileUtil::CloseDumpFile(&dumpHdi_);
}

AudioEndpointInner::~AudioEndpointInner()
{
    if (isInited_.load()) {
        AudioEndpointInner::Release();
    }
    AUDIO_INFO_LOG("~AudioEndpoint()");
}

bool AudioEndpointInner::ConfigInputPoint(const AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("ConfigInputPoint enter.");
    IAudioSourceAttr attr = {};
    attr.sampleRate = dstStreamInfo_.samplingRate;
    attr.channel = dstStreamInfo_.channels;
    attr.format = dstStreamInfo_.format;
    attr.deviceNetworkId = deviceInfo.networkId_.c_str();
    attr.deviceType = deviceInfo.deviceType_;
    attr.audioStreamFlag = endpointType_ == TYPE_VOIP_MMAP ? AUDIO_FLAG_VOIP_FAST : AUDIO_FLAG_MMAP;

    std::shared_ptr<IAudioCaptureSource> source = GetFastSource(deviceInfo.networkId_, endpointType_, attr);

    if (deviceInfo.networkId_ == LOCAL_NETWORK_ID) {
        attr.adapterName = "primary";
    } else {
#ifdef DAUDIO_ENABLE
        attr.adapterName = "remote";
#endif
    }
    if (source == nullptr) {
        AUDIO_ERR_LOG("ConfigInputPoint GetInstance failed.");
        HdiAdapterManager::GetInstance().ReleaseId(fastCaptureId_);
        return false;
    }

    if (!source->IsInited()) {
        AUDIO_INFO_LOG("Source is not inited");
        int32_t err = source->Init(attr);
        if (err != SUCCESS || !source->IsInited()) {
            AUDIO_ERR_LOG("init remote fast fail, err %{public}d.", err);
            HdiAdapterManager::GetInstance().ReleaseId(fastCaptureId_);
            return false;
        }
    }

    if (PrepareDeviceBuffer(deviceInfo) != SUCCESS) {
        source->DeInit();
        HdiAdapterManager::GetInstance().ReleaseId(fastCaptureId_);
        return false;
    }

    bool ret = writeTimeModel_.ConfigSampleRate(dstStreamInfo_.samplingRate);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "Config LinearPosTimeModel failed.");

    endpointStatus_ = UNLINKED;
    isInited_.store(true);
    endpointWorkThread_ = std::thread([this] { this->RecordEndpointWorkLoopFuc(); });
    pthread_setname_np(endpointWorkThread_.native_handle(), "OS_AudioEpLoop");

    updatePosTimeThread_ = std::thread([this] { this->AsyncGetPosTime(); });
    pthread_setname_np(updatePosTimeThread_.native_handle(), "OS_AudioEpUpdate");

    // eg: input_endpoint_hdi_audio_8_0_20240527202236189_48000_2_1.pcm
    dumpHdiName_ = "input_endpoint_hdi_audio_" + std::to_string(attr.deviceType) + '_' +
        std::to_string(endpointType_) + '_' + GetTime() + '_' + std::to_string(attr.sampleRate) + "_" +
        std::to_string(attr.channel) + "_" + std::to_string(attr.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpHdiName_, &dumpHdi_);
    return true;
}

static std::shared_ptr<IAudioCaptureSource> SwitchSource(uint32_t &id, HdiIdType type, const std::string &info)
{
    if (id != HDI_INVALID_ID) {
        HdiAdapterManager::GetInstance().ReleaseId(id);
    }
    id = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, type, info, true);
    return HdiAdapterManager::GetInstance().GetCaptureSource(id, true);
}

std::shared_ptr<IAudioCaptureSource> AudioEndpointInner::GetFastSource(const std::string &networkId, EndpointType type,
    IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("Network id %{public}s, endpoint type %{public}d", networkId.c_str(), type);
    if (networkId != LOCAL_NETWORK_ID) {
        attr.adapterName = "remote";
#ifdef DAUDIO_ENABLE
        fastSourceType_ = type == AudioEndpoint::TYPE_MMAP ? FAST_SOURCE_TYPE_REMOTE : FAST_SOURCE_TYPE_VOIP;
        // Distributed only requires a singleton because there won't be both voip and regular fast simultaneously
        return SwitchSource(fastCaptureId_, HDI_ID_TYPE_REMOTE_FAST, networkId);
#endif
    }

    attr.adapterName = "primary";
    if (type == AudioEndpoint::TYPE_MMAP) {
        AUDIO_INFO_LOG("Use mmap");
        fastSourceType_ = FAST_SOURCE_TYPE_NORMAL;
        return SwitchSource(fastCaptureId_, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT);
    } else if (type == AudioEndpoint::TYPE_VOIP_MMAP) {
        AUDIO_INFO_LOG("Use voip mmap");
        fastSourceType_ = FAST_SOURCE_TYPE_VOIP;
        SwitchSource(fastCaptureId_, HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP);
    }
    return nullptr;
}

void AudioEndpointInner::StartThread(const IAudioSinkAttr &attr)
{
    endpointStatus_ = UNLINKED;
    isInited_.store(true);
    endpointWorkThread_ = std::thread([this] { this->EndpointWorkLoopFuc(); });
    pthread_setname_np(endpointWorkThread_.native_handle(), "OS_AudioEpLoop");

    updatePosTimeThread_ = std::thread([this] { this->AsyncGetPosTime(); });
    pthread_setname_np(updatePosTimeThread_.native_handle(), "OS_AudioEpUpdate");

    // eg: endpoint_hdi_audio_8_0_20240527202236189_48000_2_1.pcm
    dumpHdiName_ = "endpoint_hdi_audio_" + std::to_string(attr.deviceType) + '_' + std::to_string(endpointType_) +
        '_' + GetTime() + '_' + std::to_string(attr.sampleRate) + "_" +
        std::to_string(attr.channel) + "_" + std::to_string(attr.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpHdiName_, &dumpHdi_);
}

bool AudioEndpointInner::Config(const AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo)
{
    AUDIO_INFO_LOG("Role %{public}d, format %{public}d", deviceInfo.deviceRole_,
        streamInfo.format);
    deviceInfo_ = deviceInfo;
    dstStreamInfo_ = streamInfo;

    if (deviceInfo.deviceRole_ == INPUT_DEVICE) {
        return ConfigInputPoint(deviceInfo);
    }

    std::shared_ptr<IAudioRenderSink> sink = GetFastSink(deviceInfo, endpointType_);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("Get fastSink instance failed");
        HdiAdapterManager::GetInstance().ReleaseId(fastRenderId_);
        return false;
    }

    IAudioSinkAttr attr = {};
    InitSinkAttr(attr, deviceInfo);

    if (!sink->IsInited()) {
        AUDIO_INFO_LOG("Sink is not inited");
        sink->Init(attr);
        if (!sink->IsInited()) {
            HdiAdapterManager::GetInstance().ReleaseId(fastRenderId_);
            return false;
        }
    }

    if (PrepareDeviceBuffer(deviceInfo) != SUCCESS) {
        sink->DeInit();
        HdiAdapterManager::GetInstance().ReleaseId(fastRenderId_);
        return false;
    }

    Volume vol = {true, 1.0f, 0};
    DeviceType deviceType = PolicyHandler::GetInstance().GetActiveOutPutDevice();
    if ((clientConfig_.streamType == STREAM_VOICE_COMMUNICATION || clientConfig_.streamType == STREAM_VOICE_CALL) &&
        endpointType_ == TYPE_VOIP_MMAP) {
        PolicyHandler::GetInstance().GetSharedVolume(STREAM_VOICE_CALL, deviceType, vol);
        sink->SetVolume(vol.volumeFloat, vol.volumeFloat);
        AUDIO_INFO_LOG("Init Volume %{public}f with Device %{public}d", vol.volumeFloat, deviceType);
    } else {
        sink->SetVolume(1.0f, 1.0f);
        AUDIO_INFO_LOG("Init Volume 1.0 with Device %{public}d", deviceType);
    }

    bool ret = readTimeModel_.ConfigSampleRate(dstStreamInfo_.samplingRate);
    CHECK_AND_RETURN_RET_LOG(ret != false, false, "Config LinearPosTimeModel failed.");
    StartThread(attr);
    return true;
}

static std::shared_ptr<IAudioRenderSink> SwitchSink(uint32_t &id, HdiIdType type, const std::string &info)
{
    AUDIO_INFO_LOG("Id: %{public}u", id);
    if (id != HDI_INVALID_ID) {
        HdiAdapterManager::GetInstance().ReleaseId(id);
    }
    id = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, type, info, true);
    AUDIO_INFO_LOG("Id after process: %{public}u", id);
    return HdiAdapterManager::GetInstance().GetRenderSink(id, true);
}

std::shared_ptr<IAudioRenderSink> AudioEndpointInner::GetFastSink(const AudioDeviceDescriptor &deviceInfo,
    EndpointType type)
{
    AUDIO_INFO_LOG("Network id %{public}s, endpoint type %{public}d", deviceInfo.networkId_.c_str(), type);
    if (deviceInfo.networkId_ != LOCAL_NETWORK_ID) {
#ifdef DAUDIO_ENABLE
        fastSinkType_ = type == AudioEndpoint::TYPE_MMAP ? FAST_SINK_TYPE_REMOTE : FAST_SINK_TYPE_VOIP;
        // Distributed only requires a singleton because there won't be both voip and regular fast simultaneously
        return SwitchSink(fastRenderId_, HDI_ID_TYPE_REMOTE_FAST, deviceInfo.networkId_);
#endif
    }

    if (deviceInfo.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP && deviceInfo.a2dpOffloadFlag_ != A2DP_OFFLOAD) {
        fastSinkType_ = FAST_SINK_TYPE_BLUETOOTH;
        return SwitchSink(fastRenderId_, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_MMAP);
    }

    if (type == AudioEndpoint::TYPE_MMAP) {
        fastSinkType_ = FAST_SINK_TYPE_NORMAL;
        return SwitchSink(fastRenderId_, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT);
    } else if (type == AudioEndpoint::TYPE_VOIP_MMAP) {
        fastSinkType_ = FAST_SINK_TYPE_VOIP;
        return SwitchSink(fastRenderId_, HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP);
    }
    return nullptr;
}

void AudioEndpointInner::InitSinkAttr(IAudioSinkAttr &attr, const AudioDeviceDescriptor &deviceInfo)
{
    bool isDefaultAdapterEnable = AudioService::GetInstance()->GetDefaultAdapterEnable();
    if (isDefaultAdapterEnable) {
        attr.adapterName = "dp";
    } else {
        attr.adapterName = deviceInfo.networkId_ == LOCAL_NETWORK_ID ? "primary" : "remote";
    }
    attr.sampleRate = dstStreamInfo_.samplingRate; // 48000hz
    attr.channel = dstStreamInfo_.channels; // STEREO = 2
    attr.format = dstStreamInfo_.format; // SAMPLE_S16LE = 1
    attr.deviceNetworkId = deviceInfo.networkId_.c_str();
    attr.deviceType = static_cast<int32_t>(deviceInfo.deviceType_);
    attr.audioStreamFlag = endpointType_ == TYPE_VOIP_MMAP ? AUDIO_FLAG_VOIP_FAST : AUDIO_FLAG_MMAP;
}

int32_t AudioEndpointInner::GetAdapterBufferInfo(const AudioDeviceDescriptor &deviceInfo)
{
    int32_t ret = 0;
    AUDIO_INFO_LOG("GetAdapterBufferInfo enter, deviceRole %{public}d.", deviceInfo.deviceRole_);
    if (deviceInfo.deviceRole_ == INPUT_DEVICE) {
        std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
        CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "fast source is null.");
        ret = source->GetMmapBufferInfo(dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_,
        dstByteSizePerFrame_, syncInfoSize_);
    } else {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "fast sink is null.");
        ret = sink->GetMmapBufferInfo(dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_,
        dstByteSizePerFrame_, syncInfoSize_);
    }

    if (ret != SUCCESS || dstBufferFd_ == -1 || dstTotalSizeInframe_ == 0 || dstSpanSizeInframe_ == 0 ||
        dstByteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("get mmap buffer info fail, ret %{public}d, dstBufferFd %{public}d, \
            dstTotalSizeInframe %{public}d, dstSpanSizeInframe %{public}d, dstByteSizePerFrame %{public}d.",
            ret, dstBufferFd_, dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_);
        return ERR_ILLEGAL_STATE;
    }
    AUDIO_INFO_LOG("mmap buffer info: dstTotalSizeInframe %{public}d, dstSpanSizeInframe %{public}d,"
        "dstByteSizePerFrame %{public}d.", dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_);
    return SUCCESS;
}

int32_t AudioEndpointInner::PrepareDeviceBuffer(const AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("enter, deviceRole %{public}d.", deviceInfo.deviceRole_);
    if (dstAudioBuffer_ != nullptr) {
        AUDIO_INFO_LOG("endpoint buffer is preapred, fd:%{public}d", dstBufferFd_);
        return SUCCESS;
    }

    int32_t ret = GetAdapterBufferInfo(deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "get adapter buffer Info fail, ret %{public}d.", ret);

    // spanDuration_ may be less than the correct time of dstSpanSizeInframe_.
    spanDuration_ = static_cast<int64_t>(dstSpanSizeInframe_) * AUDIO_NS_PER_SECOND /
        static_cast<int64_t>(dstStreamInfo_.samplingRate);
    int64_t temp = spanDuration_ / 5 * 3; // 3/5 spanDuration
    int64_t setTime = -1;
    int64_t maxSetTime = (static_cast<int64_t>(dstTotalSizeInframe_ - dstSpanSizeInframe_)) *
        AUDIO_NS_PER_SECOND / static_cast<int64_t>(dstStreamInfo_.samplingRate);
    GetSysPara("persist.multimedia.serveraheadreadtime", setTime);
    temp = setTime > 0 && setTime < maxSetTime ? setTime : temp;
    serverAheadReadTime_ = temp < ONE_MILLISECOND_DURATION ? ONE_MILLISECOND_DURATION : temp; // at least 1ms ahead.
    AUDIO_INFO_LOG("spanDuration %{public}" PRIu64" ns, serverAheadReadTime %{public}" PRIu64" ns.",
        spanDuration_, serverAheadReadTime_);

    CHECK_AND_RETURN_RET_LOG(spanDuration_ > 0 && spanDuration_ < MAX_SPAN_DURATION_NS,
        ERR_INVALID_PARAM, "mmap span info error, spanDuration %{public}" PRIu64".", spanDuration_);
    AudioBufferHolder holder = syncInfoSize_ != 0 ? AUDIO_SERVER_ONLY_WITH_SYNC : AUDIO_SERVER_ONLY;
    dstAudioBuffer_ = OHAudioBuffer::CreateFromRemote(dstTotalSizeInframe_, dstSpanSizeInframe_, dstByteSizePerFrame_,
        holder, dstBufferFd_, INVALID_BUFFER_FD);
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr && dstAudioBuffer_->GetBufferHolder() == holder,
        ERR_ILLEGAL_STATE, "create buffer from remote fail.");

    if (dstAudioBuffer_ == nullptr || dstAudioBuffer_->GetStreamStatus() == nullptr) {
        AUDIO_ERR_LOG("The stream status is null!");
        return ERR_INVALID_PARAM;
    }

    dstAudioBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_IDEL);

    // clear data buffer
    ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0, dstAudioBuffer_->GetDataSize());
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memset buffer fail, ret %{public}d, fd %{public}d.", ret, dstBufferFd_);
    }
    InitAudiobuffer(true);

    AUDIO_DEBUG_LOG("end, fd %{public}d.", dstBufferFd_);
    return SUCCESS;
}

void AudioEndpointInner::InitAudiobuffer(bool resetReadWritePos)
{
    CHECK_AND_RETURN_LOG((dstAudioBuffer_ != nullptr), "dst audio buffer is null.");
    if (resetReadWritePos) {
        dstAudioBuffer_->ResetCurReadWritePos(0, 0, false);
    }

    uint32_t spanCount = dstAudioBuffer_->GetSpanCount();
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = dstAudioBuffer_->GetSpanInfoByIndex(i);
        CHECK_AND_RETURN_LOG(spanInfo != nullptr, "InitAudiobuffer failed.");
        if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
            spanInfo->spanStatus = SPAN_WRITE_DONE;
        } else {
            spanInfo->spanStatus = SPAN_READ_DONE;
        }
        spanInfo->offsetInFrame = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->writeStartTime = 0;
        spanInfo->writeDoneTime = 0;

        spanInfo->volumeStart = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->volumeEnd = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->isMute = false;
    }
    return;
}

int32_t AudioEndpointInner::GetPreferBufferInfo(uint32_t &totalSizeInframe, uint32_t &spanSizeInframe)
{
    totalSizeInframe = dstTotalSizeInframe_;
    spanSizeInframe = dstSpanSizeInframe_;
    return SUCCESS;
}

bool AudioEndpointInner::IsAnyProcessRunning()
{
    std::lock_guard<std::mutex> lock(listLock_);
    return IsAnyProcessRunningInner();
}

// Should be called with AudioEndpointInner::listLock_ locked
bool AudioEndpointInner::IsAnyProcessRunningInner()
{
    bool isRunning = false;
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        if (processBufferList_[i]->GetStreamStatus() &&
            processBufferList_[i]->GetStreamStatus()->load() == STREAM_RUNNING) {
            isRunning = true;
            break;
        }
    }
    return isRunning;
}

void AudioEndpointInner::RecordReSyncPosition()
{
    AUDIO_INFO_LOG("RecordReSyncPosition enter.");
    uint64_t curHdiWritePos = 0;
    int64_t writeTime = 0;
    CHECK_AND_RETURN_LOG(GetDeviceHandleInfo(curHdiWritePos, writeTime), "get device handle info fail.");
    AUDIO_DEBUG_LOG("get capturer info, curHdiWritePos %{public}" PRIu64", writeTime %{public}" PRId64".",
        curHdiWritePos, writeTime);
    int64_t temp = ClockTime::GetCurNano() - writeTime;
    if (temp > spanDuration_) {
        AUDIO_WARNING_LOG("GetDeviceHandleInfo cost long time %{public}" PRIu64".", temp);
    }

    writeTimeModel_.ResetFrameStamp(curHdiWritePos, writeTime);
    uint64_t nextDstReadPos = curHdiWritePos;
    uint64_t nextDstWritePos = curHdiWritePos;
    InitAudiobuffer(false);
    int32_t ret = dstAudioBuffer_->ResetCurReadWritePos(nextDstReadPos, nextDstWritePos, false);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "ResetCurReadWritePos failed.");

    SpanInfo *nextReadSapn = dstAudioBuffer_->GetSpanInfo(nextDstReadPos);
    CHECK_AND_RETURN_LOG(nextReadSapn != nullptr, "GetSpanInfo failed.");
    nextReadSapn->offsetInFrame = nextDstReadPos;
    nextReadSapn->spanStatus = SpanStatus::SPAN_WRITE_DONE;
}

void AudioEndpointInner::ReSyncPosition()
{
    Trace loopTrace("AudioEndpoint::ReSyncPosition");
    uint64_t curHdiReadPos = 0;
    int64_t readTime = 0;
    bool res = GetDeviceHandleInfo(curHdiReadPos, readTime);
    CHECK_AND_RETURN_LOG(res, "ReSyncPosition call GetDeviceHandleInfo failed.");
    int64_t curTime = ClockTime::GetCurNano();
    int64_t temp = curTime - readTime;
    if (temp > spanDuration_) {
        AUDIO_ERR_LOG("GetDeviceHandleInfo may cost long time.");
    }

    readTimeModel_.ResetFrameStamp(curHdiReadPos, readTime);
    uint64_t nextDstWritePos = curHdiReadPos + dstSpanSizeInframe_;
    InitAudiobuffer(false);
    int32_t ret = dstAudioBuffer_->ResetCurReadWritePos(nextDstWritePos, nextDstWritePos, false);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "ResetCurReadWritePos failed.");

    SpanInfo *nextWriteSapn = dstAudioBuffer_->GetSpanInfo(nextDstWritePos);
    CHECK_AND_RETURN_LOG(nextWriteSapn != nullptr, "GetSpanInfo failed.");
    nextWriteSapn->offsetInFrame = nextDstWritePos;
    nextWriteSapn->spanStatus = SpanStatus::SPAN_READ_DONE;
    return;
}

bool AudioEndpointInner::StartDevice(EndpointStatus preferredState)
{
    AUDIO_INFO_LOG("StartDevice enter.");
    // how to modify the status while unlinked and started?

    AudioXCollie audioXCollie("AudioEndpointInner::StartDevice", START_DEVICE_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("[xcollie] StartDevice timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    // startDevice and unlinkprocess may called in concurrency
    std::unique_lock<std::mutex> listLock(listLock_);

    CHECK_AND_RETURN_RET_LOG(endpointStatus_ == IDEL, false, "Endpoint status is %{public}s",
        GetStatusStr(endpointStatus_).c_str());
    endpointStatus_ = STARTING;
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
    if ((deviceInfo_.deviceRole_ == INPUT_DEVICE && (source == nullptr || source->Start() != SUCCESS)) ||
        (deviceInfo_.deviceRole_ == OUTPUT_DEVICE && (sink == nullptr || sink->Start() != SUCCESS))) {
        HandleStartDeviceFailed();
        return false;
    }
    isStarted_ = true;
    ResetZeroVolumeState();

    Trace trace("AudioEndpointInner::StartDupStream");
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capture : fastCaptureInfos_) {
            if (capture.second.isInnerCapEnabled && capture.second.dupStream != nullptr) {
                capture.second.dupStream->Start();
            }
        }
    }

    std::unique_lock<std::mutex> lock(loopThreadLock_);
    needReSyncPosition_ = true;
    endpointStatus_ = IsAnyProcessRunningInner() ? RUNNING : IDEL;
    if (preferredState != INVALID) {
        AUDIO_INFO_LOG("Preferred state: %{public}d, current: %{public}d", preferredState, endpointStatus_.load());
        endpointStatus_ = preferredState;
    }
    workThreadCV_.notify_all();
    AUDIO_DEBUG_LOG("StartDevice out, status is %{public}s", GetStatusStr(endpointStatus_).c_str());

    listLock.unlock();
    return true;
}

void AudioEndpointInner::HandleStartDeviceFailed()
{
    AUDIO_ERR_LOG("Start failed for %{public}d, endpoint type %{public}u, process list size: %{public}zu.",
        deviceInfo_.deviceRole_, endpointType_, processList_.size());
    isStarted_ = false;
    if (processList_.size() <= 1) { // The endpoint only has the current stream
        endpointStatus_ = UNLINKED;
    } else {
        endpointStatus_ = IDEL;
    }
    workThreadCV_.notify_all();
}

// will not change state to stopped
bool AudioEndpointInner::DelayStopDevice()
{
    AUDIO_INFO_LOG("Status:%{public}s", GetStatusStr(endpointStatus_).c_str());

    // Clear data buffer to avoid noise in some case.
    if (dstAudioBuffer_ != nullptr) {
        int32_t ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0,
            dstAudioBuffer_->GetDataSize());
        if (ret != EOK) {
            AUDIO_WARNING_LOG("reset buffer fail, ret %{public}d.", ret);
        }
    }

    {
        Trace trace("AudioEndpointInner::StopDupStreamInDelay");
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capture : fastCaptureInfos_) {
            if (capture.second.isInnerCapEnabled && capture.second.dupStream != nullptr) {
                capture.second.dupStream->Stop();
            }
        }
    }

    if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
        std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
        CHECK_AND_RETURN_RET_LOG(source != nullptr && source->Stop() == SUCCESS,
            false, "Source stop failed.");
    } else {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        CHECK_AND_RETURN_RET_LOG(endpointStatus_ == IDEL && sink != nullptr && sink->Stop() == SUCCESS,
            false, "Sink stop failed.");
    }
    isStarted_ = false;
    return true;
}

bool AudioEndpointInner::StopDevice()
{
    DeinitLatencyMeasurement();

    AUDIO_INFO_LOG("StopDevice with status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    // todo
    endpointStatus_ = STOPPING;
    // Clear data buffer to avoid noise in some case.
    if (dstAudioBuffer_ != nullptr) {
        int32_t ret = memset_s(dstAudioBuffer_->GetDataBase(), dstAudioBuffer_->GetDataSize(), 0,
            dstAudioBuffer_->GetDataSize());
        AUDIO_INFO_LOG("StopDevice clear buffer ret:%{public}d", ret);
    }

    {
        Trace trace("AudioEndpointInner::StopDupStream");
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capture : fastCaptureInfos_) {
            if (capture.second.isInnerCapEnabled && capture.second.dupStream != nullptr) {
                capture.second.dupStream->Stop();
            }
        }
    }

    if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
        std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
        CHECK_AND_RETURN_RET_LOG(source != nullptr && source->Stop() == SUCCESS,
            false, "Source stop failed.");
    } else {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        CHECK_AND_RETURN_RET_LOG(sink != nullptr && sink->Stop() == SUCCESS, false, "Sink stop failed.");
    }
    endpointStatus_ = STOPPED;
    isStarted_ = false;
    return true;
}

int32_t AudioEndpointInner::OnStart(IAudioProcessStream *processStream)
{
    InitLatencyMeasurement();
    AUDIO_PRERELEASE_LOGI("OnStart endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == RUNNING) {
        AUDIO_INFO_LOG("OnStart find endpoint already in RUNNING.");
        return SUCCESS;
    }
    if (endpointStatus_ == IDEL) {
        // call sink start
        if (!isStarted_) {
            CHECK_AND_RETURN_RET_LOG(StartDevice(RUNNING), ERR_OPERATION_FAILED, "StartDevice failed");
        }
    }

    endpointStatus_ = RUNNING;
    delayStopTime_ = INT64_MAX;
    return SUCCESS;
}

int32_t AudioEndpointInner::OnPause(IAudioProcessStream *processStream)
{
    AUDIO_PRERELEASE_LOGI("OnPause endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == RUNNING) {
        endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
    }
    if (endpointStatus_ == IDEL) {
        // delay call sink stop when no process running
        AUDIO_PRERELEASE_LOGI("OnPause status is IDEL, need delay call stop");
        delayStopTime_ = ClockTime::GetCurNano() + ((clientConfig_.audioMode == AUDIO_MODE_PLAYBACK)
            ? PLAYBACK_DELAY_STOP_HDI_TIME_NS : RECORDER_DELAY_STOP_HDI_TIME_NS);
    }
    // todo
    return SUCCESS;
}

int32_t AudioEndpointInner::OnUpdateHandleInfo(IAudioProcessStream *processStream)
{
    Trace trace("AudioEndpoint::OnUpdateHandleInfo");
    bool isFind = false;
    std::lock_guard<std::mutex> lock(listLock_);
    auto processItr = processList_.begin();
    while (processItr != processList_.end()) {
        if (*processItr != processStream) {
            processItr++;
            continue;
        }
        std::shared_ptr<OHAudioBufferBase> processBuffer = (*processItr)->GetStreamBuffer();
        CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_OPERATION_FAILED, "Process found but buffer is null");
        uint64_t proHandleFrame = 0;
        int64_t proHandleTime = 0;
        processBuffer->GetHandleInfo(proHandleFrame, proHandleTime);

        isFind = true;
        break;
    }
    CHECK_AND_RETURN_RET_LOG(isFind, ERR_OPERATION_FAILED, "Can not find any process to UpdateHandleInfo");
    return SUCCESS;
}

void AudioEndpointInner::AddProcessStreamToList(IAudioProcessStream *processStream,
    const std::shared_ptr<OHAudioBufferBase> &processBuffer)
{
    std::lock_guard<std::mutex> lock(listLock_);
    processList_.push_back(processStream);
    processBufferList_.push_back(processBuffer);
    processTmpBufferList_.push_back({});
}

int32_t AudioEndpointInner::LinkProcessStream(IAudioProcessStream *processStream, bool startWhenLinking)
{
    CHECK_AND_RETURN_RET_LOG(processStream != nullptr, ERR_INVALID_PARAM, "IAudioProcessStream is null");
    std::shared_ptr<OHAudioBufferBase> processBuffer = processStream->GetStreamBuffer();
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_PARAM, "processBuffer is null");
    CHECK_AND_RETURN_RET_LOG(processBuffer->GetStreamStatus() != nullptr, ERR_INVALID_PARAM, "stream status is null");

    CHECK_AND_RETURN_RET_LOG(processList_.size() < MAX_LINKED_PROCESS, ERR_OPERATION_FAILED, "reach link limit.");

    AUDIO_INFO_LOG("LinkProcessStream start status is:%{public}s.", GetStatusStr(endpointStatus_).c_str());
    processBuffer->SetSessionId(processStream->GetAudioSessionId());
    bool needEndpointRunning = processBuffer->GetStreamStatus()->load() == STREAM_RUNNING;

    AddEndpointStreamVolume(processStream);

    if (endpointStatus_ == STARTING) {
        AUDIO_INFO_LOG("LinkProcessStream wait start begin.");
        std::unique_lock<std::mutex> lock(loopThreadLock_);
        workThreadCV_.wait_for(lock, std::chrono::milliseconds(SLEEP_TIME_IN_DEFAULT), [this] {
            return endpointStatus_ != STARTING;
        });
    }

    if (endpointStatus_ == RUNNING) {
        LinkProcessStreamExt(processStream, processBuffer);
        return SUCCESS;
    }

    if (endpointStatus_ == UNLINKED) {
        endpointStatus_ = IDEL; // handle push_back in IDEL
        if (isDeviceRunningInIdel_) {
            delayStopTime_ = INT64_MAX;
            CHECK_AND_RETURN_RET_LOG(StartDevice(), ERR_OPERATION_FAILED, "StartDevice failed");
            delayStopTime_ = ClockTime::GetCurNano() + ((clientConfig_.audioMode == AUDIO_MODE_PLAYBACK)
                ? PLAYBACK_DELAY_STOP_HDI_TIME_NS : LINK_RECORDER_DELAY_STOP_HDI_TIME_NS);
        }
    }

    if (endpointStatus_ == IDEL) {
        {
            AddProcessStreamToList(processStream, processBuffer);
        }
        if (!needEndpointRunning || !startWhenLinking) {
            AUDIO_INFO_LOG("LinkProcessStream success, process stream status is not running.");
            return SUCCESS;
        }
        // needEndpointRunning = true
        if (isDeviceRunningInIdel_) {
            endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
        } else {
            // needEndpointRunning = true & isDeviceRunningInIdel_ = false
            // KeepWorkloopRunning will wait on IDEL
            CHECK_AND_RETURN_RET_LOG(StartDevice(), ERR_OPERATION_FAILED, "StartDevice failed");
        }
        AUDIO_INFO_LOG("LinkProcessStream success with status:%{public}s", GetStatusStr(endpointStatus_).c_str());
        return SUCCESS;
    }

    AUDIO_INFO_LOG("LinkProcessStream success with status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    return SUCCESS;
}

void AudioEndpointInner::AddEndpointStreamVolume(IAudioProcessStream *processStream)
{
    Trace trace("AudioEndpointInner::AddEndpointStreamVolume");
    bool isSystemApp = CheckoutSystemAppUtil::CheckoutSystemApp(processStream->GetAppInfo().appUid);
    StreamVolumeParams streamVolumeParams = { processStream->GetAudioSessionId(),
        processStream->GetAudioProcessConfig().streamType,
        processStream->GetAudioProcessConfig().rendererInfo.streamUsage,
        processStream->GetAppInfo().appUid, processStream->GetAppInfo().appPid, isSystemApp,
        processStream->GetAudioProcessConfig().rendererInfo.volumeMode,
        processStream->GetAudioProcessConfig().rendererInfo.isVirtualKeyboard };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
    AUDIO_INFO_LOG("when stream start, add streamVolume for this stream");
}

void AudioEndpointInner::LinkProcessStreamExt(IAudioProcessStream *processStream,
    const std::shared_ptr<OHAudioBufferBase>& processBuffer)
{
    AddProcessStreamToList(processStream, processBuffer);
    AUDIO_INFO_LOG("LinkProcessStream success in RUNNING.");
}

int32_t AudioEndpointInner::UnlinkProcessStream(IAudioProcessStream *processStream)
{
    AUDIO_INFO_LOG("UnlinkProcessStream in status:%{public}s.", GetStatusStr(endpointStatus_).c_str());
    CHECK_AND_RETURN_RET_LOG(processStream != nullptr, ERR_INVALID_PARAM, "IAudioProcessStream is null");
    std::shared_ptr<OHAudioBufferBase> processBuffer = processStream->GetStreamBuffer();
    CHECK_AND_RETURN_RET_LOG(processBuffer != nullptr, ERR_INVALID_PARAM, "processBuffer is null");
    AudioVolume::GetInstance()->RemoveStreamVolume(processStream->GetAudioSessionId());

    bool isFind = false;
    std::lock_guard<std::mutex> lock(listLock_);
    auto processItr = processList_.begin();
    auto bufferItr = processBufferList_.begin();
    auto tmpBufferItr = processTmpBufferList_.begin();
    while (processItr != processList_.end()) {
        if (*processItr == processStream && *bufferItr == processBuffer) {
            processList_.erase(processItr);
            processBufferList_.erase(bufferItr);
            processTmpBufferList_.erase(tmpBufferItr);
            isFind = true;
            break;
        } else {
            processItr++;
            bufferItr++;
            tmpBufferItr++;
        }
    }
    if (processList_.size() == 0) {
        StopDevice();
        endpointStatus_ = UNLINKED;
    } else if (!IsAnyProcessRunningInner()) {
        endpointStatus_ = IDEL;
        delayStopTime_ = DELAY_STOP_HDI_TIME_WHEN_NO_RUNNING_NS;
    }

    AUDIO_INFO_LOG("UnlinkProcessStream end, %{public}s the process.", (isFind ? "find and remove" : "not find"));
    return SUCCESS;
}

bool AudioEndpointInner::IsBufferDataInsufficient(int32_t readableDataFrame, uint32_t spanSizeInFrame)
{
    if (readableDataFrame < 0) {
        return false;
    }

    if (static_cast<uint32_t>(readableDataFrame) >= spanSizeInFrame) {
        return false;
    }

    return true;
}

bool AudioEndpointInner::CheckAllBufferReady(int64_t checkTime, uint64_t curWritePos)
{
    bool isAllReady = true;
    bool needCheckStandby = false;
    {
        // lock list without sleep
        std::lock_guard<std::mutex> lock(listLock_);
        for (size_t i = 0; i < processBufferList_.size(); i++) {
            std::shared_ptr<OHAudioBufferBase> tempBuffer = processBufferList_[i];
            uint64_t eachCurReadPos = processBufferList_[i]->GetCurReadFrame();
            lastHandleProcessTime_ = checkTime;
            processBufferList_[i]->SetHandleInfo(eachCurReadPos, lastHandleProcessTime_); // update handle info
            if (tempBuffer->GetStreamStatus() &&
                tempBuffer->GetStreamStatus()->load() != StreamStatus::STREAM_RUNNING) {
                // Process is not running, server will continue to check the same location in the next cycle.
                int64_t duration = 5000000; // 5ms
                processBufferList_[i]->SetHandleInfo(eachCurReadPos, lastHandleProcessTime_ + duration);
                continue; // process not running
            }
            // Status is RUNNING
            int64_t current = ClockTime::GetCurNano();
            int64_t lastWrittenTime = tempBuffer->GetLastWrittenTime();
            uint32_t sessionId = processList_[i]->GetAudioSessionId();
            if (current - lastWrittenTime > WAIT_CLIENT_STANDBY_TIME_NS) {
                Trace trace("AudioEndpoint::MarkClientStandby:" + std::to_string(sessionId));
                AUDIO_INFO_LOG("change the status to stand-by, session %{public}u", sessionId);
                processList_[i]->EnableStandby();
                needCheckStandby = true;
                continue;
            }
            int32_t readableDataFrame = tempBuffer->GetReadableDataFrames();
            uint32_t spanSizeInFrame = processList_[i]->GetSpanSizeInFrame();
            if (IsBufferDataInsufficient(readableDataFrame, spanSizeInFrame)) {
                isAllReady = false;
                AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_LOWLATENCY_OUT,
                    processList_[i]->GetAppInfo().appUid);
                continue;
            } else {
                AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_LOWLATENCY_OUT,
                    processList_[i]->GetAppInfo().appUid);
            }
            // process Status is RUNNING && buffer status is WRITE_DONE
            tempBuffer->SetLastWrittenTime(current);
        }
    }

    if (needCheckStandby) {
        CheckStandBy();
    }

    if (!isAllReady) {
        WaitAllProcessReady(curWritePos);
    }
    return isAllReady;
}

void AudioEndpointInner::WaitAllProcessReady(uint64_t curWritePos)
{
    Trace trace("AudioEndpoint::WaitAllProcessReady");
    int64_t tempWakeupTime = readTimeModel_.GetTimeOfPos(curWritePos) + WRITE_TO_HDI_AHEAD_TIME;
    if (tempWakeupTime - ClockTime::GetCurNano() < ONE_MILLISECOND_DURATION) {
        ClockTime::RelativeSleep(ONE_MILLISECOND_DURATION);
    } else {
        ClockTime::AbsoluteSleep(tempWakeupTime); // sleep to hdi read time ahead 1ms.
    }
}

void AudioEndpointInner::MixToDupStream(const std::vector<AudioStreamData> &srcDataList, int32_t innerCapId)
{
    Trace trace("AudioEndpointInner::MixToDupStream");
    CHECK_AND_RETURN_LOG(fastCaptureInfos_.count(innerCapId) && fastCaptureInfos_[innerCapId].dupStream != nullptr,
        "captureInfo is errro");
    CHECK_AND_RETURN_LOG(dupBuffer_ != nullptr, "Buffer is not ready");

    std::vector<AudioStreamData> tempList;
    for (size_t i = 0; i < srcDataList.size(); i++) {
        if (!srcDataList[i].isInnerCapeds.count(innerCapId) ||
            !srcDataList[i].isInnerCapeds.at(innerCapId)) {
            continue;
        }
        AudioStreamData cur = srcDataList[i];
        cur.volumeStart = cur.volumeHap;
        tempList.push_back(cur);
    }
    BufferDesc temp;
    temp.buffer = dupBuffer_.get();
    temp.bufLength = dupBufferSize_;
    temp.dataLength = dupBufferSize_;
    AudioStreamData dstStream;
    dstStream.streamInfo = dstStreamInfo_;
    dstStream.bufferDesc = temp;
    FormatConverter::DataAccumulationFromVolume(tempList, dstStream);

    int32_t ret;
    if (GetEngineFlag() == 1) {
        WriteDupBufferInner(temp, innerCapId);
    } else {
        ret = fastCaptureInfos_[innerCapId].dupStream->EnqueueBuffer(temp);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "EnqueueBuffer failed:%{public}d", ret);
    }

    ret = memset_s(reinterpret_cast<void *>(dupBuffer_.get()), dupBufferSize_, 0, dupBufferSize_);
    if (ret != EOK) {
        AUDIO_WARNING_LOG("memset buffer fail, ret %{public}d", ret);
    }
}

void AudioEndpointInner::ProcessData(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData)
{
    bool ret = FormatConverter::DataAccumulationFromVolume(srcDataList, dstData);
    CHECK_AND_RETURN_LOG(ret, "Format may not match");

    ChannelVolumes channelVolumes = VolumeTools::CountVolumeLevel(
        dstData.bufferDesc, dstData.streamInfo.format, dstData.streamInfo.channels);
    if (!isExistLoopback_) {
        ZeroVolumeCheck(std::accumulate(channelVolumes.volStart, channelVolumes.volStart +
            channelVolumes.channel, static_cast<int64_t>(0)) / channelVolumes.channel);
    }
}

void AudioEndpointInner::HandleRendererDataParams(const AudioStreamData &srcData, const AudioStreamData &dstData,
    bool applyVol)
{
    if (srcData.streamInfo.encoding != dstData.streamInfo.encoding) {
        AUDIO_ERR_LOG("Different encoding formats");
        return;
    }
    if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == STEREO) {
        return ProcessSingleData(srcData, dstData, applyVol);
    }

    if (srcData.streamInfo.format == SAMPLE_S16LE || srcData.streamInfo.format == SAMPLE_F32LE) {
        CHECK_AND_RETURN_LOG(processList_.size() > 0 && processList_[0] != nullptr, "No avaliable process");
        BufferDesc &convertedBuffer = processList_[0]->GetConvertedBuffer();
        int32_t ret = -1;
        if (srcData.streamInfo.format == SAMPLE_S16LE && srcData.streamInfo.channels == MONO) {
            ret = FormatConverter::S16MonoToS16Stereo(srcData.bufferDesc, convertedBuffer);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Convert channel from s16 mono to s16 stereo failed");
        } else if (srcData.streamInfo.format == SAMPLE_F32LE && srcData.streamInfo.channels == MONO) {
            ret = FormatConverter::F32MonoToS16Stereo(srcData.bufferDesc, convertedBuffer);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Convert channel from f32 mono to s16 stereo failed");
        } else if (srcData.streamInfo.format == SAMPLE_F32LE && srcData.streamInfo.channels == STEREO) {
            ret = FormatConverter::F32StereoToS16Stereo(srcData.bufferDesc, convertedBuffer);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Convert channel from f32 stereo to s16 stereo failed");
        } else {
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unsupport conversion");
        }
        AudioStreamData dataAfterProcess = srcData;
        dataAfterProcess.bufferDesc = convertedBuffer;
        ProcessSingleData(dataAfterProcess, dstData, applyVol);
        ret = memset_s(static_cast<void *>(convertedBuffer.buffer), convertedBuffer.bufLength, 0,
            convertedBuffer.bufLength);
        CHECK_AND_RETURN_LOG(ret == EOK, "memset converted buffer to 0 failed");
    }
}

void AudioEndpointInner::ProcessSingleData(const AudioStreamData &srcData, const AudioStreamData &dstData,
    bool applyVol)
{
    CHECK_AND_RETURN_LOG(dstData.streamInfo.format == SAMPLE_S16LE && dstData.streamInfo.channels == STEREO,
        "ProcessData failed, streamInfo are not support");

    size_t dataLength = dstData.bufferDesc.dataLength;
    dataLength /= 2; // SAMPLE_S16LE--> 2 byte
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstData.bufferDesc.buffer);
    for (size_t offset = 0; dataLength > 0; dataLength--) {
        int32_t vol = 1 << VOLUME_SHIFT_NUMBER;
        int16_t *srcPtr = reinterpret_cast<int16_t *>(srcData.bufferDesc.buffer) + offset;
        int32_t sum = applyVol ? (*srcPtr * static_cast<int64_t>(vol)) >> VOLUME_SHIFT_NUMBER : *srcPtr; // 1/65536
        offset++;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
}

// call with listLock_ hold
void AudioEndpointInner::GetAllReadyProcessData(std::vector<AudioStreamData> &audioDataList,
    std::function<void()> &moveClientsIndex)
{
    isExistLoopback_ = false;
    audioHapticsSyncId_ = 0;
    std::vector<std::function<void()>> moveClientIndexVector;
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        CHECK_AND_CONTINUE_LOG(processBufferList_[i] != nullptr, "this processBuffer is nullptr!");
        uint64_t curRead = processBufferList_[i]->GetCurReadFrame();
        Trace trace("AudioEndpoint::ReadProcessData->" + std::to_string(curRead));
        CHECK_AND_CONTINUE_LOG(processList_[i] != nullptr, "this process is nullptr!");
        auto processConfig = processList_[i]->GetAudioProcessConfig();
        if (processConfig.rendererInfo.isLoopback) {
            isExistLoopback_ = true;
        }
        // If there is a sync ID in the process and it is the current first frame.
        // then the sync ID needs to be recorded.
        if (processList_[i]->GetAudioHapticsSyncId() > 0 && curRead == 0) {
            audioHapticsSyncId_ = processList_[i]->GetAudioHapticsSyncId();
        }
        std::function<void()> moveClientIndexFunc;
        GetAllReadyProcessDataSub(i, audioDataList, curRead, moveClientIndexFunc);
        moveClientIndexVector.push_back(moveClientIndexFunc);
    }

    moveClientsIndex =
        [moveClientIndexVec = std::move(moveClientIndexVector)] () {
            for (const auto& moveFunc : moveClientIndexVec) {
                if (moveFunc) {
                    moveFunc();
                }
            }
        };
}

AudioEndpointInner::VolumeResult AudioEndpointInner::CalculateVolume(size_t i)
{
    Volume vol = {true, 1.0f, 0};

    AudioStreamType streamType = processList_[i]->GetAudioStreamType();
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    DeviceType deviceType = PolicyHandler::GetInstance().GetActiveOutPutDevice();
    bool getVolumeRet = PolicyHandler::GetInstance().GetSharedVolume(volumeType, deviceType, vol);
    int32_t doNotDisturbStatusVolume = static_cast<int32_t>(AudioVolume::GetInstance()->GetDoNotDisturbStatusVolume(
        streamType, clientConfig_.appInfo.appUid, processList_[i]->GetAudioSessionId()));
    float appVolume = AudioVolume::GetInstance()->GetAppVolume(clientConfig_.appInfo.appUid,
        clientConfig_.rendererInfo.volumeMode);
    int32_t volumeFromOhaudioBuffer = processBufferList_[i]->GetStreamVolume() *
        processBufferList_[i]->GetDuckFactor() * processBufferList_[i]->GetMuteFactor() * (1 << VOLUME_SHIFT_NUMBER);
    float baseVolume = volumeFromOhaudioBuffer * appVolume * doNotDisturbStatusVolume;

    VolumeResult result;
    if (deviceInfo_.networkId_ != LOCAL_NETWORK_ID || (deviceInfo_.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP
        && volumeType == STREAM_MUSIC && PolicyHandler::GetInstance().IsAbsVolumeSupported()) || !getVolumeRet ||
        IsNearlinkAbsVolSupportStream(deviceInfo_.deviceType_, volumeType)) {
        result.volumeStart = vol.isMute ? 0 : static_cast<int32_t>(baseVolume);
    } else if (clientConfig_.rendererInfo.isVirtualKeyboard) {
        result.volumeStart = vol.isMute ? 0 : static_cast<int32_t>(baseVolume);
    } else {
        result.volumeStart = vol.isMute ? 0 : static_cast<int32_t>(baseVolume * vol.volumeFloat);
    }

    result.muteFlag = processList_[i]->GetMuteState();
    result.volumeEnd = volumeFromOhaudioBuffer;
    result.volumeHap = result.muteFlag ? 0 : volumeFromOhaudioBuffer;
    AudioStreamMonitor::GetInstance().UpdateMonitorVolume(processList_[i]->GetAudioSessionId(), result.volumeStart);

    return result;
}

bool AudioEndpointInner::PrepareRingBuffer(size_t i, uint64_t curRead, RingBufferWrapper& ringBuffer)
{
    int32_t ret = processBufferList_[i]->GetAllReadableBufferFromPosFrame(curRead, ringBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && ringBuffer.dataLength > 0, false,
        "getBuffer failed ret: %{public}d lenth: %{public}zu",
        ret, ringBuffer.dataLength);

    auto byteSizePerFrame = processList_[i]->GetByteSizePerFrame();
    CHECK_AND_RETURN_RET_LOG(byteSizePerFrame != 0, false, "byteSizePerFrame is 0");

    size_t spanSizeInByte = processList_[i]->GetSpanSizeInFrame() * byteSizePerFrame;
    if (ringBuffer.dataLength > spanSizeInByte) {
        ringBuffer.dataLength = spanSizeInByte;
    }
    
    return true;
}

void AudioEndpointInner::SetupMoveCallback(size_t i, uint64_t curRead, const RingBufferWrapper& ringBuffer,
    std::function<void()>& moveClientIndex
)
{
    auto byteSizePerFrame = processList_[i]->GetByteSizePerFrame();
    CHECK_AND_RETURN_LOG(byteSizePerFrame != 0, "byteSizePerFrame is 0");
    uint64_t readFramePosAfterRead = curRead + (ringBuffer.dataLength / byteSizePerFrame);
    auto ohAudioBuffer = processBufferList_[i];
    moveClientIndex = [readFramePosAfterRead, ringBuffer, ohAudioBuffer] () mutable {
        ohAudioBuffer->SetCurReadFrame(readFramePosAfterRead);
        ringBuffer.SetBuffersValueWithSpecifyDataLen(0);
    };
}

bool AudioEndpointInner::IsNearlinkAbsVolSupportStream(DeviceType deviceType, AudioVolumeType volumeType)
{
    bool isNearlink = deviceType == DEVICE_TYPE_NEARLINK;
    bool isMusicStream = volumeType == STREAM_MUSIC;
    bool isVoiceCallStream = volumeType == STREAM_VOICE_CALL;
    return isNearlink && (isMusicStream || isVoiceCallStream);
}

bool AudioEndpointInner::NeedUseTempBuffer(const RingBufferWrapper &ringBuffer, size_t spanSizeInByte)
{
    if (ringBuffer.dataLength > ringBuffer.basicBufferDescs[0].bufLength) {
        return true;
    }

    if (ringBuffer.dataLength < spanSizeInByte) {
        return true;
    }

    return false;
}

void AudioEndpointInner::PrepareStreamDataBuffer(size_t i, size_t spanSizeInByte,
    RingBufferWrapper &ringBuffer, AudioStreamData &streamData)
{
    if (NeedUseTempBuffer(ringBuffer, spanSizeInByte)) {
        processTmpBufferList_[i].resize(0);
        processTmpBufferList_[i].resize(spanSizeInByte);
        RingBufferWrapper ringBufferDescForCotinueData;
        ringBufferDescForCotinueData.dataLength = ringBuffer.dataLength;
        ringBufferDescForCotinueData.basicBufferDescs[0].buffer = processTmpBufferList_[i].data();
        ringBufferDescForCotinueData.basicBufferDescs[0].bufLength = ringBuffer.dataLength;
        ringBufferDescForCotinueData.CopyInputBufferValueToCurBuffer(ringBuffer);
        streamData.bufferDesc.buffer = processTmpBufferList_[i].data();
        streamData.bufferDesc.bufLength = spanSizeInByte;
        streamData.bufferDesc.dataLength = spanSizeInByte;
    } else {
        streamData.bufferDesc.buffer = ringBuffer.basicBufferDescs[0].buffer;
        streamData.bufferDesc.bufLength = ringBuffer.dataLength;
        streamData.bufferDesc.dataLength = ringBuffer.dataLength;
    }
}

void AudioEndpointInner::GetAllReadyProcessDataSub(size_t i,
    std::vector<AudioStreamData> &audioDataList, uint64_t curRead, std::function<void()> &moveClientIndex)
{
    VolumeResult volResult = CalculateVolume(i);
    AudioStreamData streamData;
    streamData.volumeStart = volResult.volumeStart;
    streamData.volumeEnd = volResult.volumeEnd;
    streamData.volumeHap = volResult.volumeHap;

    streamData.streamInfo = processList_[i]->GetStreamInfo();
    streamData.isInnerCapeds = processList_[i]->GetInnerCapState();

    Trace traceVol("VolumeProcess " + std::to_string(volResult.volumeStart) +
        " sessionid:" + std::to_string(processList_[i]->GetAudioSessionId()) +
        (volResult.muteFlag ? " muted" : " unmuted"));

    RingBufferWrapper ringBuffer;
    if (!PrepareRingBuffer(i, curRead, ringBuffer)) {
        auto tempProcess = processList_[i];
        CHECK_AND_RETURN_LOG(tempProcess, "tempProcess is nullptr!");
        if (tempProcess->GetStreamStatus() == STREAM_RUNNING) {
            tempProcess->AddNoDataFrameSize();
        }
        return;
    }

    SetupMoveCallback(i, curRead, ringBuffer, moveClientIndex);

    if (volResult.muteFlag) {
        ringBuffer.SetBuffersValueWithSpecifyDataLen(0);
    }
    size_t spanSizeInByte = processList_[i]->GetSpanSizeInFrame() * processList_[i]->GetByteSizePerFrame();
    PrepareStreamDataBuffer(i, spanSizeInByte, ringBuffer, streamData);
    CheckPlaySignal(streamData.bufferDesc.buffer, streamData.bufferDesc.bufLength);
    audioDataList.push_back(streamData);
    processList_[i]->WriteDumpFile(static_cast<void *>(streamData.bufferDesc.buffer),
        streamData.bufferDesc.bufLength);
    WriteMuteDataSysEvent(streamData.bufferDesc.buffer, streamData.bufferDesc.bufLength, i);
    HandleMuteWriteData(streamData.bufferDesc, i);
}

void AudioEndpointInner::HandleMuteWriteData(BufferDesc &bufferDesc, int32_t index)
{
    CHECK_AND_RETURN_LOG(static_cast<size_t>(index + 1) <= processList_.size(), "invalid index");
    auto tempProcess = processList_[index];
    CHECK_AND_RETURN_LOG(tempProcess, "tempProcess is nullptr");
    tempProcess->AddNormalFrameSize();
    int64_t muteFrameCnt = 0;
    VolumeTools::CalcMuteFrame(bufferDesc, dstStreamInfo_, logUtilsTag_, volumeDataCount_, muteFrameCnt);
    tempProcess->AddMuteWriteFrameCnt(muteFrameCnt);
    tempProcess->AddMuteFrameSize(volumeDataCount_);
}

bool AudioEndpointInner::ProcessToEndpointDataHandle(uint64_t curWritePos, std::function<void()> &moveClientIndex)
{
    std::lock_guard<std::mutex> lock(listLock_);

    std::vector<AudioStreamData> audioDataList;
    GetAllReadyProcessData(audioDataList, moveClientIndex);
    CheckAudioHapticsSync(curWritePos);

    AudioStreamData dstStreamData;
    dstStreamData.streamInfo = dstStreamInfo_;
    int32_t ret = dstAudioBuffer_->GetWriteBuffer(curWritePos, dstStreamData.bufferDesc);
    CHECK_AND_RETURN_RET_LOG(((ret == SUCCESS && dstStreamData.bufferDesc.buffer != nullptr)), false,
        "GetWriteBuffer failed, ret:%{public}d", ret);

    Trace trace("AudioEndpoint::WriteDstBuffer=>" + std::to_string(curWritePos));
    // do write work
    if (audioDataList.size() == 0) {
        memset_s(dstStreamData.bufferDesc.buffer, dstStreamData.bufferDesc.bufLength, 0,
            dstStreamData.bufferDesc.bufLength);
    } else {
        if (endpointType_ == TYPE_VOIP_MMAP && audioDataList.size() == 1) {
            HandleRendererDataParams(audioDataList[0], dstStreamData);
        } else {
            ProcessData(audioDataList, dstStreamData);
        }
    }
    if (syncInfoSize_ != 0) {
        CheckSyncInfo(curWritePos);
        lastWriteTime_ = ClockTime::GetCurNano();
    }
    AdapterType type = endpointType_ == TYPE_VOIP_MMAP ? ADAPTER_TYPE_VOIP_FAST : ADAPTER_TYPE_FAST;
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(type, ClockTime::GetCurNano());
    {
        std::lock_guard<std::mutex> captureLock(dupMutex_);
        for (auto &capture: fastCaptureInfos_) {
            if (capture.second.isInnerCapEnabled) {
                ProcessToDupStream(audioDataList, dstStreamData, capture.first);
            }
        }
    }
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpHdi_, static_cast<void *>(dstStreamData.bufferDesc.buffer),
            dstStreamData.bufferDesc.bufLength);
        AudioCacheMgr::GetInstance().CacheData(dumpHdiName_,
            static_cast<void *>(dstStreamData.bufferDesc.buffer), dstStreamData.bufferDesc.bufLength);
    }

    CheckUpdateState(reinterpret_cast<char *>(dstStreamData.bufferDesc.buffer),
        dstStreamData.bufferDesc.bufLength);

    return true;
}

void AudioEndpointInner::CheckSyncInfo(uint64_t curWritePos)
{
    if (dstSpanSizeInframe_ == 0) {
        return;
    }
    uint32_t curWriteFrame = curWritePos / dstSpanSizeInframe_;
    dstAudioBuffer_->SetSyncWriteFrame(curWriteFrame);
    uint32_t curReadFrame = dstAudioBuffer_->GetSyncReadFrame();
    Trace trace("Sync: writeIndex:" + std::to_string(curWriteFrame) + " readIndex:" + std::to_string(curReadFrame));

    if (curWriteFrame >= curReadFrame) {
        // seems running ok.
        return;
    }
    AUDIO_WARNING_LOG("write %{public}d is slower than read %{public}d ", curWriteFrame, curReadFrame);
    AdapterType type = endpointType_ == TYPE_VOIP_MMAP ? ADAPTER_TYPE_VOIP_FAST : ADAPTER_TYPE_FAST;
    int64_t cost = (ClockTime::GetCurNano() - lastWriteTime_) / AUDIO_US_PER_SECOND;
    AudioPerformanceMonitor::GetInstance().ReportWriteSlow(type, cost);
    return;
}

void AudioEndpointInner::ProcessToDupStream(const std::vector<AudioStreamData> &audioDataList,
    AudioStreamData &dstStreamData, int32_t innerCapId)
{
    if (!fastCaptureInfos_.count(innerCapId) || fastCaptureInfos_[innerCapId].dupStream == nullptr) {
        AUDIO_ERR_LOG("innerCapId error or dupStream error");
        return;
    }
    Trace trace("AudioEndpointInner::ProcessToDupStream");
    if (endpointType_ == TYPE_VOIP_MMAP) {
        if (audioDataList.size() == 1 && audioDataList[0].isInnerCapeds.count(innerCapId)
            && audioDataList[0].isInnerCapeds.at(innerCapId)) {
            BufferDesc temp;
            temp.buffer = dupBuffer_.get();
            temp.bufLength = dupBufferSize_;
            temp.dataLength = dupBufferSize_;

            dstStreamData.bufferDesc = temp;
            HandleRendererDataParams(audioDataList[0], dstStreamData, false);
            if (GetEngineFlag() == 1) {
                WriteDupBufferInner(temp, innerCapId);
            } else {
                fastCaptureInfos_[innerCapId].dupStream->EnqueueBuffer(temp);
            }
        }
    } else {
        MixToDupStream(audioDataList, innerCapId);
    }
}

void AudioEndpointInner::CheckUpdateState(char *frame, uint64_t replyBytes)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(dstStreamInfo_.format),
            frame, replyBytes);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

float AudioEndpointInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

AudioMode AudioEndpointInner::GetAudioMode() const
{
    return clientConfig_.audioMode;
}

int64_t AudioEndpointInner::GetPredictNextReadTime(uint64_t posInFrame)
{
    Trace trace("AudioEndpoint::GetPredictNextRead");
    uint64_t handleSpanCnt = posInFrame / dstSpanSizeInframe_;
    uint32_t startPeriodCnt = 20; // sync each time when start
    uint32_t oneBigPeriodCnt = 40; // 200ms
    if (handleSpanCnt < startPeriodCnt || handleSpanCnt % oneBigPeriodCnt == 0) {
        updateThreadCV_.notify_all();
    }
    uint64_t readFrame = 0;
    int64_t readtime = 0;
    if (readTimeModel_.GetFrameStamp(readFrame, readtime)) {
        if (readFrame != posInFrame_) {
            CheckPosTimeRes res = readTimeModel_.UpdataFrameStamp(posInFrame_, timeInNano_);
            if (res == CHECK_FAILED) {
                updateThreadCV_.notify_all();
            } else if (res == NEED_MODIFY) {
                needReSyncPosition_ = true;
            }
        }
    }

    int64_t nextHdiReadTime = readTimeModel_.GetTimeOfPos(posInFrame);
    return nextHdiReadTime;
}

int64_t AudioEndpointInner::GetPredictNextWriteTime(uint64_t posInFrame)
{
    uint64_t handleSpanCnt = posInFrame / dstSpanSizeInframe_;
    uint32_t startPeriodCnt = 20;
    uint32_t oneBigPeriodCnt = 40;
    if (handleSpanCnt < startPeriodCnt || handleSpanCnt % oneBigPeriodCnt == 0) {
        updateThreadCV_.notify_all();
    }
    uint64_t writeFrame = 0;
    int64_t writetime = 0;
    if (writeTimeModel_.GetFrameStamp(writeFrame, writetime)) {
        if (writeFrame != posInFrame_) {
            CheckPosTimeRes res = writeTimeModel_.UpdataFrameStamp(posInFrame_, timeInNano_);
            if (res == CHECK_FAILED) {
                updateThreadCV_.notify_all();
            } else if (res == NEED_MODIFY) {
                needReSyncPosition_ = true;
            }
        }
    }
    int64_t nextHdiWriteTime = writeTimeModel_.GetTimeOfPos(posInFrame);
    return nextHdiWriteTime;
}

bool AudioEndpointInner::RecordPrepareNextLoop(uint64_t curReadPos, int64_t &wakeUpTime)
{
    uint64_t nextHandlePos = curReadPos + dstSpanSizeInframe_;
    int64_t nextHdiWriteTime = GetPredictNextWriteTime(nextHandlePos);
    int64_t tempDelay = endpointType_ == TYPE_VOIP_MMAP ? RECORD_VOIP_DELAY_TIME_NS : RECORD_DELAY_TIME_NS;
    int64_t predictWakeupTime = nextHdiWriteTime + tempDelay;
    if (predictWakeupTime <= ClockTime::GetCurNano()) {
        wakeUpTime = ClockTime::GetCurNano() + ONE_MILLISECOND_DURATION;
        AUDIO_ERR_LOG("hdi send wrong position time");
    } else {
        wakeUpTime = predictWakeupTime;
    }

    int32_t ret = dstAudioBuffer_->SetCurWriteFrame(nextHandlePos, false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "set dst buffer write frame fail, ret %{public}d.", ret);
    ret = dstAudioBuffer_->SetCurReadFrame(nextHandlePos, false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "set dst buffer read frame fail, ret %{public}d.", ret);

    return true;
}

bool AudioEndpointInner::PrepareNextLoop(uint64_t curWritePos, int64_t &wakeUpTime,
    const std::function<void()> &moveClientIndex)
{
    uint64_t nextHandlePos = curWritePos + dstSpanSizeInframe_;
    Trace prepareTrace("AudioEndpoint::PrepareNextLoop " + std::to_string(nextHandlePos));
    int64_t nextHdiReadTime = GetPredictNextReadTime(nextHandlePos);
    int64_t predictWakeupTime = nextHdiReadTime - serverAheadReadTime_;
    if (predictWakeupTime <= ClockTime::GetCurNano()) {
        wakeUpTime = ClockTime::GetCurNano() + ONE_MILLISECOND_DURATION;
        AUDIO_ERR_LOG("hdi send wrong position time");
    } else {
        wakeUpTime = predictWakeupTime;
    }

    SpanInfo *nextWriteSpan = dstAudioBuffer_->GetSpanInfo(nextHandlePos);
    CHECK_AND_RETURN_RET_LOG(nextWriteSpan != nullptr, false, "GetSpanInfo failed, can not get next write span");

    int32_t ret1 = dstAudioBuffer_->SetCurWriteFrame(nextHandlePos, false);
    int32_t ret2 = dstAudioBuffer_->SetCurReadFrame(nextHandlePos, false);
    CHECK_AND_RETURN_RET_LOG(ret1 == SUCCESS && ret2 == SUCCESS, false,
        "SetCurWriteFrame or SetCurReadFrame failed, ret1:%{public}d ret2:%{public}d", ret1, ret2);
    std::lock_guard<std::mutex> lock(listLock_);
    if (moveClientIndex) {
        moveClientIndex();
    }
    return true;
}

bool AudioEndpointInner::GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime)
{
    Trace trace("AudioEndpoint::GetMmapHandlePosition");
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = 0;
    if (deviceInfo_.deviceRole_ == INPUT_DEVICE) {
        std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
        CHECK_AND_RETURN_RET_LOG(source != nullptr && source->IsInited(),
            false, "Source start failed.");
        // GetMmapHandlePosition will call using ipc.
        ret = source->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    } else {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        CHECK_AND_RETURN_RET_LOG(sink != nullptr && sink->IsInited(),
            false, "GetDeviceHandleInfo failed: sink is not inited.");
        // GetMmapHandlePosition will call using ipc.
        ret = sink->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Call adapter GetMmapHandlePosition failed: %{public}d", ret);
    trace.End();
    nanoTime = timeNanoSec + timeSec * AUDIO_NS_PER_SECOND;
    Trace infoTrace("AudioEndpoint::GetDeviceHandleInfo frames=>" + std::to_string(frames) + " " +
        std::to_string(nanoTime) + " at " + std::to_string(ClockTime::GetCurNano()));
    nanoTime += DELTA_TO_REAL_READ_START_TIME; // global delay in server
    return true;
}

void AudioEndpointInner::AsyncGetPosTime()
{
    AUDIO_INFO_LOG("AsyncGetPosTime thread start.");
    while (!stopUpdateThread_) {
        std::unique_lock<std::mutex> lock(updateThreadLock_);
        updateThreadCV_.wait_for(lock, std::chrono::milliseconds(UPDATE_THREAD_TIMEOUT));
        if (stopUpdateThread_) {
            break;
        }
        if (endpointStatus_ == IDEL && isStarted_ && ClockTime::GetCurNano() > delayStopTime_) {
            AUDIO_INFO_LOG("IDEL for too long, let's call hdi stop");
            DelayStopDevice();
            continue;
        }
        if (!isStarted_) {
            continue;
        }
        // get signaled, call get pos-time
        uint64_t curHdiHandlePos = posInFrame_;
        int64_t handleTime = timeInNano_;
        if (!GetDeviceHandleInfo(curHdiHandlePos, handleTime)) {
            AUDIO_WARNING_LOG("AsyncGetPosTime call GetDeviceHandleInfo failed.");
            continue;
        }
        // keep it
        if (posInFrame_ != curHdiHandlePos) {
            posInFrame_ = curHdiHandlePos;
            timeInNano_ = handleTime;
        }
    }
}

std::string AudioEndpointInner::GetStatusStr(EndpointStatus status)
{
    switch (status) {
        case INVALID:
            return "INVALID";
        case UNLINKED:
            return "UNLINKED";
        case IDEL:
            return "IDEL";
        case STARTING:
            return "STARTING";
        case RUNNING:
            return "RUNNING";
        case STOPPING:
            return "STOPPING";
        case STOPPED:
            return "STOPPED";
        default:
            break;
    }
    return "NO_SUCH_STATUS";
}

bool AudioEndpointInner::KeepWorkloopRunning()
{
    EndpointStatus targetStatus = INVALID;
    switch (endpointStatus_.load()) {
        case RUNNING:
            return true;
        case IDEL:
            if (ClockTime::GetCurNano() > delayStopTime_) {
                targetStatus = RUNNING;
                updateThreadCV_.notify_all();
                break;
            }
            if (isDeviceRunningInIdel_) {
                return true;
            }
            break;
        case UNLINKED:
            targetStatus = IDEL;
            break;
        case STARTING:
            targetStatus = RUNNING;
            break;
        case STOPPING:
            targetStatus = STOPPED;
            break;
        default:
            break;
    }

    // when return false, EndpointWorkLoopFuc will continue loop immediately. Wait to avoid a inifity loop.
    std::unique_lock<std::mutex> lock(loopThreadLock_);
    AUDIO_PRERELEASE_LOGI("%{public}s now, wait for %{public}s...", GetStatusStr(endpointStatus_).c_str(),
        GetStatusStr(targetStatus).c_str());
    threadStatus_ = WAITTING;
    workThreadCV_.wait_for(lock, std::chrono::milliseconds(SLEEP_TIME_IN_DEFAULT));
    AUDIO_DEBUG_LOG("Wait end. Cur is %{public}s now, target is %{public}s...", GetStatusStr(endpointStatus_).c_str(),
        GetStatusStr(targetStatus).c_str());

    return false;
}

int32_t AudioEndpointInner::WriteToSpecialProcBuf(const std::shared_ptr<OHAudioBufferBase> &procBuf,
    const BufferDesc &readBuf, const BufferDesc &convertedBuffer, bool muteFlag)
{
    CHECK_AND_RETURN_RET_LOG(procBuf != nullptr, ERR_INVALID_HANDLE, "process buffer is null.");
    uint64_t curWritePos = procBuf->GetCurWriteFrame();
    Trace trace("AudioEndpoint::WriteProcessData-<" + std::to_string(curWritePos));

    int32_t writeAbleSize = procBuf->GetWritableDataFrames();
    if (writeAbleSize <= 0 || static_cast<uint32_t>(writeAbleSize) <= dstSpanSizeInframe_) {
        AUDIO_WARNING_LOG("client read too slow: curWritePos:%{public}" PRIu64" writeAbleSize:%{public}d",
            curWritePos, writeAbleSize);
        return ERR_OPERATION_FAILED;
    }

    RingBufferWrapper ringBuffer;
    int32_t ret = procBuf->GetAllWritableBufferFromPosFrame(curWritePos, ringBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get write buffer fail, ret %{public}d.", ret);

    uint32_t totalSizeInFrame;
    uint32_t byteSizePerFrame;
    procBuf->GetSizeParameter(totalSizeInFrame, byteSizePerFrame);
    CHECK_AND_RETURN_RET_LOG(byteSizePerFrame > 0, ERR_OPERATION_FAILED, "byteSizePerFrame is 0");
    uint32_t writeableSizeInFrame = ringBuffer.dataLength / byteSizePerFrame;
    if (writeableSizeInFrame > dstSpanSizeInframe_) {
        ringBuffer.dataLength = dstSpanSizeInframe_ * byteSizePerFrame;
    }

    if (muteFlag) {
        ringBuffer.SetBuffersValueWithSpecifyDataLen(0);
    } else {
        ret = HandleCapturerDataParams(ringBuffer, readBuf, convertedBuffer);
    }

    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "memcpy data to process buffer fail, "
        "curWritePos %{public}" PRIu64", ret %{public}d.", curWritePos, ret);

    procBuf->SetHandleInfo(curWritePos, ClockTime::GetCurNano());
    ret = procBuf->SetCurWriteFrame(curWritePos + dstSpanSizeInframe_);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set procBuf next write frame fail, ret %{public}d.", ret);
        return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

int32_t AudioEndpointInner::WriteToRingBuffer(RingBufferWrapper &writeBuf, const BufferDesc &buffer)
{
    CHECK_AND_RETURN_RET_LOG(buffer.buffer != nullptr && buffer.bufLength > 0, ERR_WRITE_FAILED, "failed");
    return writeBuf.CopyInputBufferValueToCurBuffer(RingBufferWrapper{
        .basicBufferDescs = {{
            {.buffer = buffer.buffer, .bufLength = buffer.bufLength},
            {.buffer = nullptr, .bufLength = 0}}},
        .dataLength = buffer.bufLength
    });
}

int32_t AudioEndpointInner::HandleCapturerDataParams(RingBufferWrapper &writeBuf, const BufferDesc &readBuf,
    const BufferDesc &convertedBuffer)
{
    if (clientConfig_.streamInfo.format == SAMPLE_S16LE && clientConfig_.streamInfo.channels == STEREO) {
        return WriteToRingBuffer(writeBuf, readBuf);
    }
    if (clientConfig_.streamInfo.format == SAMPLE_S16LE && clientConfig_.streamInfo.channels == MONO) {
        int32_t ret = FormatConverter::S16StereoToS16Mono(readBuf, convertedBuffer);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "Convert channel from stereo to mono failed");
        ret = WriteToRingBuffer(writeBuf, convertedBuffer);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "memcpy_s failed");
        ret = memset_s(static_cast<void *>(convertedBuffer.buffer), convertedBuffer.bufLength, 0,
            convertedBuffer.bufLength);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "memset converted buffer to 0 failed");
        return EOK;
    }
    if (clientConfig_.streamInfo.format == SAMPLE_F32LE) {
        int32_t ret = 0;
        if (clientConfig_.streamInfo.channels == STEREO) {
            ret = FormatConverter::S16StereoToF32Stereo(readBuf, convertedBuffer);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED,
                "Convert channel from s16 stereo to f32 stereo failed");
        } else if (clientConfig_.streamInfo.channels == MONO) {
            ret = FormatConverter::S16StereoToF32Mono(readBuf, convertedBuffer);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED,
                "Convert channel from s16 stereo to f32 mono failed");
        } else {
            return ERR_NOT_SUPPORTED;
        }
        ret = WriteToRingBuffer(writeBuf, convertedBuffer);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "memcpy_s failed");
        ret = memset_s(static_cast<void *>(convertedBuffer.buffer), convertedBuffer.bufLength, 0,
            convertedBuffer.bufLength);
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "memset converted buffer to 0 failed");
        return EOK;
    }

    return ERR_NOT_SUPPORTED;
}

void AudioEndpointInner::WriteToProcessBuffers(const BufferDesc &readBuf)
{
    CheckRecordSignal(readBuf.buffer, readBuf.bufLength);
    std::lock_guard<std::mutex> lock(listLock_);
    for (size_t i = 0; i < processBufferList_.size(); i++) {
        CHECK_AND_CONTINUE_LOG(processBufferList_[i] != nullptr, "process buffer %{public}zu is null.", i);
        if (processBufferList_[i]->GetStreamStatus() &&
            processBufferList_[i]->GetStreamStatus()->load() != STREAM_RUNNING) {
            AUDIO_WARNING_LOG("process buffer %{public}zu not running, stream status %{public}d.",
                i, processBufferList_[i]->GetStreamStatus()->load());
            continue;
        }

        int32_t ret = WriteToSpecialProcBuf(processBufferList_[i], readBuf, processList_[i]->GetConvertedBuffer(),
            processList_[i]->GetMuteState());
        CHECK_AND_CONTINUE_LOG(ret == SUCCESS,
            "endpoint write to process buffer %{public}zu fail, ret %{public}d.", i, ret);
        AUDIO_DEBUG_LOG("endpoint process buffer %{public}zu write success.", i);
    }
}

int32_t AudioEndpointInner::ReadFromEndpoint(uint64_t curReadPos)
{
    Trace trace("AudioEndpoint::ReadDstBuffer=<" + std::to_string(curReadPos));
    AUDIO_DEBUG_LOG("ReadFromEndpoint enter, dstAudioBuffer curReadPos %{public}" PRIu64".", curReadPos);
    CHECK_AND_RETURN_RET_LOG(dstAudioBuffer_ != nullptr, ERR_INVALID_HANDLE,
        "dst audio buffer is null.");
    SpanInfo *curReadSpan = dstAudioBuffer_->GetSpanInfo(curReadPos);
    CHECK_AND_RETURN_RET_LOG(curReadSpan != nullptr, ERR_INVALID_HANDLE,
        "get source read span info of source adapter fail.");
    curReadSpan->readStartTime = ClockTime::GetCurNano();
    curReadSpan->spanStatus.store(SpanStatus::SPAN_READING);
    BufferDesc readBuf;
    int32_t ret = dstAudioBuffer_->GetReadbuffer(curReadPos, readBuf);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get read buffer fail, ret %{public}d.", ret);
    VolumeTools::DfxOperation(readBuf, dstStreamInfo_, logUtilsTag_, volumeDataCount_);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpHdi_, static_cast<void *>(readBuf.buffer), readBuf.bufLength);
        AudioCacheMgr::GetInstance().CacheData(dumpHdiName_,
            static_cast<void *>(readBuf.buffer), readBuf.bufLength);
    }
    WriteToProcessBuffers(readBuf);
    ret = memset_s(readBuf.buffer, readBuf.bufLength, 0, readBuf.bufLength);
    if (ret != EOK) {
        AUDIO_WARNING_LOG("reset buffer fail, ret %{public}d.", ret);
    }
    curReadSpan->readDoneTime = ClockTime::GetCurNano();
    curReadSpan->spanStatus.store(SpanStatus::SPAN_READ_DONE);
    return SUCCESS;
}

void AudioEndpointInner::RecordEndpointWorkLoopFuc()
{
    SetThreadQosLevel();
    int64_t curTime = 0;
    uint64_t curReadPos = 0;
    int64_t wakeUpTime = ClockTime::GetCurNano();
    AUDIO_INFO_LOG("Record endpoint work loop fuc start.");
    while (isInited_.load()) {
        if (!KeepWorkloopRunning()) {
            continue;
        }
        threadStatus_ = INRUNNING;
        if (needReSyncPosition_) {
            RecordReSyncPosition();
            wakeUpTime = ClockTime::GetCurNano();
            needReSyncPosition_ = false;
            continue;
        }
        curTime = ClockTime::GetCurNano();
        Trace loopTrace("Record_loop_trace");
        if (curTime - wakeUpTime > THREE_MILLISECOND_DURATION) {
            AUDIO_WARNING_LOG("Wake up cost %{public}" PRId64" ms!", (curTime - wakeUpTime) / AUDIO_US_PER_SECOND);
        } else if (curTime - wakeUpTime > ONE_MILLISECOND_DURATION) {
            AUDIO_DEBUG_LOG("Wake up cost %{public}" PRId64" ms!", (curTime - wakeUpTime) / AUDIO_US_PER_SECOND);
        }

        curReadPos = dstAudioBuffer_->GetCurReadFrame();
        CHECK_AND_BREAK_LOG(ReadFromEndpoint(curReadPos) == SUCCESS, "read from endpoint to process service fail.");

        bool ret = RecordPrepareNextLoop(curReadPos, wakeUpTime);
        CHECK_AND_BREAK_LOG(ret, "PrepareNextLoop failed!");

        ProcessUpdateAppsUidForRecord();

        loopTrace.End();
        threadStatus_ = SLEEPING;
        CheckWakeUpTime(wakeUpTime);
        ClockTime::AbsoluteSleep(wakeUpTime);
    }
    ResetThreadQosLevel();
}

void AudioEndpointInner::BindCore()
{
    if (coreBinded_) {
        return;
    }
    // bind cpu cores 2-7 for fast mixer
    cpu_set_t targetCpus;
    CPU_ZERO(&targetCpus);
    int32_t cpuNum = sysconf(_SC_NPROCESSORS_CONF);
    for (int32_t i = CPU_INDEX; i < cpuNum; i++) {
        CPU_SET(i, &targetCpus);
    }

    int32_t ret = sched_setaffinity(gettid(), sizeof(cpu_set_t), &targetCpus);
    if (ret != 0) {
        AUDIO_ERR_LOG("set target cpu failed, set ret: %{public}d", ret);
    }
    AUDIO_INFO_LOG("set pid: %{public}d, tid: %{public}d cpus", getpid(), gettid());
    coreBinded_ = true;
}

void AudioEndpointInner::CheckTimeAndBufferReady(uint64_t &curWritePos, int64_t &wakeUpTime, int64_t &curTime)
{
    int64_t deltaTime = curTime - wakeUpTime;
    if (deltaTime > THREE_MILLISECOND_DURATION) {
        AUDIO_WARNING_LOG("Wake up cost %{public}" PRId64" ms!", deltaTime / AUDIO_US_PER_SECOND);
    } else if (deltaTime > ONE_MILLISECOND_DURATION) {
        AUDIO_DEBUG_LOG("Wake up cost %{public}" PRId64" ms!", deltaTime / AUDIO_US_PER_SECOND);
    }

    // First, wake up at client may-write-done time, and check if all process write done.
    // If not, do another sleep to the possible latest write time.
    curWritePos = dstAudioBuffer_->GetCurWriteFrame();
    if (!CheckAllBufferReady(wakeUpTime, curWritePos)) { curTime = ClockTime::GetCurNano(); }
}

void AudioEndpointInner::CheckWakeUpTime(int64_t &wakeUpTime)
{
    int64_t curTime = ClockTime::GetCurNano();
    if (wakeUpTime - curTime > MAX_WAKEUP_TIME_NS) {
        wakeUpTime = curTime + RELATIVE_SLEEP_TIME_NS;
    }
}

void AudioEndpointInner::EndpointWorkLoopFuc()
{
    BindCore();
    bool setPriorityResult = SetEndpointThreadPriority();
    if (!setPriorityResult) {
        SetThreadQosLevel();
    }
    int64_t curTime = 0;
    uint64_t curWritePos = 0;
    int64_t wakeUpTime = ClockTime::GetCurNano();
    AUDIO_INFO_LOG("Endpoint work loop fuc start");
    while (isInited_.load()) {
        if (!KeepWorkloopRunning()) {
            continue;
        }
        threadStatus_ = INRUNNING;
        curTime = ClockTime::GetCurNano();
        Trace loopTrace("AudioEndpoint::loop_trace " + std::to_string(wakeUpTime));
        if (needReSyncPosition_) {
            ReSyncPosition();
            wakeUpTime = curTime;
            needReSyncPosition_ = false;
            continue;
        }

        CheckTimeAndBufferReady(curWritePos, wakeUpTime, curTime);

        std::function<void()> moveClientIndex;
        // then do mix & write to hdi buffer and prepare next loop
        CHECK_AND_BREAK_LOG(ProcessToEndpointDataHandle(curWritePos, moveClientIndex),
            "ProcessToEndpointDataHandle failed!");

        // prepare info of next loop
        CHECK_AND_BREAK_LOG(PrepareNextLoop(curWritePos, wakeUpTime, moveClientIndex),
            "ProcessToEndpointDataHandle failed!");

        ProcessUpdateAppsUidForPlayback();

        loopTrace.End();
        // start sleep
        threadStatus_ = SLEEPING;
        CheckWakeUpTime(wakeUpTime);
        ClockTime::AbsoluteSleep(wakeUpTime);
    }
    AUDIO_DEBUG_LOG("Endpoint work loop fuc end");
    if (setPriorityResult) {
        ResetEndpointThreadPriority();
    } else {
        ResetThreadQosLevel();
    }
}

void AudioEndpointInner::ProcessUpdateAppsUidForPlayback()
{
    std::vector<int32_t> appsUid;
    {
        std::lock_guard<std::mutex> lock(listLock_);

        appsUid.reserve(processList_.size());
        for (auto iProccessStream : processList_) {
            appsUid.push_back(iProccessStream->GetAppInfo().appUid);
        }
    }
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
    CHECK_AND_RETURN_LOG(sink, "fastSink_ is nullptr");
    sink->UpdateAppsUid(appsUid);
}

void AudioEndpointInner::ProcessUpdateAppsUidForRecord()
{
    std::vector<int32_t> appsUid;
    {
        std::lock_guard<std::mutex> lock(listLock_);

        appsUid.reserve(processList_.size());
        for (auto iProccessStream : processList_) {
            appsUid.push_back(iProccessStream->GetAppInfo().appUid);
        }
    }
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
    CHECK_AND_RETURN_LOG(source, "fastSource_ is nullptr");
    source->UpdateAppsUid(appsUid);
}

uint32_t AudioEndpointInner::GetLinkedProcessCount()
{
    std::lock_guard<std::mutex> lock(listLock_);
    return processList_.size();
}

bool AudioEndpointInner::IsInvalidBuffer(uint8_t *buffer, size_t bufferSize, AudioSampleFormat format)
{
    bool isInvalid = false;
    uint8_t ui8Data = 0;
    int16_t i16Data = 0;
    switch (format) {
        case SAMPLE_U8:
            CHECK_AND_RETURN_RET_LOG(bufferSize > 0, false, "buffer size is too small");
            ui8Data = *buffer;
            isInvalid = ui8Data == 0;
            break;
        case SAMPLE_S16LE:
            CHECK_AND_RETURN_RET_LOG(bufferSize > 1, false, "buffer size is too small");
            i16Data = *(reinterpret_cast<const int16_t*>(buffer));
            isInvalid = i16Data == 0;
            break;
        default:
            break;
    }
    return isInvalid;
}

void AudioEndpointInner::WriteMuteDataSysEvent(uint8_t *buffer, size_t bufferSize, int32_t index)
{
    CHECK_AND_RETURN_LOG(static_cast<size_t>(index + 1) <= processList_.size(), "invalid index");
    auto tempProcess = processList_[index];
    CHECK_AND_RETURN_LOG(tempProcess, "tempProcess is nullptr");
    if (IsInvalidBuffer(buffer, bufferSize, processList_[index]->GetStreamInfo().format)) {
        if (tempProcess->GetStartMuteTime() == 0) {
            tempProcess->SetStartMuteTime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        }
        std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if ((currentTime - tempProcess->GetStartMuteTime() >= ONE_MINUTE) && !tempProcess->GetSilentState()) {
            tempProcess->SetSilentState(true);
            AUDIO_WARNING_LOG("write invalid data for some time in server");
 
            std::unordered_map<std::string, std::string> payload;
            payload["uid"] = std::to_string(tempProcess->GetAppInfo().appUid);
            payload["sessionId"] = std::to_string(tempProcess->GetAudioSessionId());
            payload["isSilent"] = std::to_string(true);
#ifdef RESSCHE_ENABLE
            ReportDataToResSched(payload, ResourceSchedule::ResType::RES_TYPE_AUDIO_RENDERER_SILENT_PLAYBACK);
#endif
        }
    } else {
        if (tempProcess->GetStartMuteTime() != 0) {
            tempProcess->SetStartMuteTime(0);
        }
        if (tempProcess->GetSilentState()) {
            AUDIO_WARNING_LOG("begin write valid data in server");
            tempProcess->SetSilentState(false);

            std::unordered_map<std::string, std::string> payload;
            payload["uid"] = std::to_string(tempProcess->GetAppInfo().appUid);
            payload["sessionId"] = std::to_string(tempProcess->GetAudioSessionId());
            payload["isSilent"] = std::to_string(false);
#ifdef RESSCHE_ENABLE
            ReportDataToResSched(payload, ResourceSchedule::ResType::RES_TYPE_AUDIO_RENDERER_SILENT_PLAYBACK);
#endif
        }
    }
}

void AudioEndpointInner::ReportDataToResSched(std::unordered_map<std::string, std::string> payload, uint32_t type)
{
#ifdef RESSCHE_ENABLE
    AUDIO_INFO_LOG("report event to ResSched ,event type : %{public}d", type);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, payload);
#endif
}

int32_t AudioEndpointInner::CreateDupBufferInner(int32_t innerCapId)
{
    // todo dynamic
    if (innerCapIdToDupStreamCallbackMap_[innerCapId] == nullptr ||
        innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() != nullptr) {
        AUDIO_INFO_LOG("dup buffer already configed!");
        return SUCCESS;
    }

    auto &capInfo = fastCaptureInfos_[innerCapId];

    capInfo.dupStream->GetSpanSizePerFrame(dupSpanSizeInFrame_);
    dupTotalSizeInFrame_ = dupSpanSizeInFrame_ * (DUP_COMMON_LEN/DUP_DEFAULT_LEN);
    capInfo.dupStream->GetByteSizePerFrame(dupByteSizePerFrame_);
    if (dupSpanSizeInFrame_ == 0 || dupByteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("ERR_INVALID_PARAM");
        return ERR_INVALID_PARAM;
    }
    dupSpanSizeInByte_ = dupSpanSizeInFrame_ * dupByteSizePerFrame_;
    CHECK_AND_RETURN_RET_LOG(dupSpanSizeInByte_ != 0, ERR_OPERATION_FAILED, "Config dup buffer failed");
    AUDIO_INFO_LOG("dupTotalSizeInFrame_: %{public}zu, dupSpanSizeInFrame_: %{public}zu,"
        "dupByteSizePerFrame_:%{public}zu dupSpanSizeInByte_: %{public}zu,",
        dupTotalSizeInFrame_, dupSpanSizeInFrame_, dupByteSizePerFrame_, dupSpanSizeInByte_);
 
    // create dupBuffer in server
    innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() =
        AudioRingCache::Create(dupTotalSizeInFrame_ * dupByteSizePerFrame_);
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() != nullptr,
        ERR_OPERATION_FAILED, "Create dup buffer failed");
    size_t emptyBufferSize = static_cast<size_t>(dupSpanSizeInFrame_) * dupByteSizePerFrame_;
    auto buffer = std::make_unique<uint8_t []>(emptyBufferSize);
    BufferDesc emptyBufferDesc = {buffer.get(), emptyBufferSize, emptyBufferSize};
    memset_s(emptyBufferDesc.buffer, emptyBufferDesc.bufLength, 0, emptyBufferDesc.bufLength);
    WriteDupBufferInner(emptyBufferDesc, innerCapId);
    return SUCCESS;
}
 
int32_t AudioEndpointInner::WriteDupBufferInner(const BufferDesc &bufferDesc, int32_t innerCapId)
{
    size_t targetSize = bufferDesc.bufLength;

    if (innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() == nullptr) {
        AUDIO_INFO_LOG("dup buffer is nnullptr, failed WriteDupBuffer!");
        return ERROR;
    }
    OptResult result = innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer()->GetWritableSize();
    // todo get writeable size failed
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "DupRingBuffer write invalid size is:%{public}zu", result.size);
    size_t writableSize = result.size;
    AUDIO_DEBUG_LOG("targetSize: %{public}zu, writableSize: %{public}zu", targetSize, writableSize);
    size_t writeSize = std::min(writableSize, targetSize);
    BufferWrap bufferWrap = {bufferDesc.buffer, writeSize};

    if (writeSize > 0) {
        result = innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer()->Enqueue(bufferWrap);
        if (result.ret != OPERATION_SUCCESS) {
            AUDIO_ERR_LOG("RingCache Enqueue failed ret:%{public}d size:%{public}zu", result.ret, result.size);
        }
        DumpFileUtil::WriteDumpFile(dumpDupIn_, static_cast<void *>(bufferDesc.buffer), writeSize);
    }
    return SUCCESS;
}

void AudioEndpointInner::CheckAudioHapticsSync(uint64_t curWritePos)
{
    if (audioHapticsSyncId_ > 0) {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        if (sink != nullptr) {
            uint64_t offset = dstSpanSizeInframe_ * curWritePos;
            std::string condition = "AudioHapticsSync";
            std::string value = "haptic_sessionid=" + std::to_string(audioHapticsSyncId_) +
                ";haptic_offset=" + std::to_string(offset);
            sink->SetAudioParameter(AudioParamKey::NONE, condition, value);
        }
        audioHapticsSyncId_ = 0;
    }
}
} // namespace AudioStandard
} // namespace OHOS
