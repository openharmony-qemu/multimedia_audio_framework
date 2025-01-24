/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "RemoteAudioRendererSinkInner"
#endif

#include "remote_audio_renderer_sink.h"

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <list>
#include <string>
#include <sstream>
#include <unistd.h>
#include <map>
#include <mutex>
#include "securec.h"
#include <algorithm>

#include <v1_0/iaudio_manager.h>

#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "audio_utils.h"
#include "i_audio_device_adapter.h"
#include "i_audio_device_manager.h"
#include "audio_log_utils.h"
#include "audio_dump_pcm.h"

using namespace std;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioAdapter;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioAdapterDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortDirection;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioManager;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioCategory;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRouteNode;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRoute;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSceneDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortPin;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortType;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortRole;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t HALF_FACTOR = 2;
const float DEFAULT_VOLUME_LEVEL = 1.0f;
const uint32_t AUDIO_CHANNELCOUNT = 2;
const uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
const uint32_t DEEP_BUFFER_RENDER_PERIOD_SIZE = 4096;
const uint32_t INT_32_MAX = 0x7fffffff;
const uint32_t PCM_8_BIT = 8;
const uint32_t PCM_16_BIT = 16;
const uint32_t REMOTE_OUTPUT_STREAM_ID = 29; // 13 + 2 * 8

const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;

const string MEDIA_STREAM_TYPE = "1";
const string COMMUNICATION_STREAM_TYPE = "2";
const string NAVIGATION_STREAM_TYPE = "13";
uint32_t MEDIA_RENDERID = 0;
uint32_t NAVIGATION_RENDERID = 1;
uint32_t COMMUNICATION_RENDERID = 2;
}
class RemoteAudioRendererSinkInner : public RemoteAudioRendererSink, public IAudioDeviceAdapterCallback {
public:
    explicit RemoteAudioRendererSinkInner(const std::string &deviceNetworkId);
    ~RemoteAudioRendererSinkInner();

    int32_t Init(const IAudioSinkAttr &attr) override;
    bool IsInited(void) override;
    void DeInit(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t Pause(void) override;
    int32_t Resume(void) override;
    int32_t SuspendRenderSink(void) override;
    int32_t RestoreRenderSink(void) override;

    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override;
    int32_t SplitRenderFrame(char &data, uint64_t len, uint64_t &writeLen, char *streamType) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetVoiceVolume(float volume) override;
    int32_t GetTransactionId(uint64_t *transactionId) override;
    int32_t GetLatency(uint32_t *latency) override;
    int32_t GetAudioScene() override;
    int32_t SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices) override;
    int32_t SetOutputRoutes(std::vector<DeviceType> &outputDevices) override;
    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void SetAudioMonoState(bool audioMono) override;
    void SetAudioBalanceValue(float audioBalance) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    void RegisterParameterCallback(IAudioSinkCallback* callback) override;
    void ResetOutputRouteForDisconnect(DeviceType device) override;
    int32_t SetPaPower(int32_t flag) override;
    int32_t SetPriPaPower() override;

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;
    float GetMaxAmplitude() override;

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    std::string GetNetworkId();
    IAudioSinkCallback* GetParamCallback();

private:
    int32_t CreateRender(const struct AudioPort &renderPort, AudioCategory type, uint32_t &renderId);
    void InitAttrs(struct AudioSampleAttributes &attrs);
    void splitStreamInit(const char *splitStreamString, vector<string> &splitStreamVector);
    int32_t RenderFrameLogic(char &data, uint64_t len, uint64_t &writeLen, const char *streamType);
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    int32_t OpenOutput(DeviceType outputDevice);
    void ClearRender();

    void CheckUpdateState(char *frame, uint64_t replyBytes);
    void DfxOperation(BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel) const;
private:
    std::string deviceNetworkId_ = "";
    std::atomic<bool> rendererInited_ = false;
    std::atomic<bool> isRenderCreated_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;
    float leftVolume_ = DEFAULT_VOLUME_LEVEL;
    float rightVolume_ = DEFAULT_VOLUME_LEVEL;

    std::shared_ptr<IAudioDeviceManager> audioManager_ = nullptr;
    std::mutex audioMangerMutex_;

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter_ = nullptr;
    std::mutex audioAdapterMutex_;

