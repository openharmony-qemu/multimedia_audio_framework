/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "BluetoothCapturerSourceInner"
#endif

#include "bluetooth_capturer_source.h"

#include <cstring>
#include <dlfcn.h>
#include <string>
#include <cinttypes>
#include <thread>
#include <future>
#include <cstdio>
#include <unistd.h>

#include "securec.h"
#include "parameters.h"
#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#include "running_lock.h"
#include "audio_running_lock_manager.h"
#endif

#include "media_monitor_manager.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_proxy_manager.h"
#include "audio_enhance_chain_manager.h"
#include "audio_attribute.h"
#include "volume_tools.h"
#include "audio_dump_pcm.h"

using namespace std;
using namespace OHOS::HDI::Audio_Bluetooth;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
constexpr uint32_t DEEP_BUFFER_CAPTURE_PERIOD_SIZE = 4096;
constexpr uint32_t INT_32_MAX = 0x7fffffff;
constexpr uint32_t AUDIO_BUFF_SIZE = (16 * 1024);
constexpr uint32_t PCM_8_BIT = 8;
constexpr uint32_t PCM_16_BIT = 16;
constexpr uint16_t GET_MAX_AMPLITUDE_FRAMES_THRESHOLD = 10;
#ifdef FEATURE_POWER_MANAGER
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;
#endif
} // namespace

static AudioFormat ConvertToHdiFormat(HdiAdapterFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

static int32_t SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, uint32_t size,
    const std::string &adapterNameCase, enum AudioPortDirection portFlag, struct AudioPort &capturePort)
{
    AUDIO_INFO_LOG("SwitchAdapter: adapterNameCase: %{public}s", adapterNameCase.c_str());
    CHECK_AND_RETURN_RET(descs != nullptr, ERROR);

    for (uint32_t index = 0; index < size; ++index) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
        AUDIO_DEBUG_LOG("size: %{public}d, adapterNameCase %{public}s, adapterName %{public}s",
            size, adapterNameCase.c_str(), desc->adapterName);
        if (adapterNameCase.compare(desc->adapterName)) {
            continue;
        }
        for (uint32_t port = 0; port < desc->portNum; port++) {
            // only find out the port_in in the sound card
            if (desc->ports[port].dir == portFlag) {
                capturePort = desc->ports[port];
                return index;
            }
        }
    }
    AUDIO_ERR_LOG("SwitchAdapterCapture Fail");
    return ERR_INVALID_INDEX;
}

class BluetoothCapturerSourceInner : public BluetoothCapturerSource {
public:
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
    int32_t CaptureFrameWithEc(
        FrameDesc *fdesc, uint64_t &replyBytes,
        FrameDesc *fdescEc, uint64_t &replyBytesEc) override;

    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;

    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice,
        const std::string &deviceName) override;
    int32_t SetInputRoute(DeviceType inputDevice, const std::string &deviceName) override;
    uint64_t GetTransactionId() override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;
    float GetMaxAmplitude() override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;
    int32_t GetCaptureId(uint32_t &captureId) const override;

    explicit BluetoothCapturerSourceInner();
    ~BluetoothCapturerSourceInner() override;

private:
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr int32_t MAX_AUDIO_ADAPTER_NUM = 8;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;

    int32_t CreateCapture(struct AudioPort &capturePort);
    int32_t SetAudioRouteInfoForEnhanceChain(const DeviceType &inputDevice, const std::string &deviceName = "");
    int32_t InitAudioManager();
    void InitAttrsCapture(struct AudioSampleAttributes &attrs);

    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckLatencySignal(uint8_t *frame, size_t replyBytes);

    void CheckUpdateState(char *frame, uint64_t replyBytes);
    int32_t DoStop();

    IAudioSourceAttr attr_;
    bool captureInited_;
    bool started_;
    bool paused_;
    float leftVolume_ = 0.0;
    float rightVolume_ = 0.0;

    int32_t logMode_ = 0;
    mutable int64_t volumeDataCount_ = 0;

    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int capFrameNum_ = 0;

    struct HDI::Audio_Bluetooth::AudioProxyManager *audioManager_;
    struct HDI::Audio_Bluetooth::AudioAdapter *audioAdapter_;
    struct HDI::Audio_Bluetooth::AudioCapture *audioCapture_;
    struct HDI::Audio_Bluetooth::AudioPort audioPort_ = {};

    void *handle_;
    const std::string halName_;
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> runningLockManager_;
#endif

    DeviceType currentActiveDevice_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    std::unique_ptr<ICapturerStateCallback> audioCapturerSourceCallback_ = nullptr;
    FILE *dumpFile_ = nullptr;
    std::string dumpFileName_ = "";
    std::string logUtilsTag_ = "";
    bool muteState_ = false;

    bool latencyMeasEnabled_ = false;
    bool signalDetected_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    std::mutex signalDetectAgentMutex_;
    std::mutex managerAndAdapterMutex_;
    std::mutex statusMutex_;
};

