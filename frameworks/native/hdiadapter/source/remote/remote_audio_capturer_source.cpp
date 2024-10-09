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
#define LOG_TAG "RemoteAudioCapturerSourceInner"
#endif

#include "remote_audio_capturer_source.h"

#include <cstring>
#include <dlfcn.h>
#include <string>
#include <mutex>
#include "securec.h"

#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "audio_utils.h"
#include "i_audio_device_adapter.h"
#include "i_audio_device_manager.h"

using namespace std;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioAdapter;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioAdapterDescriptor;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioFormat;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort;
using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortDirection;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioManager;
using OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture;
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
const uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
}

class RemoteAudioCapturerSourceInner : public RemoteAudioCapturerSource, public IAudioDeviceAdapterCallback {
public:
    explicit RemoteAudioCapturerSourceInner(const std::string &deviceNetworkId);
    ~RemoteAudioCapturerSourceInner();

    int32_t Init(const IAudioSourceAttr &attr) override;
    bool IsInited(void) override;
    void DeInit(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t Pause(void) override;
    int32_t Resume(void) override;
    int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;
    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice) override;
    int32_t SetInputRoute(DeviceType inputDevice) override;
    uint64_t GetTransactionId() override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;

    void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value) override;
    
    float GetMaxAmplitude() override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;
private:
    int32_t CreateCapture(const AudioPort &capturePort);
    int32_t SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source);
    AudioCategory GetAudioCategory(AudioScene audioScene);
    void ClearCapture();
    AudioFormat ConvertToHdiFormat(HdiAdapterFormat format);
    void CheckUpdateState(char *frame, uint64_t replyBytes);

private:
    static constexpr uint32_t REMOTE_INPUT_STREAM_ID = 30; // 14 + 2 * 8
    const uint32_t maxInt32 = 0x7fffffff;
    const uint32_t audioBufferSize = 16 * 1024;
    const uint32_t deepBufferCapturePeriodSize = 4096;
    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int capFrameNum_ = 0;

    IAudioSourceAttr attr_ = {};
    std::string deviceNetworkId_ = "";
    std::atomic<bool> capturerInited_ = false;
    std::atomic<bool> isCapturerCreated_ = false;
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = false;

    std::shared_ptr<IAudioDeviceManager> audioManager_ = nullptr;
    std::mutex audioMangerMutex_;

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter_ = nullptr;
    std::mutex audioAdapterMutex_;

    IAudioSourceCallback *paramCb_ = nullptr;
    sptr<IAudioCapture> audioCapture_ = nullptr;
    struct AudioPort audioPort_ = {};
    FILE *dumpFile_ = nullptr;
    bool muteState_ = false;
    std::mutex createCaptureMutex_;
    uint32_t captureId_ = 0;
};

std::mutex g_capturerRemoteSourcesMutex;
std::map<std::string, RemoteAudioCapturerSourceInner *> allRemoteSources;
RemoteAudioCapturerSource *RemoteAudioCapturerSource::GetInstance(const std::string &deviceNetworkId)
{
    std::lock_guard<std::mutex> lock(g_capturerRemoteSourcesMutex);
    AUDIO_INFO_LOG("GetInstance.");
    bool isEmpty = deviceNetworkId.empty();
    CHECK_AND_RETURN_RET_LOG(!isEmpty, nullptr, "Remote capture device networkId is null.");

    if (allRemoteSources.count(deviceNetworkId)) {
        return allRemoteSources[deviceNetworkId];
    }
    RemoteAudioCapturerSourceInner *audioCapturer = new(std::nothrow) RemoteAudioCapturerSourceInner(deviceNetworkId);
    AUDIO_INFO_LOG("New daudio remote capture device [%{public}s].", GetEncryptStr(deviceNetworkId).c_str());
    allRemoteSources[deviceNetworkId] = audioCapturer;
    return audioCapturer;
}

void RemoteAudioCapturerSource::GetAllInstance(std::vector<IAudioCapturerSource *> &allInstance)
{
    std::lock_guard<std::mutex> lock(g_capturerRemoteSourcesMutex);
    for (auto it = allRemoteSources.begin(); it != allRemoteSources.end(); it++) {
        allInstance.push_back((*it).second);
    }
}

RemoteAudioCapturerSourceInner::RemoteAudioCapturerSourceInner(const std::string &deviceNetworkId)
    : deviceNetworkId_(deviceNetworkId) {}

RemoteAudioCapturerSourceInner::~RemoteAudioCapturerSourceInner()
{
    if (capturerInited_.load()) {
        RemoteAudioCapturerSourceInner::DeInit();
    } else {
        AUDIO_DEBUG_LOG("RemoteAudioCapturerSource has already DeInit.");
    }
}