    IAudioSinkCallback *callback_ = nullptr;
    IAudioSinkAttr attr_ = {};
    unordered_map<AudioCategory, sptr<IAudioRender>> audioRenderMap_;
    unordered_map<AudioCategory, AudioPort> audioPortMap_;
    unordered_map<string, AudioCategory> splitStreamMap_;
    unordered_map<AudioCategory, FILE*> dumpFileMap_;
    unordered_map<AudioCategory, std::string> dumpFileNameMap_;
    std::mutex createRenderMutex_;
    vector<uint32_t> renderIdVector_ = {MEDIA_RENDERID, NAVIGATION_RENDERID, COMMUNICATION_RENDERID};
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int renderFrameNum_ = 0;
    std::string logUtilsTag_ = "Remote";
    mutable int64_t volumeDataCount_ = 0;
};

RemoteAudioRendererSinkInner::RemoteAudioRendererSinkInner(const std::string &deviceNetworkId)
    :deviceNetworkId_(deviceNetworkId)
{
    AUDIO_DEBUG_LOG("RemoteAudioRendererSinkInner constract.");
}

RemoteAudioRendererSinkInner::~RemoteAudioRendererSinkInner()
{
    if (rendererInited_.load()) {
        RemoteAudioRendererSinkInner::DeInit();
    }
    AUDIO_DEBUG_LOG("RemoteAudioRendererSink destruction.");
}

std::mutex g_rendererSinksMutex;
std::map<std::string, RemoteAudioRendererSinkInner *> allsinks;
RemoteAudioRendererSink *RemoteAudioRendererSink::GetInstance(const std::string &deviceNetworkId)
{
    std::lock_guard<std::mutex> lock(g_rendererSinksMutex);
    AUDIO_INFO_LOG("RemoteAudioRendererSink::GetInstance");
    CHECK_AND_RETURN_RET_LOG(!deviceNetworkId.empty(), nullptr, "Remote render device networkId is null.");

    if (allsinks.count(deviceNetworkId)) {
        return allsinks[deviceNetworkId];
    }
    RemoteAudioRendererSinkInner *audioRenderer = new(std::nothrow) RemoteAudioRendererSinkInner(deviceNetworkId);
    AUDIO_INFO_LOG("New daudio remote render device [%{public}s].", GetEncryptStr(deviceNetworkId).c_str());
    allsinks[deviceNetworkId] = audioRenderer;
    return audioRenderer;
}

void RemoteAudioRendererSinkInner::ClearRender()
{
    AUDIO_INFO_LOG("Clear remote audio render enter.");
    rendererInited_.store(false);
    isRenderCreated_.store(false);
    started_.store(false);
    paused_.store(false);

    auto renderId = renderIdVector_.begin();
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = std::move(audioAdapter_);
        audioAdapter_ = nullptr;
    }

    if (audioAdapter != nullptr) {
        for (auto &audioRender : audioRenderMap_) {
            audioAdapter->DestroyRender(audioRender.second, *renderId);
            audioRender.second = nullptr;
            renderId++;
            FILE *dumpFile = dumpFileMap_[audioRender.first];
            DumpFileUtil::CloseDumpFile(&dumpFile);
        }
        audioAdapter->Release();
    }
    audioRenderMap_.clear();
    audioAdapter = nullptr;

    std::shared_ptr<IAudioDeviceManager> audioManager;
    {
        std::lock_guard<std::mutex> lock(audioMangerMutex_);
        audioManager = std::move(audioManager_);
        audioManager_ = nullptr;
    }

    if (audioManager != nullptr) {
        audioManager->UnloadAdapter(deviceNetworkId_);
    }
    audioManager = nullptr;

    AudioDeviceManagerFactory::GetInstance().DestoryDeviceManager(REMOTE_DEV_MGR);

    dumpFileMap_.clear();
    dumpFileNameMap_.clear();
    AUDIO_INFO_LOG("Clear remote audio render end.");
}

void RemoteAudioRendererSinkInner::DeInit()
{
    Trace trace("RemoteAudioRendererSinkInner::DeInit");
    std::lock_guard<std::mutex> lock(g_rendererSinksMutex);
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::DeInit");
    ClearRender();

    // remove map recorder.
    CHECK_AND_RETURN_LOG(allsinks.count(this->deviceNetworkId_) > 0,
        "not find %{public}s", this->deviceNetworkId_.c_str());
    RemoteAudioRendererSinkInner *temp = allsinks[this->deviceNetworkId_];
    allsinks.erase(this->deviceNetworkId_);
    if (temp == nullptr) {
        AUDIO_ERR_LOG("temp is nullptr");
    } else {
        delete temp;
    }
    AUDIO_INFO_LOG("end.");
}