BluetoothCapturerSourceInner::BluetoothCapturerSourceInner()
    : captureInited_(false), started_(false), paused_(false), audioManager_(nullptr), audioAdapter_(nullptr),
      audioCapture_(nullptr), handle_(nullptr), halName_ ("bt_hdap") {}

BluetoothCapturerSourceInner::~BluetoothCapturerSourceInner()
{
    BluetoothCapturerSourceInner::DeInit();
    AUDIO_INFO_LOG("[%{public}s] volume data counts: %{public}" PRIu64, logUtilsTag_.c_str(), volumeDataCount_);
}

BluetoothCapturerSource *BluetoothCapturerSource::GetInstance()
{
    Trace trace("BluetoothCapturerSourceInner:GetInstance");
    static BluetoothCapturerSourceInner audioCapturer;
    return &audioCapturer;
}

bool BluetoothCapturerSourceInner::IsInited(void)
{
    return captureInited_;
}

void BluetoothCapturerSourceInner::DeInit(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    Trace trace("BluetoothCapturerSourceInner::Deinit");
    started_ = false;

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyCapture(audioAdapter_, audioCapture_);
    }
    captureInited_ = false;
    audioCapture_ = nullptr;
    audioAdapter_ = nullptr;

    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

void BluetoothCapturerSourceInner::InitAttrsCapture(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for hdap record */
    attrs.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;

    attrs.type = AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    attrs.frameSize = PCM_16_BIT * attrs.channelCount / PCM_8_BIT;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (attrs.frameSize);
    attrs.stopThreshold = INT_32_MAX;
    /* 16 * 1024 */
    attrs.silenceThreshold = AUDIO_BUFF_SIZE;
}

int32_t BluetoothCapturerSourceInner::InitAudioManager()
{
#if (defined(__aarch64__) || defined(__x86_64__))
    char resolvedPath[100] = "/vendor/lib64/chipsetsdk/libaudio_bluetooth_hdi_proxy_server.z.so";
#else
    char resolvedPath[100] = "/vendor/lib/chipsetsdk/libaudio_bluetooth_hdi_proxy_server.z.so";
#endif
    struct AudioProxyManager *(*getAudioManager)() = nullptr;

    handle_ = dlopen(resolvedPath, 1);
    CHECK_AND_RETURN_RET_LOG(handle_ != nullptr, ERR_INVALID_HANDLE, "Open so Fail");
    AUDIO_DEBUG_LOG("dlopen successfully");

    getAudioManager = (struct AudioProxyManager *(*)())(dlsym(handle_, "GetAudioProxyManagerFuncs"));
    if (getAudioManager == nullptr) {
#ifndef TEST_COVERAGE
        dlclose(handle_);
#endif
        handle_ = nullptr;
        AUDIO_ERR_LOG("getaudiomanager fail!");
        return ERR_INVALID_HANDLE;
    }
    AUDIO_DEBUG_LOG("getaudiomanager done");

    audioManager_ = getAudioManager();
    if (audioManager_ == nullptr) {
#ifndef TEST_COVERAGE
        dlclose(handle_);
#endif
        handle_ = nullptr;
        AUDIO_ERR_LOG("getAudioManager() fail!");
        return ERR_INVALID_HANDLE;
    }
    AUDIO_DEBUG_LOG("audio manager created");

    return 0;
}

int32_t BluetoothCapturerSourceInner::CreateCapture(struct AudioPort &capturePort)
{
    Trace trace("BluetoothCapturerSourceInner::CreateCapture");

    struct AudioSampleAttributes param;
    // User needs to set
    InitAttrsCapture(param);
    param.sampleRate = attr_.sampleRate;
    param.format = ConvertToHdiFormat(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = capturePort.portId;
    deviceDesc.pins = PIN_IN_MIC;
    deviceDesc.desc = nullptr;

    AUDIO_INFO_LOG("create capture sourceName:%{public}s, " \
        "rate:%{public}u channel:%{public}u format:%{public}u, devicePin:%{public}u",
        halName_.c_str(), param.sampleRate, param.channelCount, param.format, deviceDesc.pins);
    int32_t ret = audioAdapter_->CreateCapture(audioAdapter_, &deviceDesc, &param, &audioCapture_);
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr && ret >=0, ERR_NOT_STARTED, "create capture failed");

    return 0;
}