void RemoteAudioCapturerSourceInner::ClearCapture()
{
    AUDIO_INFO_LOG("Clear capture enter.");
    capturerInited_.store(false);
    isCapturerCreated_.store(false);
    started_.store(false);
    paused_.store(false);

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = std::move(audioAdapter_);
        audioAdapter_ = nullptr;
    }

    if (audioAdapter != nullptr) {
        audioAdapter->DestroyCapture(audioCapture_, captureId_);
        audioAdapter->Release();
    }
    audioCapture_ = nullptr;
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
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    AUDIO_DEBUG_LOG("Clear capture end.");
}

void RemoteAudioCapturerSourceInner::DeInit()
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::DeInit");
    ClearCapture();

    std::lock_guard<std::mutex> lock(g_capturerRemoteSourcesMutex);
    // remove map recorder.

    CHECK_AND_RETURN_LOG(allRemoteSources.count(this->deviceNetworkId_) > 0,
        "not find %{public}s", this->deviceNetworkId_.c_str());

    RemoteAudioCapturerSource *temp = allRemoteSources[this->deviceNetworkId_];
    allRemoteSources.erase(this->deviceNetworkId_);
    if (temp == nullptr) {
        AUDIO_ERR_LOG("temp is nullptr");
    } else {
        delete temp;
    }

    AUDIO_INFO_LOG("end.");
}

int32_t RemoteAudioCapturerSourceInner::Init(const IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::Init");
    attr_ = attr;

    auto audioManager = AudioDeviceManagerFactory::GetInstance().CreatDeviceManager(REMOTE_DEV_MGR);
    CHECK_AND_RETURN_RET_LOG(audioManager != nullptr, ERR_NOT_STARTED, "Init audio manager fail.");

    {
        std::lock_guard<std::mutex> lock(audioMangerMutex_);
        audioManager_ = audioManager;
    }

    struct AudioAdapterDescriptor *desc = audioManager->GetTargetAdapterDesc(deviceNetworkId_, false);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, ERR_NOT_STARTED, "Get target adapters descriptor fail.");
    for (uint32_t port = 0; port < desc->ports.size(); port++) {
        if (desc->ports[port].portId == AudioPortPin::PIN_IN_MIC) {
            audioPort_ = desc->ports[port];
            break;
        }
        CHECK_AND_RETURN_RET_LOG(port != (desc->ports.size() - 1), ERR_INVALID_INDEX,
            "Not found the audio mic port.");
    }

    auto audioAdapter = audioManager->LoadAdapters(deviceNetworkId_, false);
    CHECK_AND_RETURN_RET_LOG(audioAdapter != nullptr, ERR_NOT_STARTED, "Load audio device adapter failed.");

    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter_ = audioAdapter;
    }

    int32_t ret = audioAdapter->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Audio adapter init fail, ret %{public}d.", ret);

    capturerInited_.store(true);

    SetMute(muteState_);

    AUDIO_DEBUG_LOG("RemoteAudioCapturerSource: Init end.");
    return SUCCESS;
}

bool RemoteAudioCapturerSourceInner::IsInited(void)
{
    return capturerInited_.load();
}

AudioFormat RemoteAudioCapturerSourceInner::ConvertToHdiFormat(HdiAdapterFormat format)
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

int32_t RemoteAudioCapturerSourceInner::CreateCapture(const struct AudioPort &capturePort)
{
    struct AudioSampleAttributes param;
    param.type = AudioCategory::AUDIO_IN_MEDIA;
    param.period = deepBufferCapturePeriodSize;
    param.streamId = REMOTE_INPUT_STREAM_ID;
    param.isSignedData = true;
    param.stopThreshold = maxInt32;
    param.silenceThreshold = audioBufferSize;
    // User needs to set
    param.sampleRate = attr_.sampleRate;
    param.format = ConvertToHdiFormat(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    param.startThreshold = deepBufferCapturePeriodSize / (param.frameSize);
    param.sourceType = attr_.sourceType;

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = capturePort.portId;
    deviceDesc.pins = AudioPortPin::PIN_IN_MIC;
    deviceDesc.desc = "";

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = audioAdapter_;
    }

    CHECK_AND_RETURN_RET_LOG(audioAdapter != nullptr, ERR_INVALID_HANDLE, "CreateCapture: Audio adapter is null.");
    int32_t ret = audioAdapter->CreateCapture(deviceDesc, param, audioCapture_, this, captureId_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && audioCapture_ != nullptr, ret,
        "Create capture failed, ret %{public}d.", ret);

    isCapturerCreated_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG((audioCapture_ != nullptr), ERR_INVALID_HANDLE, "CaptureFrame: Audio capture is null.");
    if (!started_.load()) {
        AUDIO_DEBUG_LOG("AudioRendererSinkInner::RenderFrame invalid state not started!");
    }
    std::vector<int8_t> frameHal(requestBytes);
    int32_t ret = audioCapture_->CaptureFrame(frameHal, replyBytes);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_READ_FAILED, "Capture frame fail, ret %{public}x.", ret);

    ret = memcpy_s(frame, requestBytes, frameHal.data(), requestBytes);
    if (ret != EOK) {
        AUDIO_ERR_LOG("Copy capture frame failed, error code %d.", ret);
        return ERR_MEMORY_ALLOC_FAILED;
    }
    replyBytes = requestBytes;

    DumpFileUtil::WriteDumpFile(dumpFile_, frame, requestBytes);
    CheckUpdateState(frame, requestBytes);

    return SUCCESS;
}