inline std::string PrintRemoteAttr(const IAudioSinkAttr &attr)
{
    std::stringstream value;
    value << "adapterName[" << attr.adapterName << "] openMicSpeaker[" << attr.openMicSpeaker << "] ";
    value << "format[" << static_cast<int32_t>(attr.format) << "] ";
    value << "sampleRate[" << attr.sampleRate << "] channel[" << attr.channel << "] ";
    value << "volume[" << attr.volume << "] filePath[" << attr.filePath << "] ";
    value << "deviceNetworkId[" << attr.deviceNetworkId << "] device_type[" << attr.deviceType << "]";
    return value.str();
}

bool RemoteAudioRendererSinkInner::IsInited()
{
    return rendererInited_.load();
}

int32_t RemoteAudioRendererSinkInner::Init(const IAudioSinkAttr &attr)
{
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::Init");
    attr_ = attr;
    splitStreamMap_[MEDIA_STREAM_TYPE] = AudioCategory::AUDIO_IN_MEDIA;
    splitStreamMap_[NAVIGATION_STREAM_TYPE] = AudioCategory::AUDIO_IN_NAVIGATION;
    splitStreamMap_[COMMUNICATION_STREAM_TYPE] = AudioCategory::AUDIO_IN_COMMUNICATION;
    vector<string> splitStreamVector;
    splitStreamInit(attr_.aux, splitStreamVector);
    auto audioManager = AudioDeviceManagerFactory::GetInstance().CreatDeviceManager(REMOTE_DEV_MGR);
    {
        std::lock_guard<std::mutex> lock(audioMangerMutex_);
        audioManager_ = audioManager;
    }
    CHECK_AND_RETURN_RET_LOG(audioManager != nullptr, ERR_NOT_STARTED, "Init audio manager fail");
    struct AudioAdapterDescriptor *desc = audioManager->GetTargetAdapterDesc(deviceNetworkId_, false);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NOT_STARTED, "Get target adapters descriptor fail.");
    auto splitStreamTypeIter = splitStreamVector.begin();
    for (uint32_t port = 0; port < desc->ports.size(); port++) {
        if (desc->ports[port].portId == AudioPortPin::PIN_OUT_SPEAKER) {
            AUDIO_INFO_LOG("current audio stream type is %{public}s, port index is %{public}d",
                splitStreamTypeIter->c_str(), port);
            while (splitStreamTypeIter != splitStreamVector.end()) {
                audioPortMap_[splitStreamMap_[*splitStreamTypeIter]] = desc->ports[port];
                splitStreamTypeIter++;
            }
        }
    }

    auto audioAdapter = audioManager->LoadAdapters(deviceNetworkId_, false);
    CHECK_AND_RETURN_RET_LOG(audioAdapter != nullptr, ERR_NOT_STARTED, "Load audio device adapter failed.");

    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter_ = audioAdapter;
    }

    int32_t ret = audioAdapter->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Audio adapter init fail, ret %{public}d.", ret);

    rendererInited_.store(true);

    AUDIO_DEBUG_LOG("RemoteAudioRendererSink: Init end.");
    return SUCCESS;
}

void RemoteAudioRendererSinkInner::splitStreamInit(const char *splitStreamString, vector<string> &splitStreamVector)
{
    AUDIO_INFO_LOG("audio split stream is %{public}s", splitStreamString);
    if (splitStreamString == nullptr) {
        splitStreamVector.push_back("1");
        AUDIO_INFO_LOG("audio split stream is default 1");
        return;
    }

    istringstream iss(splitStreamString);
    std::string currentSplitStream;
    while (getline(iss, currentSplitStream, ':')) {
        splitStreamVector.push_back(currentSplitStream);
        AUDIO_INFO_LOG("current split stream type is %{public}s", currentSplitStream.c_str());
    }
    sort(splitStreamVector.begin(), splitStreamVector.end());
}