int32_t BluetoothCapturerSourceInner::Init(const IAudioSourceAttr &attr)
{
    if (captureInited_) {
        AUDIO_INFO_LOG("Adapter already inited");
        return SUCCESS;
    }

    std::lock_guard<std::mutex> statusLock(statusMutex_);
    attr_ = attr;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);
    logUtilsTag_ = "A2dpSource";

    std::lock_guard lock(managerAndAdapterMutex_);
    CHECK_AND_RETURN_RET_LOG(InitAudioManager() == 0, ERR_NOT_STARTED, "Init audio manager Fail");

    int32_t size = 0;
    struct AudioAdapterDescriptor *descs = nullptr;
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, &descs, &size);

    CHECK_AND_RETURN_RET_LOG(size <= MAX_AUDIO_ADAPTER_NUM && size != 0 && descs != nullptr && ret == 0,
        ERR_NOT_STARTED, "Get adapters Fail");

    string adapterNameCase = "bt_hdap";
    enum AudioPortDirection port = PORT_IN;

    int32_t index = SwitchAdapterCapture(descs, size, adapterNameCase, port, audioPort_);
    CHECK_AND_RETURN_RET_LOG(index >= 0, ERR_NOT_STARTED, "Switch Adapter Capturer Fail");

    struct AudioAdapterDescriptor *desc = &descs[index];
    int32_t loadAdapter = audioManager_->LoadAdapter(audioManager_, desc, &audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(loadAdapter == 0, ERR_NOT_STARTED, "Load Adapter Fail");
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_NOT_STARTED, "Load audio device failed");

    // Initialize port information, can fill through mode and other parameters
    ret = audioAdapter_->InitAllPorts(audioAdapter_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_DEVICE_INIT, "initAllPorts failed");

    ret = CreateCapture(audioPort_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "create capture failed");

    ret = SetAudioRouteInfoForEnhanceChain(currentActiveDevice_, "");
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set device %{public}d failed", currentActiveDevice_);
    }

    captureInited_ = true;
    SetMute(muteState_);

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "Audio capture handle is nullpre");

    Trace trace("BluetoothCapturerSourceInner::CaptureFrame");

    int64_t stamp = ClockTime::GetCurNano();
    uint64_t frameLen = static_cast<uint64_t>(requestBytes);

    int32_t ret = audioCapture_->CaptureFrame(audioCapture_, reinterpret_cast<int8_t*>(frame), frameLen, &replyBytes);

    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_NOT_STARTED, "Capture Frame Fail");
    CheckLatencySignal(reinterpret_cast<uint8_t*>(frame), replyBytes);

    BufferDesc tmpBuffer = {reinterpret_cast<uint8_t*>(frame), replyBytes, replyBytes};
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(tmpBuffer, streamInfo, logUtilsTag_, volumeDataCount_);

    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, frame, replyBytes);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void*>(frame), replyBytes);
    }
    CheckUpdateState(frame, requestBytes);

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    if (logMode_) {
        AUDIO_DEBUG_LOG("RenderFrame len[%{public}" PRIu64 "] cost [%{public}" PRIu64 "]ms", requestBytes, stamp);
    }
    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::CaptureFrameWithEc(
    FrameDesc *fdesc, uint64_t &replyBytes,
    FrameDesc *fdescEc, uint64_t &replyBytesEc)
{
    AUDIO_ERR_LOG("Bluetooth captureFrameWithEc is not support!");
    return ERR_NOT_SUPPORTED;
}

void BluetoothCapturerSourceInner::CheckUpdateState(char *frame, uint64_t replyBytes)
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

float BluetoothCapturerSourceInner::GetMaxAmplitude()
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t BluetoothCapturerSourceInner::Start(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);

    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());
    Trace trace("BluetoothCapturerSourceInner::Start");

    InitLatencyMeasurement();
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock;
    if (runningLockManager_ == nullptr) {
        keepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioBluetoothCapturer",
            PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        if (keepRunningLock) {
            runningLockManager_ = std::make_shared<AudioRunningLockManager<PowerMgr::RunningLock>> (keepRunningLock);
        }
    }

    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock lock result: %{public}d",
            runningLockManager_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING));
    } else {
        AUDIO_WARNING_LOG("keepRunningLock is null, capture can not work well");
    }