void RemoteAudioCapturerSourceInner::CheckUpdateState(char *frame, uint64_t replyBytes)
{
    if (startUpdate_) {
        if (capFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        capFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), frame, replyBytes);
        if (capFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            capFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
                maxAmplitude_ = 0;
            }
        }
    }
}

float RemoteAudioCapturerSourceInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t RemoteAudioCapturerSourceInner::Start(void)
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::Start");
    std::lock_guard<std::mutex> lock(createCaptureMutex_);
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, DUMP_REMOTE_CAPTURE_SOURCE_FILENAME, &dumpFile_);
    if (!isCapturerCreated_.load()) {
        CHECK_AND_RETURN_RET_LOG(CreateCapture(audioPort_) == SUCCESS, ERR_NOT_STARTED,
            "Create capture fail, audio port %{public}d", audioPort_.portId);
    }

    if (started_.load()) {
        AUDIO_INFO_LOG("Remote capture is already started.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG((audioCapture_ != nullptr), ERR_INVALID_HANDLE, "Audio capture Handle is nullptr!");
    int32_t ret = audioCapture_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "Start fail, ret %{public}d.", ret);
    started_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::Stop(void)
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::Stop");
    if (!started_.load()) {
        AUDIO_INFO_LOG("Remote capture is already stopped.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Stop: Audio capture is null.");
    int32_t ret = audioCapture_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Stop fail, ret %{public}d.", ret);
    started_.store(false);
    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::Pause(void)
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::Pause");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Pause invalid state!");

    if (paused_.load()) {
        AUDIO_INFO_LOG("Remote render is already paused.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Pause: Audio capture is null.");
    int32_t ret = audioCapture_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Pause fail, ret %{public}d.", ret);
    paused_.store(true);
    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::Resume(void)
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::Resume");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Resume invalid state!");

    if (!paused_.load()) {
        AUDIO_INFO_LOG("Remote render is already resumed.");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Resume: Audio capture is null.");
    int32_t ret = audioCapture_->Resume();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Resume fail, ret %{public}d.", ret);
    paused_.store(false);
    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::Reset(void)
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::Reset");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Reset invalid state!");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Reset: Audio capture is null.");
    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Reset fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::Flush(void)
{
    AUDIO_INFO_LOG("RemoteAudioCapturerSourceInner::Flush");
    CHECK_AND_RETURN_RET_LOG(started_.load(), ERR_ILLEGAL_STATE, "Flush invalid state!");

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Flush: Audio capture is null.");
    int32_t ret = audioCapture_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "Flush fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::SetVolume(float left, float right)
{
    // remote setvolume may not supported
    float volume = 0.5;
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "SetVolume: Audio capture is null.");

    float leftVolume = left;
    float rightVolume = right;
    float half = 0.5;
    if ((leftVolume == 0) && (rightVolume != 0)) {
        volume = rightVolume;
    } else if ((leftVolume != 0) && (rightVolume == 0)) {
        volume = leftVolume;
    } else {
        volume = (leftVolume + rightVolume) * half;
    }

    int32_t ret = audioCapture_->SetVolume(volume);
    AUDIO_INFO_LOG("remote setVolume(%{public}f, %{public}f):%{public}d", left, right, ret);
    return ret;
}

int32_t RemoteAudioCapturerSourceInner::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "GetVolume: Audio capture is null.");
    float val = 0.0;
    audioCapture_->GetVolume(val);
    left = val;
    right = val;

    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::SetMute(bool isMute)
{
    muteState_ = isMute;

    if (!IsInited()) {
        AUDIO_INFO_LOG("SetMute before init, only record mute state");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "SetMute: Audio capture is null.");

    int32_t ret = audioCapture_->SetMute(isMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("SetMute failed from hdi");
    }

    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::GetMute(bool &isMute)
{
    isMute = muteState_;

    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "GetMute: Audio capture is null.");

    bool isHdiMute = false;
    int32_t ret = audioCapture_->GetMute(isHdiMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioCapturerSource::GetMute failed from hdi");
    }

    return SUCCESS;
}