int32_t RemoteAudioRendererSinkInner::CreateRender(const struct AudioPort &renderPort, AudioCategory type,
    uint32_t &renderId)
{
    int64_t start = ClockTime::GetCurNano();
    struct AudioSampleAttributes param;
    InitAttrs(param);
    param.type = type;
    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    AUDIO_DEBUG_LOG("Create render format: %{public}d", param.format);

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = renderPort.portId;
    deviceDesc.pins = AudioPortPin::PIN_OUT_SPEAKER;
    deviceDesc.desc = "";

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = audioAdapter_;
    }

    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_INVALID_HANDLE, "CreateRender: Audio adapter is null.");
    sptr<IAudioRender> audioRender = nullptr;
    int32_t ret = audioAdapter_->CreateRender(deviceDesc, param, audioRender, this, renderId);
    audioRenderMap_[type] = audioRender;
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && audioRender != nullptr, ret,
        "AudioDeviceCreateRender fail, ret %{public}d.", ret);

    isRenderCreated_.store(true);
    int64_t cost = (ClockTime::GetCurNano() - start) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("CreateRender cost[%{public}" PRId64 "]ms", cost);
    return SUCCESS;
}

void RemoteAudioRendererSinkInner::InitAttrs(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = 0;
    attrs.streamId = REMOTE_OUTPUT_STREAM_ID;
    attrs.type = AudioCategory::AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.stopThreshold = INT_32_MAX;
    attrs.silenceThreshold = 0;
}

AudioFormat RemoteAudioRendererSinkInner::ConvertToHdiFormat(HdiAdapterFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }

    return hdiFormat;
}

int32_t RemoteAudioRendererSinkInner::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    Trace trace("RemoteAudioRendererSinkInner::RenderFrame");
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::RenderFrame");
    const char *mediaStreamType = "1";
    return RenderFrameLogic(data, len, writeLen, mediaStreamType);
}

int32_t RemoteAudioRendererSinkInner::SplitRenderFrame(char &data, uint64_t len, uint64_t &writeLen, char *streamType)
{
        Trace trace("RemoteAudioRendererSinkInner::SplitRenderFrame");
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::SplitRenderFrame");
    return RenderFrameLogic(data, len, writeLen, streamType);
}

int32_t RemoteAudioRendererSinkInner::RenderFrameLogic(char &data, uint64_t len, uint64_t &writeLen,
    const char *streamType)
{
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::RenderFrameLogic, streamType is %{public}s", streamType);
    int64_t start = ClockTime::GetCurNano();
    sptr<IAudioRender> audioRender_ = audioRenderMap_[splitStreamMap_[streamType]];
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "RenderFrame: Audio render is null.");

    if (!started_.load()) {
        AUDIO_DEBUG_LOG("RemoteAudioRendererSinkInner::RenderFrameLogic invalid state not started!");
    }
    std::vector<int8_t> frameHal(len);
    int32_t ret = memcpy_s(frameHal.data(), len, &data, len);
    if (ret != EOK) {
        AUDIO_ERR_LOG("Copy render frame failed, error code %d.", ret);
        return ERR_OPERATION_FAILED;
    }

    BufferDesc buffer = { reinterpret_cast<uint8_t*>(&data), len, len };
    DfxOperation(buffer, static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    Trace traceRenderFrame("audioRender_->RenderFrame");
    Trace::CountVolume("RemoteAudioRendererSinkInner::RenderFrameLogic", static_cast<uint8_t>(data));
    ret = audioRender_->RenderFrame(frameHal, writeLen);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_FAILED, "Render frame fail, ret %{public}x.", ret);
    writeLen = len;

    FILE *dumpFile = dumpFileMap_[splitStreamMap_[streamType]];
    std::string dumpFileName = dumpFileNameMap_[splitStreamMap_[streamType]];
    DumpFileUtil::WriteDumpFile(dumpFile, static_cast<void *>(&data), len);
    AudioCacheMgr::GetInstance().CacheData(dumpFileName, static_cast<void *>(&data), len);

    CheckUpdateState(&data, len);

    int64_t cost = (ClockTime::GetCurNano() - start) / AUDIO_US_PER_SECOND;
    AUDIO_DEBUG_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", len, cost);

    int64_t stampThreshold = 50; // 50ms
    if (cost >= stampThreshold) {
        AUDIO_WARNING_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", len, cost);
    }

    return SUCCESS;
}