#endif
    dumpFileName_ = halName_ + "_" + std::to_string(attr_.sourceType) + "_" + GetTime()
        + "_bluetooth_source_" + std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel)
        + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    if (!started_) {
        if (audioCapturerSourceCallback_ != nullptr) {
            audioCapturerSourceCallback_->OnCapturerState(true);
        }

        int32_t ret = audioCapture_->control.Start(reinterpret_cast<AudioHandle>(audioCapture_));
        if (ret < 0) {
#ifdef FEATURE_POWER_MANAGER
            if (runningLockManager_ != nullptr) {
                AUDIO_WARNING_LOG("capturer start failed, keepRunningLock unLock");
                runningLockManager_->UnLock();
            } else {
                AUDIO_WARNING_LOG("capturer start failed, try unlock but KeepRunningLock is null!");
            }
#endif
            return ERR_NOT_STARTED;
        }
        started_ = true;
    }

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::SetVolume(float left, float right)
{
    float volume;
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE,
        "SetVolume failed audioCapture_ null");

    rightVolume_ = right;
    leftVolume_ = left;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    audioCapture_->volume.SetVolume(reinterpret_cast<AudioHandle>(audioCapture_), volume);

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE,
        "GetVolume failed audioCapture_ null");

    float val = 0.0;
    audioCapture_->volume.GetVolume(reinterpret_cast<AudioHandle>(audioCapture_), &val);
    left = val;
    right = val;

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::SetMute(bool isMute)
{
    muteState_ = isMute;

    if (IsInited() && audioCapture_) {
        int32_t ret = audioCapture_->volume.SetMute(reinterpret_cast<AudioHandle>(audioCapture_), isMute);
        if (ret != 0) {
            AUDIO_WARNING_LOG("SetMute for hdi capturer failed");
        } else {
            AUDIO_INFO_LOG("SetMute for hdi capture success");
        }
    }

    AUDIO_INFO_LOG("end isMute=%{public}d", isMute);
    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::GetMute(bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE,
        "GetMute failed audioCapture_ handle is null!");

    bool isHdiMute = false;
    int32_t ret = audioCapture_->volume.GetMute(reinterpret_cast<AudioHandle>(audioCapture_), &isHdiMute);
    if (ret != 0) {
        AUDIO_WARNING_LOG("GetMute failed from hdi");
    }

    isMute = muteState_;
    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::SetInputRoute(DeviceType inputDevice, const std::string &deviceName)
{
    AUDIO_WARNING_LOG("SetInputRoutes not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothCapturerSourceInner::SetAudioRouteInfoForEnhanceChain(const DeviceType &inputDevice,
    const std::string &deviceName)
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
    uint32_t captureId = 0;
    int32_t ret = GetCaptureId(captureId);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("GetCaptureId failed");
    }

    audioEnhanceChainManager->SetInputDevice(captureId, inputDevice, deviceName);
    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::SetAudioScene(AudioScene audioScene, DeviceType activeDevice,
    const std::string &deviceName)
{
    AUDIO_WARNING_LOG("SetAudioScene not supported.");
    return ERR_NOT_SUPPORTED;
}

uint64_t BluetoothCapturerSourceInner::GetTransactionId()
{
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t BluetoothCapturerSourceInner::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_WARNING_LOG("GetPresentationPosition not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothCapturerSourceInner::DoStop()
{
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());

    Trace trace("BluetoothCapturerSourceInner::DoStop");

    DeinitLatencyMeasurement();

#ifdef FEATURE_POWER_MANAGER
    if (runningLockManager_ != nullptr) {
        AUDIO_INFO_LOG("keepRunningLock unLock");
        runningLockManager_->UnLock();
    } else {
        AUDIO_WARNING_LOG("KeepRunningLock is null, stop can not work well");
    }
#endif

    if (started_ && audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->control.Stop(reinterpret_cast<AudioHandle>(audioCapture_));
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "Stop capture Failed");
    }
    started_ = false;
    paused_ = false;

    if (audioCapturerSourceCallback_ != nullptr) {
        audioCapturerSourceCallback_->OnCapturerState(false);
    }

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::Stop(void)
{
    Trace trace("BluetoothCapturerSourceInner::Stop");
    std::promise<void> promiseEnsueThreadLock;
    auto futureWaitThreadLock = promiseEnsueThreadLock.get_future();
    std::thread threadAsyncStop([&promiseEnsueThreadLock, this] {
        std::lock_guard<std::mutex> statusLock(statusMutex_);
        promiseEnsueThreadLock.set_value();
        DoStop();
    });
    futureWaitThreadLock.get();
    threadAsyncStop.detach();

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::Pause(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "Pause invalid State");
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());

    Trace trace("BluetoothCapturerSourceInner::Pause");
    if (started_ && audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->control.Pause(reinterpret_cast<AudioHandle>(audioCapture_));
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "pause capture Failed");
    }
    paused_ = true;

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::Resume(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "Resume invalid State");
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());
    Trace trace("BluetoothCapturerSourceInner::Resume");
    if (started_ && audioCapture_ != nullptr) {
        int32_t ret = audioCapture_->control.Resume(reinterpret_cast<AudioHandle>(audioCapture_));
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "resume capture Failed");
    }
    paused_ = false;

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::Reset(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());
    Trace trace("BluetoothCapturerSourceInner::Reset");
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->control.Flush(reinterpret_cast<AudioHandle>(audioCapture_));
    }
    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::Flush(void)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    AUDIO_INFO_LOG("sourceName %{public}s", halName_.c_str());
    Trace trace("BluetoothCapturerSourceInner::Flush");
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->control.Flush(reinterpret_cast<AudioHandle>(audioCapture_));
    }
    return SUCCESS;
}