int32_t RemoteAudioCapturerSourceInner::SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source)
{
    int32_t ret = SUCCESS;

    switch (inputDevice) {
        case DEVICE_TYPE_MIC:
            source.ext.device.type = AudioPortPin::PIN_IN_MIC;
            source.ext.device.desc = "pin_in_mic";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            source.ext.device.type = AudioPortPin::PIN_IN_HS_MIC;
            source.ext.device.desc = "pin_in_hs_mic";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            source.ext.device.type = AudioPortPin::PIN_IN_USB_EXT;
            source.ext.device.desc = "pin_in_usb_ext";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t RemoteAudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice)
{
    AudioRouteNode source = {};
    AudioRouteNode sink = {};

    int32_t ret = SetInputPortPin(inputDevice, source);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Set input port pin fail, ret %{public}d", ret);

    source.portId = static_cast<int32_t>(audioPort_.portId);
    source.role = AudioPortRole::AUDIO_PORT_SOURCE_ROLE;
    source.type = AudioPortType::AUDIO_PORT_DEVICE_TYPE;
    source.ext.device.moduleId = 0;

    sink.portId = 0;
    sink.role = AudioPortRole::AUDIO_PORT_SINK_ROLE;
    sink.type = AudioPortType::AUDIO_PORT_MIX_TYPE;
    sink.ext.mix.moduleId = 0;
    sink.ext.mix.streamId = REMOTE_INPUT_STREAM_ID;

    AudioRoute route;
    route.sources.push_back(source);
    route.sinks.push_back(sink);

    std::shared_ptr<IAudioDeviceAdapter> audioAdapter;
    {
        std::lock_guard<std::mutex> lock(audioAdapterMutex_);
        audioAdapter = audioAdapter_;
    }

    CHECK_AND_RETURN_RET_LOG(audioAdapter != nullptr, ERR_INVALID_HANDLE, "SetInputRoute: Audio adapter is null.");
    ret = audioAdapter->UpdateAudioRoute(route);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Update audio route fail, ret %{public}d", ret);
    return SUCCESS;
}

AudioCategory RemoteAudioCapturerSourceInner::GetAudioCategory(AudioScene audioScene)
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
    AUDIO_DEBUG_LOG("RemoteAudioCapturerSource: Audio category returned is: %{public}d", audioCategory);

    return audioCategory;
}

int32_t RemoteAudioCapturerSourceInner::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "SetAudioScene: Audio capture is null.");
    struct AudioSceneDescriptor scene;
    scene.scene.id = GetAudioCategory(audioScene);
    scene.desc.pins = AudioPortPin::PIN_IN_MIC;
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX,
        ERR_INVALID_PARAM, "invalid audioScene");

    AUDIO_INFO_LOG("AudioCapturerSource::SelectScene start");
    int32_t ret = audioCapture_->SelectScene(scene);
    AUDIO_INFO_LOG("AudioCapturerSource::SelectScene over");
    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED,
        "AudioCapturerSource: Select scene FAILED: %{public}d", ret);
    AUDIO_INFO_LOG("AudioCapturerSource::Select audio scene SUCCESS: %{public}d", audioScene);
    return SUCCESS;
}

uint64_t RemoteAudioCapturerSourceInner::GetTransactionId()
{
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioCapturerSourceInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec,
    int64_t& timeNanoSec)
{
    return ERR_OPERATION_FAILED;
}

std::string RemoteAudioCapturerSourceInner::GetAudioParameter(const AudioParamKey key,
                                                              const std::string &condition)
{
    AUDIO_WARNING_LOG("not supported yet");
    return "";
}

void RemoteAudioCapturerSourceInner::RegisterWakeupCloseCallback(IAudioSourceCallback *callback)
{
    AUDIO_ERR_LOG("RegisterWakeupCloseCallback FAILED");
}

void RemoteAudioCapturerSourceInner::RegisterAudioCapturerSourceCallback(
    std::unique_ptr<ICapturerStateCallback> callback)
{
    AUDIO_ERR_LOG("RegisterAudioCapturerSourceCallback FAILED");
}

void RemoteAudioCapturerSourceInner::RegisterParameterCallback(IAudioSourceCallback *callback)
{
    AUDIO_INFO_LOG("register params callback");
    paramCb_ = callback;

#ifdef FEATURE_DISTRIBUTE_AUDIO
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

void RemoteAudioCapturerSourceInner::OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("Audio param change event, key:%{public}d, condition:%{public}s, value:%{public}s",
        key, condition.c_str(), value.c_str());
    if (key == AudioParamKey::PARAM_KEY_STATE) {
        ClearCapture();
    }

    CHECK_AND_RETURN_LOG(paramCb_ != nullptr, "Sink audio param callback is null.");
    paramCb_->OnAudioSourceParamChange(adapterName, key, condition, value);
}

int32_t RemoteAudioCapturerSourceInner::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteAudioCapturerSourceInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namesapce OHOS