void RemoteAudioRendererSinkInner::CheckUpdateState(char *frame, uint64_t replyBytes)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), frame, replyBytes);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

float RemoteAudioRendererSinkInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t RemoteAudioRendererSinkInner::Start(void)
{
    Trace trace("RemoteAudioRendererSinkInner::Start");
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::Start");
    std::lock_guard<std::mutex> lock(createRenderMutex_);

    for (const auto &audioPort : audioPortMap_) {
        FILE *dumpFile = nullptr;
        std::string dumpFileName = std::string(DUMP_REMOTE_RENDER_SINK_FILENAME) + "_" + GetTime() + "_" +
            std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel) + "_" +
            std::to_string(attr_.format) + ".pcm";
        DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, dumpFileName, &dumpFile);
        dumpFileMap_[audioPort.first] = dumpFile;
        dumpFileNameMap_[audioPort.first] = dumpFileName;
    }
    
    auto renderId = renderIdVector_.begin();
    if (!isRenderCreated_.load()) {
        for (const auto &audioPort : audioPortMap_) {
            CHECK_AND_RETURN_RET_LOG(CreateRender(audioPort.second, audioPort.first, *renderId) == SUCCESS,
                ERR_NOT_STARTED, "Create render fail, audio port %{public}d", audioPort.second.portId);
            renderId++;
        }
    }

    if (started_.load()) {
        AUDIO_INFO_LOG("Remote render is already started.");
        return SUCCESS;
    }

    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "Start: Audio render is null. Audio steam type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->Start();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "Start fail, ret %{public}d.", ret);
    }
    started_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::Stop(void)
{
    Trace trace("RemoteAudioRendererSinkInner::Stop");
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::Stop");
    if (!started_.load()) {
        AUDIO_INFO_LOG("Remote render is already stopped.");
        return SUCCESS;
    }

    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "Stop: Audio render is null.Audio stream type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->Stop();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Stop fail, ret %{public}d.", ret);
    }
    started_.store(false);
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::Pause(void)
{
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::Pause");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Pause invalid state!");

    if (paused_.load()) {
        AUDIO_INFO_LOG("Remote render is already paused.");
        return SUCCESS;
    }

    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "Pause: Audio render is null. Audio stream type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->Pause();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Pause fail, ret %{public}d.", ret);
    }
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::Resume(void)
{
    Trace trace("RemoteAudioRendererSinkInner::Resume");
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::Resume");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Resume invalid state!");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("Remote render is already resumed.");
        return SUCCESS;
    }

    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "Resume: Audio render is null.Audio stream type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->Resume();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Resume fail, ret %{public}d.", ret);
    }

    paused_.store(false);
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::Reset(void)
{
    Trace trace("RemoteAudioRendererSinkInner::Reset");
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::Reset");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Reset invalid state!");

    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "Reset: Audio render is null.Audio stream type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->Flush();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Reset fail, ret %{public}d.", ret);
    }
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::Flush(void)
{
    Trace trace("RemoteAudioRendererSinkInner::Flush");
    AUDIO_INFO_LOG("RemoteAudioRendererSinkInner::Flush");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Flush invalid state!");

    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "Flush: Audio render is null.Audio stream type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->Flush();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Flush fail, ret %{public}d.", ret);
    }
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::RestoreRenderSink(void)
{
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::SetVolume(float left, float right)
{
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
    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "SetVolume: Audio render is null. Audio stream type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->SetVolume(volume);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Set volume fail, ret %{public}d.", ret);
    }
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::GetLatency(uint32_t *latency)
{
    CHECK_AND_RETURN_RET_LOG(latency, ERR_INVALID_PARAM,
        "GetLatency failed latency null");

    uint32_t hdiLatency = 0;
    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "GetLatency: Audio render is null. Audio stream type is %{public}d", audioRender.first);
        int32_t ret = audioRender.second->GetLatency(hdiLatency);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Get latency fail, ret %{public}d.", ret);
    }

    *latency = hdiLatency;
    return SUCCESS;
}