void BluetoothCapturerSourceInner::RegisterWakeupCloseCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("RegisterWakeupCloseCallback not supported");
}

void BluetoothCapturerSourceInner::RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback)
{
    AUDIO_INFO_LOG("Register AudioCaptureSource Callback");
    audioCapturerSourceCallback_ = std::move(callback);
}

void BluetoothCapturerSourceInner::RegisterParameterCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("RegisterParameterCallback not supported");
}

std::string BluetoothCapturerSourceInner::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_WARNING_LOG("GetAudioParameter not supported");
    return "";
}

void BluetoothCapturerSourceInner::InitLatencyMeasurement()
{
    std::lock_guard<std::mutex> lock(signalDetectAgentMutex_);

    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }

    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    latencyMeasEnabled_ = true;
}

void BluetoothCapturerSourceInner::DeinitLatencyMeasurement()
{
    std::lock_guard<std::mutex> lock(signalDetectAgentMutex_);

    signalDetected_ = false;
    signalDetectAgent_ = nullptr;
}

void BluetoothCapturerSourceInner::CheckLatencySignal(uint8_t *frame, size_t replyBytes)
{
    std::lock_guard<std::mutex> lock(signalDetectAgentMutex_);
    if (!latencyMeasEnabled_) {
        return;
    }

    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetected_ = signalDetectAgent_->CheckAudioData(frame, replyBytes);
    if (signalDetected_) {
        char value[GET_EXTRA_PARAM_LEN];
        AudioParamKey key = NONE;
        AudioExtParamKey hdiKey = AudioExtParamKey(key);
        std::string condition = "debug_audio_latency_measurement";
        int32_t ret = audioAdapter_->GetExtraParams(audioAdapter_, hdiKey, condition.c_str(), value,
            DumpFileUtil::PARAM_VALUE_LENTH);
        AUDIO_INFO_LOG("GetExtraParam ret:%{public}d", ret);
        LatencyMonitor::GetInstance().UpdateDspTime(value);
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(false,
            signalDetectAgent_->lastPeakBufferTime_);
        AUDIO_INFO_LOG("LatencyMeas primarySource signal detected");
        signalDetected_ = false;
    }
}

int32_t BluetoothCapturerSourceInner::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    if (!runningLockManager_) {
        return ERROR;
    }

    runningLockManager_->UpdateAppsUid(appsUid, appsUid + size);
    runningLockManager_->UpdateAppsUidToPowerMgr();
#endif

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    if (!runningLockManager_) {
        return ERROR;
    }

    runningLockManager_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLockManager_->UpdateAppsUidToPowerMgr();
#endif

    return SUCCESS;
}

int32_t BluetoothCapturerSourceInner::GetCaptureId(uint32_t &captureId) const
{
    captureId = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_BLUETOOTH);
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