static AudioCategory GetAudioCategory(AudioScene audioScene)
{
    AudioCategory audioCategory;
    switch (audioScene) {
        case AUDIO_SCENE_DEFAULT:
            audioCategory = AudioCategory::AUDIO_IN_MEDIA;
            break;
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_VOICE_RINGING:
            audioCategory = AudioCategory::AUDIO_IN_RINGTONE;
            break;
        case AUDIO_SCENE_PHONE_CALL:
            audioCategory = AudioCategory::AUDIO_IN_CALL;
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            audioCategory = AudioCategory::AUDIO_IN_COMMUNICATION;
            break;
        default:
            audioCategory = AudioCategory::AUDIO_IN_MEDIA;
            break;
    }
    AUDIO_DEBUG_LOG("Audio category returned is: %{public}d", audioCategory);

    return audioCategory;
}

static int32_t SetOutputPortPin(DeviceType outputDevice, AudioRouteNode &sink)
{
    int32_t ret = SUCCESS;

    switch (outputDevice) {
        case DEVICE_TYPE_SPEAKER:
            sink.ext.device.type = AudioPortPin::PIN_OUT_SPEAKER;
            sink.ext.device.desc = "pin_out_speaker";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            sink.ext.device.type = AudioPortPin::PIN_OUT_HEADSET;
            sink.ext.device.desc = "pin_out_headset";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            sink.ext.device.type = AudioPortPin::PIN_OUT_USB_EXT;
            sink.ext.device.desc = "pin_out_usb_ext";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t RemoteAudioRendererSinkInner::OpenOutput(DeviceType outputDevice)
{
    AudioRouteNode source = {};
    AudioRouteNode sink = {};

    int32_t ret = SetOutputPortPin(outputDevice, sink);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Set output port pin fail, ret %{public}d", ret);

    source.portId = 0;
    source.role = AudioPortRole::AUDIO_PORT_SOURCE_ROLE;
    source.type = AudioPortType::AUDIO_PORT_MIX_TYPE;
    source.ext.mix.moduleId = 0;
    source.ext.mix.streamId = REMOTE_OUTPUT_STREAM_ID;

    if (audioPortMap_.find(AudioCategory::AUDIO_IN_MEDIA) == audioPortMap_.end()) {
        AUDIO_WARNING_LOG("audioPortMap_ is null, ret %{public}d.", ret);
        return ERR_INVALID_HANDLE;
    }
    sink.portId = static_cast<int32_t>(audioPortMap_[AudioCategory::AUDIO_IN_MEDIA].portId);
    sink.role = AudioPortRole::AUDIO_PORT_SINK_ROLE;
    sink.type = AudioPortType::AUDIO_PORT_DEVICE_TYPE;
    sink.ext.device.moduleId = 0;

    AudioRoute route;
    route.sources.push_back(source);
    route.sinks.push_back(sink);

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = audioAdapter_;
    }

    CHECK_AND_RETURN_RET_LOG(audioAdapter != nullptr, ERR_INVALID_HANDLE, "OpenOutput: Audio adapter is null.");
    ret = audioAdapter->UpdateAudioRoute(route);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Update audio route fail, ret %{public}d", ret);
    return SUCCESS;
}

int32_t RemoteAudioRendererSinkInner::GetAudioScene()
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRendererSinkInner::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    CHECK_AND_RETURN_RET_LOG(!activeDevices.empty() && activeDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "Invalid audio devices.");
    DeviceType activeDevice = activeDevices.front();
    AUDIO_INFO_LOG("SetAudioScene scene: %{public}d, device: %{public}d", audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "invalid audioScene");

    int32_t ret = OpenOutput(DEVICE_TYPE_SPEAKER);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("Audio adapter update audio route fail, ret %{public}d.", ret);
    }

    struct AudioSceneDescriptor scene;
    scene.scene.id = GetAudioCategory(audioScene);
    scene.desc.pins = AudioPortPin::PIN_OUT_SPEAKER;

    AUDIO_DEBUG_LOG("SelectScene start");
    for (const auto &audioRender : audioRenderMap_) {
        CHECK_AND_RETURN_RET_LOG(audioRender.second != nullptr, ERR_INVALID_HANDLE,
            "SetAudioScene: Audio render is null. Audio stream type is %{public}d", audioRender.first);
        ret = audioRender.second->SelectScene(scene);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED,
            "Audio render Select scene fail, ret %{public}d.", ret);
    }
    AUDIO_DEBUG_LOG("Select audio scene SUCCESS: %{public}d", audioScene);
    return SUCCESS;
}

void RemoteAudioRendererSinkInner::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
#ifdef FEATURE_DISTRIBUTE_AUDIO
    AUDIO_INFO_LOG("SetParameter: key %{public}d, condition: %{public}s, value: %{public}s",
        key, condition.c_str(), value.c_str());

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = audioAdapter_;
    }

    CHECK_AND_RETURN_LOG(audioAdapter != nullptr, "SetAudioParameter: Audio adapter is null.");
    audioAdapter->SetAudioParameter(key, condition.c_str(), value.c_str());
#endif
}

std::string RemoteAudioRendererSinkInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
#ifdef FEATURE_DISTRIBUTE_AUDIO
    AUDIO_INFO_LOG("key %{public}d, condition: %{public}s", key, condition.c_str());

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = audioAdapter_;
    }

    CHECK_AND_RETURN_RET_LOG(audioAdapter != nullptr, "", "Audio adapter is null.");
    return audioAdapter->GetAudioParameter(key, condition);
#else
    return "";
#endif
}

void RemoteAudioRendererSinkInner::RegisterParameterCallback(IAudioSinkCallback* callback)
{
    AUDIO_INFO_LOG("register sink audio param callback.");
    callback_ = callback;
#ifdef FEATURE_DISTRIBUTE_AUDIO
    // register to remote audio adapter

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = audioAdapter_;
    }

    CHECK_AND_RETURN_LOG(audioAdapter != nullptr, "RegisterParameterCallback: Audio adapter is null.");
    int32_t ret = audioAdapter->RegExtraParamObserver();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RegisterParameterCallback failed, ret %{public}d.", ret);
#endif
}

void RemoteAudioRendererSinkInner::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("Audio param change event, key:%{public}d, condition:%{public}s, value:%{public}s",
        key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        ClearRender();
    }

    CHECK_AND_RETURN_LOG(callback_ != nullptr, "Sink audio param callback is null.");
    callback_->OnAudioSinkParamChange(adapterName, key, condition, value);
}

int32_t RemoteAudioRendererSinkInner::GetTransactionId(uint64_t *transactionId)
{
    (void)transactionId;
    AUDIO_ERR_LOG("GetTransactionId not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRendererSinkInner::SetVoiceVolume(float volume)
{
    (void)volume;
    AUDIO_ERR_LOG("SetVoiceVolume not supported");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRendererSinkInner::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    (void)outputDevices;
    AUDIO_DEBUG_LOG("SetOutputRoutes not supported.");
    return ERR_NOT_SUPPORTED;
}

void RemoteAudioRendererSinkInner::SetAudioMonoState(bool audioMono)
{
    (void)audioMono;
    AUDIO_ERR_LOG("SetAudioMonoState not supported");
    return;
}

void RemoteAudioRendererSinkInner::SetAudioBalanceValue(float audioBalance)
{
    (void)audioBalance;
    AUDIO_ERR_LOG("SetAudioBalanceValue not supported");
    return;
}

int32_t RemoteAudioRendererSinkInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_ERR_LOG("GetPresentationPosition not supported");
    return ERR_NOT_SUPPORTED;
}

std::string RemoteAudioRendererSinkInner::GetNetworkId()
{
    return deviceNetworkId_;
}

void RemoteAudioRendererSinkInner::ResetOutputRouteForDisconnect(DeviceType device)
{
    AUDIO_WARNING_LOG("not supported.");
}

OHOS::AudioStandard::IAudioSinkCallback* RemoteAudioRendererSinkInner::GetParamCallback()
{
    return callback_;
}

int32_t RemoteAudioRendererSinkInner::SetPaPower(int32_t flag)
{
    (void)flag;
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRendererSinkInner::SetPriPaPower()
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRendererSinkInner::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioRendererSinkInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return ERR_NOT_SUPPORTED;
}

void RemoteAudioRendererSinkInner::DfxOperation(BufferDesc &buffer, AudioSampleFormat format,
    AudioChannel channel) const
{
    ChannelVolumes vols = VolumeTools::CountVolumeLevel(buffer, format, channel);
    if (channel == MONO) {
        Trace::Count(logUtilsTag_, vols.volStart[0]);
    } else {
        Trace::Count(logUtilsTag_, (vols.volStart[0] + vols.volStart[1]) / HALF_FACTOR);
    }
    AudioLogUtils::ProcessVolumeData(logUtilsTag_, vols, volumeDataCount_);
}
} // namespace AudioStandard
} // namespace OHOS
