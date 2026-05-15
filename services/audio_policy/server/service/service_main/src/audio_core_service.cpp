/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioCoreService"
#endif

#include "audio_core_service.h"
#include "audio_device_factory.h"
#include "system_ability.h"
#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "audio_definition_adapter_info.h"
#include "iservice_registry.h"
#include "hdi_adapter_info.h"
#include "audio_usb_manager.h"
#include "audio_spatialization_service.h"
#include "audio_zone_service.h"
#include "audio_bundle_manager.h"
#include "parameters.h"

#include <algorithm>
#include "hisysevent.h"
#include "media_monitor_manager.h"
#include "audio_volume.h"
#include "i_hpae_manager.h"
#include "audio_select_interface_service.h"
#include "audio_router_select_strategy.h"
#include "audio_router_infra.h"
#include "audio_route_select_info.h"
#include "sco_audio_scene_manager.h"
#include "audio_interrupt_custom.h"
#include "stream_dfx_manager.h"
#include "ipc_skeleton.h"

namespace {
    #include "v6_0/iaudio_manager.h"
}

namespace OHOS {
namespace AudioStandard {
namespace {
const size_t SELECT_DEVICE_HISTORY_LIMIT = 10;
const uint32_t FIRST_SESSIONID = 100000;
static const char *CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
const float RENDER_FRAME_INTERVAL_IN_SECONDS = 0.02;
const uint32_t PC_MIC_CHANNEL_NUM = 4;
const uint32_t HEADPHONE_CHANNEL_NUM = 2;
constexpr const char *MULTI_STREAM_DEVICE_SUPPORT = "const.multimedia.audio.sys_multidevice_capability.enable";
static const bool IS_DEVICE_ENHANCED_SUPPORTED = OHOS::system::GetBoolParameter(MULTI_STREAM_DEVICE_SUPPORT, false);

static const unsigned int BUFFER_CALC_20MS = 20;
static const std::string CHECK_VIDEO_COMM_SELECTION = "audio_video_comm_fast_blocklist";
static const int32_t REFETCH_DEVICE = 4;
static const int64_t RING_DUAL_END_DELAY_US = 100000;
static constexpr int32_t MAX_TRY = 100;
static constexpr int32_t DELAY_MS = 100;
static constexpr int32_t MS_PER_S = 1000;
static constexpr uint64_t ROUNDING_HALF_DIVISOR = 2ULL;
static const std::string PIPE_PRIMARY_INPUT_VOICE_RECOGNITION = "primary_input_voice_recognition";
static constexpr uint32_t ONLY_SUPPORT_MMAP = 2;
static const std::string PIPE_PRIMARY_INPUT_CAMCORDER = "primary_input_camcorder";
static const std::unordered_set<SourceType> VOIP_PRIVACY_MUTE_TARGET_SOURCES = {
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_WAKEUP,
};

static const std::set<AudioSamplingRate> DIRECT_SUPPORTED_SAMPLE_RATES = {
    SAMPLE_RATE_48000,
    SAMPLE_RATE_88200,
    SAMPLE_RATE_96000,
    SAMPLE_RATE_176400,
    SAMPLE_RATE_192000
};

static const std::set<AudioSampleFormat> DIRECT_SUPPORTED_FORMATS = {
    SAMPLE_S24LE,
    SAMPLE_S32LE
};

static const std::vector<DeviceType> MIC_REF_DEVICES = {
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK_IN,
};

static std::map<std::string, AudioSampleFormat> formatStrToEnum = {
    {"s8", SAMPLE_U8},
    {"s16", SAMPLE_S16LE},
    {"s24", SAMPLE_S24LE},
    {"s32", SAMPLE_S32LE},
    {"s16le", SAMPLE_S16LE},
    {"s24le", SAMPLE_S24LE},
    {"s32le", SAMPLE_S32LE},
};

static std::map<AudioSampleFormat, std::string> formatEnumToStr = {
    {SAMPLE_U8, "s16le"},
    {SAMPLE_S16LE, "s16le"},
    {SAMPLE_S24LE, "s24le"},
    {SAMPLE_S32LE, "s32le"},
    {SAMPLE_F32LE, "s16le"},
};

static std::map<AudioSampleFormat, std::string> formatToHpaeStr = {
    {SAMPLE_U8, "s8"},
    {SAMPLE_S16LE, "s16"},
    {SAMPLE_S24LE, "s24"},
    {SAMPLE_S32LE, "s32"},
    {SAMPLE_F32LE, "f32"},
};

static const std::map<std::pair<DeviceType, DeviceType>, EcType> DEVICE_TO_EC_TYPE = {
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_NEARLINK}, EC_TYPE_SAME_ADAPTER},

    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_NEARLINK}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_NEARLINK}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_SPEAKER}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_NEARLINK}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_NEARLINK}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_NEARLINK}, EC_TYPE_SAME_ADAPTER},

    {{DEVICE_TYPE_ACCESSORY, DEVICE_TYPE_SPEAKER}, EC_TYPE_DIFF_ADAPTER},
};

const std::map<SourceType, int> NORMAL_SOURCETYPE_PRIORITY = {
    // from high to low
    {SOURCE_TYPE_VOICE_CALL, 8},
    {SOURCE_TYPE_VOICE_COMMUNICATION, 7},
    {SOURCE_TYPE_VOICE_MESSAGE, 6},
    {SOURCE_TYPE_LIVE, 5},
    {SOURCE_TYPE_VOICE_RECOGNITION, 4},
    {SOURCE_TYPE_VOICE_TRANSCRIPTION, 4},
    {SOURCE_TYPE_MIC, 3},
    {SOURCE_TYPE_CAMCORDER, 3},
    {SOURCE_TYPE_UNPROCESSED, 2},
    {SOURCE_TYPE_ULTRASONIC, 1},
    {SOURCE_TYPE_INVALID, 0},
};

const std::map<SourceType, AudioInputType> FWKTYPE_TO_HDITYPE_MAP = {
    { SOURCE_TYPE_INVALID, AUDIO_INPUT_DEFAULT_TYPE },
    { SOURCE_TYPE_MIC, AUDIO_INPUT_MIC_TYPE },
    { SOURCE_TYPE_PLAYBACK_CAPTURE, AUDIO_INPUT_MIC_TYPE },
    { SOURCE_TYPE_ULTRASONIC, AUDIO_INPUT_ULTRASONIC_TYPE },
    { SOURCE_TYPE_WAKEUP, AUDIO_INPUT_SPEECH_WAKEUP_TYPE },
    { SOURCE_TYPE_VOICE_TRANSCRIPTION, AUDIO_INPUT_VOICE_COMMUNICATION_TYPE },
    { SOURCE_TYPE_VOICE_COMMUNICATION, AUDIO_INPUT_VOICE_COMMUNICATION_TYPE },
    { SOURCE_TYPE_VOICE_RECOGNITION, AUDIO_INPUT_VOICE_RECOGNITION_TYPE },
    { SOURCE_TYPE_VOICE_CALL, AUDIO_INPUT_VOICE_CALL_TYPE },
    { SOURCE_TYPE_CAMCORDER, AUDIO_INPUT_CAMCORDER_TYPE },
    { SOURCE_TYPE_EC, AUDIO_INPUT_EC_TYPE },
    { SOURCE_TYPE_MIC_REF, AUDIO_INPUT_NOISE_REDUCTION_TYPE },
    { SOURCE_TYPE_UNPROCESSED, AUDIO_INPUT_RAW_TYPE },
    { SOURCE_TYPE_LIVE, AUDIO_INPUT_LIVE_TYPE },
    { SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT, AUDIO_INPUT_RAW_AI_TYPE},
};

static const std::unordered_set<SourceType> specialSourceTypeSet_ = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_REMOTE_CAST
};

static uint32_t CalculateTargetBufferDurationMs(const PipeStreamPropInfo &targetInfo)
{
    uint32_t targetBufferMs = BUFFER_CALC_20MS;
    const uint32_t bytesPerSample = AudioPolicyUtils::GetInstance().PcmFormatToBytes(targetInfo.format_);
    if (bytesPerSample == 0 || targetInfo.sampleRate_ == 0 || targetInfo.channels_ == 0 ||
        targetInfo.bufferSize_ == 0) {
        return targetBufferMs;
    }

    const uint64_t denom = static_cast<uint64_t>(targetInfo.sampleRate_) *
                           static_cast<uint64_t>(targetInfo.channels_) *
                           static_cast<uint64_t>(bytesPerSample);
    const uint64_t number = static_cast<uint64_t>(targetInfo.bufferSize_) *
                            static_cast<uint64_t>(MS_PER_S);
    targetBufferMs = static_cast<uint32_t>((number + denom / ROUNDING_HALF_DIVISOR) / denom);
    return targetBufferMs;
}

static bool CalculateArmBufferDurationMs(const AudioModuleInfo &moduleInfo, uint32_t &buffCalcMs)
{
    if (moduleInfo.rate.empty() || moduleInfo.format.empty() || moduleInfo.channels.empty() ||
        moduleInfo.bufferSize.empty()) {
        return true;
    }

    uint32_t rateValue = 0;
    uint32_t channelValue = 0;
    uint32_t bufferSizeValue = 0;
    CHECK_AND_RETURN_RET_LOG(StringConverter(moduleInfo.rate, rateValue), false,
        "convert invalid moduleInfo.rate: %{public}s", moduleInfo.rate.c_str());
    CHECK_AND_RETURN_RET_LOG(StringConverter(moduleInfo.channels, channelValue), false,
        "convert invalid moduleInfo.channels: %{public}s", moduleInfo.channels.c_str());
    CHECK_AND_RETURN_RET_LOG(StringConverter(moduleInfo.bufferSize, bufferSizeValue), false,
        "convert invalid original bufferSize: %{public}s", moduleInfo.bufferSize.c_str());

    const uint32_t bytesPerSample =
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(formatStrToEnum[moduleInfo.format]);
    CHECK_AND_RETURN_RET_LOG(bytesPerSample > 0, false, "bytesPerSample is 0");

    const uint64_t denom = static_cast<uint64_t>(rateValue) *
                           static_cast<uint64_t>(channelValue) *
                           static_cast<uint64_t>(bytesPerSample);
    CHECK_AND_RETURN_RET_LOG(denom > 0, false, "denom is 0");
    const uint64_t number = static_cast<uint64_t>(bufferSizeValue) *
                            static_cast<uint64_t>(MS_PER_S);
    buffCalcMs = static_cast<uint32_t>((number + denom / ROUNDING_HALF_DIVISOR) / denom);
    return true;
}

}

static bool IsRemoteOffloadActive(uint32_t remoteOffloadStreamPropSize, int32_t streamUsage, int32_t streamUid)
{
    CHECK_AND_RETURN_RET_LOG(remoteOffloadStreamPropSize != 0 && streamUsage == STREAM_USAGE_MUSIC &&
        streamUid != AUDIO_EXT_UID, false, "Use normal for remote device or remotecast");
    AUDIO_INFO_LOG("remote offload active, music use offload");
    return true;
}

static bool IsVoiceRecognitionMicInEcSupportedInputDeviceType(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_MIC:
        case DEVICE_TYPE_WAKEUP:
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            return true;
        default:
            return false;
    }
}

static bool IsVoiceRecognitionMicInEcSupportedInputDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return deviceDesc != nullptr && deviceDesc->networkId_ == LOCAL_NETWORK_ID &&
        IsVoiceRecognitionMicInEcSupportedInputDeviceType(deviceDesc->deviceType_);
}

static bool HasDegradeInputDevice(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    return std::any_of(deviceDescs.begin(), deviceDescs.end(), [](const auto &deviceDesc) {
        return deviceDesc != nullptr && !IsVoiceRecognitionMicInEcSupportedInputDevice(deviceDesc);
    });
}

static bool IsDegradeInputStream(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    return HasDegradeInputDevice(streamDesc->newDeviceDescs_) || HasDegradeInputDevice(streamDesc->oldDeviceDescs_);
}

static bool IsVoiceRecognitionMicInEcRequested(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    return streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION &&
        !IsDegradeInputStream(streamDesc) &&
        (streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW || streamDesc->ecStreamInfo_.channels > CHANNEL_UNKNOW);
}

static bool IsCamcorderMicInRequested(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    return streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_CAMCORDER &&
        streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW;
}

static bool IsVoiceRecognitionIndependentRouteSupported()
{
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    CHECK_AND_RETURN_RET(sourceStrategyMap != nullptr, false);
    auto strategy = sourceStrategyMap->find(SOURCE_TYPE_VOICE_RECOGNITION);
    CHECK_AND_RETURN_RET(strategy != sourceStrategyMap->end(), false);
    return strategy->second.pipeName == PIPE_PRIMARY_INPUT_VOICE_RECOGNITION;
}

static bool IsCamcorderIndependentRouteSupported()
{
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    CHECK_AND_RETURN_RET(sourceStrategyMap != nullptr, false);
    auto strategy = sourceStrategyMap->find(SOURCE_TYPE_CAMCORDER);
    CHECK_AND_RETURN_RET(strategy != sourceStrategyMap->end(), false);
    return strategy->second.pipeName == PIPE_PRIMARY_INPUT_CAMCORDER;
}

static void UpdateMicInEcModuleInfoByStreamDesc(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    AudioModuleInfo &moduleInfo)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    if (streamDesc->ecStreamInfo_.channels > CHANNEL_UNKNOW) {
        moduleInfo.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
        moduleInfo.ecSamplingRate = std::to_string(streamDesc->ecStreamInfo_.samplingRate);
        moduleInfo.ecChannels = std::to_string(streamDesc->ecStreamInfo_.channels);
        if (formatToHpaeStr.count(streamDesc->ecStreamInfo_.format) > 0) {
            moduleInfo.ecFormat = formatToHpaeStr[streamDesc->ecStreamInfo_.format];
        }
    }

    if (streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW) {
        moduleInfo.micInRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micInChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatToHpaeStr.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micInFormat = formatToHpaeStr[streamDesc->micInStreamInfo_.format];
        }
        moduleInfo.micRefRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micRefChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatToHpaeStr.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micRefFormat = formatToHpaeStr[streamDesc->micInStreamInfo_.format];
        }
    }
}

static void UpdateCamcorderMicInModuleInfo(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    AudioModuleInfo &moduleInfo)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    if (streamDesc->micInStreamInfo_.channels > CHANNEL_UNKNOW) {
        moduleInfo.micInRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micInChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatToHpaeStr.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micInFormat = formatToHpaeStr[streamDesc->micInStreamInfo_.format];
        }
        moduleInfo.micRefRate = std::to_string(streamDesc->micInStreamInfo_.samplingRate);
        moduleInfo.micRefChannels = std::to_string(streamDesc->micInStreamInfo_.channels);
        if (formatToHpaeStr.count(streamDesc->micInStreamInfo_.format) > 0) {
            moduleInfo.micRefFormat = formatToHpaeStr[streamDesc->micInStreamInfo_.format];
        }
    }
}

bool AudioCoreService::isBtListenerRegistered = false;
bool AudioCoreService::isBtCrashed = false;
#ifdef BLUETOOTH_ENABLE
mutex g_btProxyMutex;
#endif

AudioCoreService::AudioCoreService()
    : audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
      audioActiveDevice_(AudioActiveDevice::GetInstance()),
      audioSceneManager_(AudioSceneManager::GetInstance()),
      audioVolumeManager_(AudioVolumeManager::GetInstance()),
      audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
      audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
      audioDeviceStatus_(AudioDeviceStatus::GetInstance()),
      audioEffectService_(AudioEffectService::GetAudioEffectService()),
      audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
      audioRecoveryDevice_(AudioRecoveryDevice::GetInstance()),
      audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
      streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
      audioStateManager_(AudioStateManager::GetAudioStateManager()),
      audioDeviceCommon_(AudioDeviceCommon::GetInstance()),
      audioOffloadStream_(AudioOffloadStream::GetInstance()),
      audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
      audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
      audioRouteMap_(AudioRouteMap::GetInstance()),
      audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
      audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
      sleAudioDeviceManager_(SleAudioDeviceManager::GetInstance()),
      audioPipeSelector_(AudioPipeSelector::GetPipeSelector()),
      audioSessionService_(OHOS::Singleton<AudioSessionService>::GetInstance()),
      audioRouterSelectStrategy_(AudioRouterSelectStrategy::GetInstance()),
      pipeManager_(AudioPipeManager::GetPipeManager()),
      audioInjectorPolicy_(AudioInjectorPolicy::GetInstance()),
      audioPolicyDump_(AudioPolicyDump::GetInstance())
{
    AUDIO_INFO_LOG("Ctor");
}

AudioCoreService::~AudioCoreService()
{
    AUDIO_INFO_LOG("Dtor");
}

std::shared_ptr<AudioCoreService> AudioCoreService::GetCoreService()
{
    static std::shared_ptr<AudioCoreService> instance = std::make_shared<AudioCoreService>();
    return instance;
}

void AudioCoreService::Init()
{
    serviceFlag_.reset();
    eventEntry_ = std::make_shared<EventEntry>(shared_from_this());
    deviceStatusListener_ = std::make_shared<DeviceStatusListener>(*eventEntry_); // shared_ptr.get() -> *

    audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    if (audioA2dpOffloadManager_ != nullptr) {
        audioA2dpOffloadManager_->Init();
    }
    audioDeviceCommon_.Init(audioPolicyServerHandler_);
    audioRecoveryDevice_.Init(audioA2dpOffloadManager_);
    AudioSelectInterfaceService::GetInstance().SetA2dpDeviceOffloadManager(audioA2dpOffloadManager_);

    audioDeviceStatus_.Init(audioA2dpOffloadManager_, audioPolicyServerHandler_);
    pipeManager_->Init();
    SetConfigParserFlag();

    // Register device status listener
    int32_t status = deviceStatusListener_->RegisterDeviceStatusListener();
    RecordBootStateTime(AudioPolicyServerBootState::LISTENING_DEVICE_STATUS, ClockTime::GetCurMilli());
    if (status != SUCCESS) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Register for device status events failed");
        AUDIO_ERR_LOG("Register for device status events failed");
    }
    isSupportUltraFast_ = pipeManager_->GetUltraFastFlag();
}

void AudioCoreService::DeInit()
{
    // Remove device status listener
    deviceStatusListener_->UnRegisterDeviceStatusListener();
    if (isBtListenerRegistered) {
        UnregisterBluetoothListener();
    }
}

void AudioCoreService::RecordBootStateTime(AudioPolicyServerBootState state, int64_t curTime)
{
    audioPolicyDump_.RecordBootStateTime(state, curTime);
}

void AudioCoreService::RecordBootStateTime(BootAnimationState state, int32_t uid, int64_t curTime)
{
    audioPolicyDump_.RecordBootStateTime(state, uid, curTime);
}

void AudioCoreService::SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler)
{
    audioPolicyServerHandler_ = handler;
}

std::shared_ptr<AudioCoreService::EventEntry> AudioCoreService::GetEventEntry()
{
    return eventEntry_;
}

void AudioCoreService::SetAsyncActionHandler(std::shared_ptr<AsyncActionHandler> &handler)
{
    asyncHandler_ = handler;
}

std::shared_ptr<AsyncActionHandler> AudioCoreService::GetAsyncActionHandler() const
{
    return asyncHandler_;
}

void AudioCoreService::DumpPipeManager(std::string &dumpString)
{
    if (pipeManager_ != nullptr) {
        pipeManager_->Dump(dumpString);
    }

    audioOffloadStream_.Dump(dumpString);
}

void AudioCoreService::SetConfigParserFlag()
{
    isPolicyConfigParsered_ = true;
}

void AudioCoreService::SetEcEnableState(bool ecEnableState)
{
    pipeManager_->SetEcEnableState(ecEnableState);
}

AudioSampleFormat AudioCoreService::GetFastFormat() const
{
    return pipeManager_->GetFastFormat();
}

bool AudioCoreService::IsSupportInnerCaptureOffload()
{
    return pipeManager_->IsSupportInnerCaptureOffload();
}

bool AudioCoreService::GetEnhancedRoutingSupported()
{
    return AudioSelectInterfaceService::GetInstance().GetEnhancedRoutingSupported();
}

int32_t AudioCoreService::GetMaxRendererInstances()
{
    return pipeManager_->GetMaxRendererInstances();
}

void AudioCoreService::GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo)
{
    pipeManager_->GetDeviceClassInfo(deviceClassInfo);
}

void AudioCoreService::FetchOutputDupDevice(std::string caller, uint32_t sessionId,

    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    FetchDeviceInfo info = {};
    info.streamUsage = streamDesc->rendererInfo_.streamUsage;
    info.clientUID = GetRealUid(streamDesc);
    info.caller = caller;
    info.privacyType = streamDesc->rendererInfo_.privacyType;

    streamDesc->oldDupDeviceDescs_ = streamDesc->newDupDeviceDescs_;
    streamDesc->newDupDeviceDescs_ =
        audioRouterCenter_.FetchDupDevices(info);
    AUDIO_INFO_LOG("[DeviceFetchInfo] device %{public}s, status %{public}u, dupDevice %{public}s, stream %{public}d",
        streamDesc->GetNewDevicesTypeString().c_str(), streamDesc->GetStatus(),
        streamDesc->GetNewDupDevicesTypeString().c_str(), sessionId);

    UpdateDupDeviceOutputRoute(streamDesc);

    if (audioPolicyServerHandler_ != nullptr && IsDupDeviceChange(streamDesc)) {
        audioPolicyServerHandler_->SendPreferredOutputDeviceUpdated();
    }
}

void AudioCoreService::SetEcAndMicRefEnableState(int32_t ecEnableState, int32_t micRefEnableState)
{
    isEcFeatureEnable_ = ecEnableState != 0;
    isMicRefFeatureEnable_ = micRefEnableState != 0;
}

AudioEcInfo AudioCoreService::GetAudioEcInfo()
{
    std::lock_guard<std::mutex> lock(audioEcInfoMutex_);
    return audioEcInfo_;
}

void AudioCoreService::ResetAudioEcInfo()
{
    std::lock_guard<std::mutex> lock(audioEcInfoMutex_);
    audioEcInfo_.inputDevice.deviceType_ = DEVICE_TYPE_NONE;
    audioEcInfo_.outputDevice.deviceType_ = DEVICE_TYPE_NONE;
}

void AudioCoreService::SetDpSinkModuleInfo(const AudioModuleInfo &moduleInfo)
{
    dpSinkModuleInfo_ = moduleInfo;
}

void AudioCoreService::SetPrimaryMicModuleInfo(const AudioModuleInfo &moduleInfo)
{
    primaryMicModuleInfo_ = moduleInfo;
}

SourceType AudioCoreService::GetSourceOpened()
{
    return normalSourceOpened_;
}

bool AudioCoreService::GetEcFeatureEnable()
{
    return isEcFeatureEnable_;
}

bool AudioCoreService::GetMicRefFeatureEnable()
{
    return isMicRefFeatureEnable_;
}

uint64_t AudioCoreService::GetOpenedNormalSourceSessionId()
{
    return sessionIdUsedToOpenSource_;
}

void AudioCoreService::SetUsbSinkModuleInfo(AudioModuleInfo &moduleInfo)
{
    usbSinkModuleInfo_ = moduleInfo;
}

void AudioCoreService::SetUsbSourceModuleInfo(AudioModuleInfo &moduleInfo)
{
    usbSourceModuleInfo_ = moduleInfo;
}

void AudioCoreService::SetOpenedNormalSourceSessionId(uint64_t sessionId)
{
    AUDIO_INFO_LOG("set normal source sessionId: %{public}" PRIu64, sessionId);
    sessionIdUsedToOpenSource_ = sessionId;
}

int32_t AudioCoreService::CheckStreamConflicts(const std::shared_ptr<AudioStreamDescriptor>& streamDesc,
    uint32_t& audioFlag)
{
    // Check interphone stream conflict with VoIP/Cellular stream
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_INTERPHONE) {
        if (pipeManager_->IsVoipOrCellularStreamActive()) {
            AUDIO_ERR_LOG("Cannot create interphone renderer: VoIP or Cellular stream is active");
            return ERR_ILLEGAL_STATE;
        }
        audioFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_INTERPHONE;
        streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_INTERPHONE;
    }
    // Check VoIP/Cellular stream conflict with interphone stream
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        if (pipeManager_->IsInterphoneStreamActive() || pipeManager_->IsInterphoneCapturerActive()) {
            AUDIO_ERR_LOG("Cannot create VoIP/Cellular renderer: Interphone stream is active");
        }
    }
    return SUCCESS;
}

int32_t AudioCoreService::CreateRendererClient(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    uint32_t &audioFlag, uint32_t &sessionId, std::string &networkId)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "stream desc is nullptr");
    if (sessionId == 0) {
        streamDesc->sessionId_ = GenerateSessionId();
        sessionId = streamDesc->sessionId_;
        AUDIO_INFO_LOG("Generate session id %{public}u for stream", sessionId);
    }

    ClientTypeManager::GetInstance()->GetAndSaveClientType(GetRealUid(streamDesc),
        AudioBundleManager::GetBundleNameFromUid(GetRealUid(streamDesc)));

    UpdateStreamDevicesForCreate(streamDesc, "CreateRendererClient");
    // Modem stream need special process, because there are no real hdi output or input in fwk.
    // Input also need to be handled because capturer won't be created, only has renderer.
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION &&
        !streamDesc->rendererInfo_.toneFlag) {
        AUDIO_INFO_LOG("Modem communication renderer create, sessionId %{public}u", sessionId);
        audioFlag = AUDIO_FLAG_NORMAL;
        AddSessionId(sessionId);
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION;
        streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_MODEM_COMMUNICATION;
        pipeManager_->AddModemCommunicationId(sessionId, streamDesc);
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(streamDesc->newDeviceDescs_.size() > 0
        && streamDesc->newDeviceDescs_.front() != nullptr, ERR_NULL_POINTER, "Invalid deviceDesc");

    // Check interphone stream conflict with VoIP/Cellular stream
    int32_t ret = CheckStreamConflicts(streamDesc, audioFlag);
    if (ret != SUCCESS) {
        return ret;
    }

    // Bluetooth may be inactive (paused ringtone stream at Speaker switches to A2dp)
    if (streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        auto ret = ActivateOutputDevice(streamDesc, AudioStreamDeviceChangeReason::UNKNOWN);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ERR_OPERATION_FAILED);
    }

    // Fetch pipe
    audioActiveDevice_.UpdateStreamDeviceMap("CreateRendererClient");
    ret = FetchRendererPipeAndExecute(streamDesc, sessionId, audioFlag);
    networkId = streamDesc->newDeviceDescs_.front()->networkId_;
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchPipeAndExecute failed");
    AddSessionId(sessionId);

    pipeManager_->RemoveSwitchStream(sessionId);
    streamDesc->SetFutureRoute(AUDIO_FLAG_NONE);

    return SUCCESS;
}

int32_t AudioCoreService::CheckCapturerStreamConflicts(const std::shared_ptr<AudioStreamDescriptor>& streamDesc)
{
    // Check interphone capturer conflict with VoIP/Cellular streams
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_INTERPHONE) {
        if (pipeManager_->IsVoipOrCellularStreamActive()) {
            AUDIO_ERR_LOG("Cannot create interphone capturer: VoIP or Cellular stream is active");
            return ERR_ILLEGAL_STATE;
        }
    }
    // Check VoIP/Cellular capturer conflict with interphone streams
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION ||
        streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_CALL) {
        if (pipeManager_->IsInterphoneStreamActive() || pipeManager_->IsInterphoneCapturerActive()) {
            AUDIO_ERR_LOG("Cannot create VoIP/Cellular capturer: Interphone stream is active");
            return ERR_ILLEGAL_STATE;
        }
    }
    return SUCCESS;
}

int32_t AudioCoreService::CreateCapturerClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_INVALID_PARAM, "streamDesc is nullptr");

    // Check interphone capturer conflict with VoIP/Cellular streams
    int32_t ret = CheckCapturerStreamConflicts(streamDesc);
    if (ret != SUCCESS) {
        return ret;
    }

    bool hasInputDeviceSnapshot = !streamDesc->newDeviceDescs_.empty() || !streamDesc->oldDeviceDescs_.empty();
    if (hasInputDeviceSnapshot && IsVoiceRecognitionMicInEcRequested(streamDesc)) {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
            "micin/ec capture denied: no system permission");
        CHECK_AND_RETURN_RET_LOG(IsVoiceRecognitionIndependentRouteSupported(), ERR_NOT_SUPPORTED,
            "micin/ec capture denied: voice recognition independent route not supported");
    }
    if (IsCamcorderMicInRequested(streamDesc)) {
        CHECK_AND_RETURN_RET_LOG(IsCamcorderIndependentRouteSupported(), ERR_NOT_SUPPORTED,
            "micin capture denied: camcorder independent route not supported");
    }
    if (sessionId == 0) {
        streamDesc->sessionId_ = GenerateSessionId();
        sessionId = streamDesc->sessionId_;
        AUDIO_INFO_LOG("Generate sessionId: %{public}u for stream", sessionId);
    }
    AUDIO_INFO_LOG("[DeviceFetchStart] for stream %{public}d", sessionId);

    SetPreferredInputDeviceIfValid(streamDesc);
    streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
    std::shared_ptr<AudioDeviceDescriptor> inputDeviceDesc = GetCaptureClientDevice(streamDesc, sessionId);
    CHECK_AND_RETURN_RET_LOG(inputDeviceDesc != nullptr, ERR_INVALID_PARAM, "inputDeviceDesc is nullptr");
    pipeManager_->UpdateNewDeviceDesc(streamDesc, {inputDeviceDesc});
    HILOG_COMM_INFO("[DeviceFetchInfo] device %{public}s for stream %{public}d",
        streamDesc->GetNewDevicesTypeString().c_str(), sessionId);
    if (!hasInputDeviceSnapshot && IsVoiceRecognitionMicInEcRequested(streamDesc)) {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(), ERR_PERMISSION_DENIED,
            "micin/ec capture denied: no system permission");
        CHECK_AND_RETURN_RET_LOG(IsVoiceRecognitionIndependentRouteSupported(), ERR_NOT_SUPPORTED,
            "micin/ec capture denied: voice recognition independent route not supported");
    }

    UpdateRecordStreamInfo(streamDesc);
    AUDIO_INFO_LOG("Target audioFlag 0x%{public}x for stream %{public}d",
        streamDesc->audioFlag_, sessionId);

    // Fetch pipe
    ret = FetchCapturerPipeAndExecute(streamDesc, audioFlag, sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchPipeAndExecute failed");
    AddSessionId(sessionId);
    pipeManager_->RemoveSwitchStream(sessionId);
    streamDesc->SetFutureRoute(AUDIO_FLAG_NONE);
    return SUCCESS;
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::GetCaptureClientDevice(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t sessionId)
{
    bool hasRunningStream = streamCollector_.HasRunningCapturerStreamByUid(INVALID_UID);
    CHECK_AND_RETURN_RET(!audioRouterCenter_.IsConfigRouterStrategy(streamDesc->capturerInfo_.sourceType) ||
        !hasRunningStream, std::make_shared<AudioDeviceDescriptor>(
            audioRouterSelectStrategy_.Get1stCurrentInputDevice(GetRealUid(streamDesc))));

    RouterType routerType = ROUTER_TYPE_NONE;
    return audioRouterCenter_.FetchInputDevice(streamDesc->capturerInfo_.sourceType,
        GetRealUid(streamDesc), routerType, sessionId);
}

void AudioCoreService::SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    RecordSelectDevice(ParsePreferredInputDeviceHistory(streamDesc));
    CHECK_AND_RETURN_LOG(PermissionUtil::VerifySystemPermission(),
        "set preferred input device denied: no system permission");
    AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(streamDesc);
}

std::string AudioCoreService::ParsePreferredInputDeviceHistory(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, "", "streamDesc is nullptr");
    std::string preferredHistoryItem =
        GetTime() + "|Uid:" + std::to_string(IPCSkeleton::GetCallingUid()) +
        " Pid: " + std::to_string(IPCSkeleton::GetCallingPid()) +
        " sessionId: " + std::to_string(streamDesc->sessionId_) +
        " preferred input device type: " + streamDesc->preferredInputDevice.GetDeviceTypeString() +
        " stream type: " + std::to_string(streamDesc->capturerInfo_.sourceType);
    return preferredHistoryItem;
}

bool AudioCoreService::IsStreamSupportMultiChannel(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportMultiChannel");

    if (streamDesc->streamInfo_.encoding == ENCODING_AUDIOVIVID &&
        pipeManager_->PreferMultiChannelPipe(streamDesc)) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "AudioVivid encoding and MultiChannelPipe supported");
        return true;
    }

    // MultiChannel: Speaker, A2dp offload
    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_SPEAKER &&
        (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP ||
        streamDesc->newDeviceDescs_[0]->a2dpOffloadFlag_ != A2DP_OFFLOAD) &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_NEARLINK) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "normal stream, deviceType: %{public}d",
            streamDesc->newDeviceDescs_[0]->deviceType_);
        return false;
    }
    if (streamDesc->streamInfo_.channels <= STEREO ||
        (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_MOVIE &&
         streamDesc->rendererInfo_.originalFlag == AUDIO_FLAG_PCM_OFFLOAD)) {
        return false;
    }
    // The multi-channel algorithm needs to be supported in the dsp
    bool isSupported = AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
    JUDGE_AND_INFO_LOG(isCreateProcess_, "effect offload enable is %{public}d", isSupported);
    return isSupported;
}

bool AudioCoreService::IsStreamSupportDirect(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportDirect");
    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_WIRED_HEADSET &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_USB_HEADSET &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_NEARLINK) {
        return false;
    }
    if (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_MUSIC) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "stream usage is not music");
        return false;
    }

    AudioSamplingRate samplingRate = streamDesc->streamInfo_.samplingRate;
    AudioSampleFormat format = streamDesc->streamInfo_.format;

    if (DIRECT_SUPPORTED_SAMPLE_RATES.find(samplingRate) == DIRECT_SUPPORTED_SAMPLE_RATES.end()) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "sample rate not supported for direct: %{public}d", samplingRate);
        return false;
    }

    if (DIRECT_SUPPORTED_FORMATS.find(format) == DIRECT_SUPPORTED_FORMATS.end()) {
        JUDGE_AND_INFO_LOG(isCreateProcess_, "format not supported for direct: %{public}d", format);
        return false;
    }
    auto ret = AudioSpatializationService::GetAudioSpatializationService().IsSpatializationEnabled(
        streamDesc->newDeviceDescs_[0]->macAddress_)  &&
        !(AudioSpatializationService::GetAudioSpatializationService().IsAdaptiveSpatialRenderingEnabled(
            streamDesc->newDeviceDescs_[0]->macAddress_) &&
        streamDesc->streamInfo_.channels <= STEREO &&
        streamDesc->streamInfo_.encoding != ENCODING_AUDIOVIVID);
    CHECK_AND_CALL_FUNC_RETURN_RET(ret == false, false,
        HILOG_COMM_ERROR("[IsStreamSupportDirect]Spatialization enabled"));
    return true;
}

bool AudioCoreService::IsStreamSupportOutputInterPhone(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportOutputInterPhone");
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_INTERPHONE) {
        return true;
    }
    return false;
}

bool AudioCoreService::IsForcedNormal(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    const auto &rendererInfo = streamDesc->rendererInfo_;
    if (rendererInfo.originalFlag == AUDIO_FLAG_FORCED_NORMAL ||
        rendererInfo.rendererFlags == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        return true;
    }

    if (rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION &&
        InVideoCommFastBlockList(streamDesc->bundleName_)) {
        AUDIO_INFO_LOG("bundleName_ is in the blocklist");
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        return true;
    }

    if (streamDesc->newDeviceDescs_.empty()) {
        return false;
    }
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    if (deviceDesc->getType() ==  DEVICE_TYPE_USB_ARM_HEADSET && !deviceDesc->GetDeviceSupportMmap()) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        AUDIO_INFO_LOG("device not support mmap");
        return true;
    }

    return false;
}

bool AudioCoreService::IsHWDecoding(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "invalid streamDesc!");
    AudioStreamInfo streamInfo = streamDesc->streamInfo_;
    RETURN_RET_IF(!IsHWDecodingType(streamInfo.encoding), false);

    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_HWDECODING;
    return true;
}

void AudioCoreService::UpdatePlaybackStreamFlag(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    bool isCreateProcess)
{
    CHECK_AND_RETURN_LOG(streamDesc, "Input param error");
    SelectA2dpType(streamDesc, isCreateProcess);
    if (isCreateProcess && streamDesc->rendererInfo_.forceToNormal) {
        AUDIO_INFO_LOG("force create normal");
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        return;
    }

    CHECK_AND_RETURN_LOG(IsHWDecoding(streamDesc) == false, "HWDecoding streams");

    // fast/normal has done in audioRendererPrivate
    CHECK_AND_RETURN_LOG(IsForcedNormal(streamDesc) == false, "Forced normal");

    if (streamDesc->newDeviceDescs_.back()->deviceType_ == DEVICE_TYPE_REMOTE_CAST ||
        streamDesc->newDeviceDescs_.back()->networkId_ != LOCAL_NETWORK_ID) {
        auto remoteOffloadStreamPropSize = pipeManager_->GetStreamPropInfoSize("remote", "offload_distributed_output");
        streamDesc->audioFlag_ = IsRemoteOffloadActive(remoteOffloadStreamPropSize,
            streamDesc->rendererInfo_.streamUsage, GetRealUid(streamDesc)) ?
            AUDIO_OUTPUT_FLAG_LOWPOWER : AUDIO_OUTPUT_FLAG_NORMAL;
        return;
    }

    CHECK_AND_RETURN_LOG(CheckStaticModeAndSelectFlag(streamDesc) == false, "StaticMode directly return!");

    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
        std::string sinkPortName =
            AudioPolicyUtils::GetInstance().GetSinkPortName(streamDesc->newDeviceDescs_.front()->deviceType_);
        // in plan: if has two voip, return normal
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
        AUDIO_INFO_LOG("sinkPortName %{public}s, audioFlag 0x%{public}x",
            sinkPortName.c_str(), streamDesc->audioFlag_);
        return;
    }
    switch (streamDesc->rendererInfo_.originalFlag) {
        case AUDIO_FLAG_MMAP:
            streamDesc->audioFlag_ = GetFlagForMmapStream(streamDesc);
            return;
        case AUDIO_FLAG_VOIP_FAST:
            streamDesc->audioFlag_ =
                IsFastAllowed(streamDesc->bundleName_) ? AUDIO_OUTPUT_FLAG_VOIP : AUDIO_OUTPUT_FLAG_NORMAL;
            return;
        case AUDIO_FLAG_VOIP_DIRECT:
            streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
            return;
        case AUDIO_FLAG_ULTRA_FAST:
            streamDesc->ResetUltraFastFlag();
            GetFlagForUltraFastStream(streamDesc, isCreateProcess);
            break;
        default:
            break;
    }
    isCreateProcess_ = isCreateProcess;
    streamDesc->audioFlag_ = SetFlagForSpecialStream(streamDesc, isCreateProcess);
    isCreateProcess_ = false;
}

AudioFlag AudioCoreService::GetFlagForMmapStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, AUDIO_OUTPUT_FLAG_NORMAL, "Invalid stream desc");
    DeviceType deviceType = streamDesc->newDeviceDescs_.front()->getType();
    // When a armusb device is not support ultrafast, set flag to fast
    if (deviceType == DEVICE_TYPE_USB_ARM_HEADSET &&
        streamDesc->newDeviceDescs_.front()->GetDeviceSupportMmap() == ONLY_SUPPORT_MMAP) {
        streamDesc->SetUltraFastRequested(false);
        return AUDIO_OUTPUT_FLAG_FAST;
    }
    // For primary hal, fast and ultra fast cannot be used concurrently.
    // When a stream switched to a device that does not support ultra fast, all fast stream should use normal route.
    // During the recreate of fast stream, if current has ultra fast stream and current device does not support
    // ultra fast, set the flag to normal.
    CHECK_AND_RETURN_RET_LOG(!pipeManager_->HasUltraFastStreamRequest() ||
        g_ultraFastDevicesSet.find(deviceType) != g_ultraFastDevicesSet.end() ||
        AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType) != PRIMARY_SPEAKER,
        AUDIO_OUTPUT_FLAG_NORMAL, "Ultra fast stream exists, set %{public}u to normal", streamDesc->GetSessionId());
    if (streamDesc->GetMainNewDeviceType() == DEVICE_TYPE_BLUETOOTH_A2DP ||
        IsFastAllowed(streamDesc->bundleName_)) {
        return AUDIO_OUTPUT_FLAG_FAST;
    }
    return AUDIO_OUTPUT_FLAG_NORMAL;
}

AudioFlag AudioCoreService::SetFlagForSpecialStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    bool isCreateProcess)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, AUDIO_OUTPUT_FLAG_NORMAL, "Invalid stream desc");

    if (IsStreamSupportDirect(streamDesc)) {
        return AUDIO_OUTPUT_FLAG_HD;
    }
    if (IsStreamSupportLowpower(streamDesc)) {
        return AUDIO_OUTPUT_FLAG_LOWPOWER;
    }
    if (IsStreamSupportMultiChannel(streamDesc)) {
        return AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    }
    if (IsStreamSupportOutputInterPhone(streamDesc)) {
        return AUDIO_OUTPUT_FLAG_INTERPHONE;
    }
    return AUDIO_OUTPUT_FLAG_NORMAL;
}

bool AudioCoreService::RecordIsForcedNormal(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (streamDesc->capturerInfo_.originalFlag == AUDIO_FLAG_FORCED_NORMAL ||
        streamDesc->capturerInfo_.capturerFlags == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_NORMAL;
        AUDIO_INFO_LOG("Forced normal cases");
        return true;
    }

    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_REMOTE_CAST) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_NORMAL;
        AUDIO_WARNING_LOG("Use normal for remotecast");
        return true;
    }

    if (streamDesc->newDeviceDescs_.empty()) {
        return false;
    }
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    if (deviceDesc->getType() == DEVICE_TYPE_USB_ARM_HEADSET && !deviceDesc->GetDeviceSupportMmap()) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_NORMAL;
        AUDIO_INFO_LOG("device not support mmap");
        return true;
    }

    return false;
}

void AudioCoreService::UpdateRecordStreamInfo(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    if (sourceStrategyMap != nullptr) {
        auto strategyIt = sourceStrategyMap->find(streamDesc->capturerInfo_.sourceType);
        if (strategyIt != sourceStrategyMap->end() && streamDesc->capturerInfo_.capturerFlags == AUDIO_FLAG_NORMAL) {
            streamDesc->audioFlag_ = strategyIt->second.audioFlag;
            AUDIO_INFO_LOG("sourceType: %{public}d, use audioFlag: %{public}u",
                streamDesc->capturerInfo_.sourceType, strategyIt->second.audioFlag);
            return;
        }
    }

    CHECK_AND_RETURN_LOG(RecordIsForcedNormal(streamDesc) == false, "Forced normal");

    // fast/normal has done in audioCapturerPrivate
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        // in plan: if has two voip, return normal
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_VOIP;
        AUDIO_INFO_LOG("Use voip");
        return;
    }

    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_WAKEUP) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_WAKEUP;
    }
    switch (streamDesc->capturerInfo_.capturerFlags) {
        case AUDIO_FLAG_MMAP:
            streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_FAST;
            return;
        case AUDIO_FLAG_VOIP_FAST:
            streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_VOIP_FAST;
            return;
        default:
            break;
    }

    streamDesc->audioFlag_ = AUDIO_FLAG_NONE;
    return;
}

void AudioCoreService::CheckAndSetCurrentOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t sessionId)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    auto streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_LOG(streamDesc, "streamDesc is null");
    auto uid = GetRealUid(streamDesc);
    CHECK_AND_RETURN_LOG(!IsSameDevice(desc,
        audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid)), "same device");
    OnRemoteDeviceStatusUpdated();
    audioRouterSelectStrategy_.UpdateCurrentOutputDevice(uid, {desc});
    OnPreferredOutputDeviceUpdated(audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid),
        AudioStreamDeviceChangeReason::STREAM_PRIORITY_CHANGED);
}

void AudioCoreService::CheckAndSetCurrentInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    CHECK_AND_RETURN_LOG(!audioRouterSelectStrategy_.IsCurrentInputDevice(desc->deviceId_, uid),
        "current input device is same as new device");
    audioRouterSelectStrategy_.UpdateCurrentInputDevice(uid, {desc});
    OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(uid), "");
}

void AudioCoreService::CheckForRemoteDeviceState(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    std::string networkId = desc->networkId_;
    DeviceRole deviceRole = desc->deviceRole_;
    CHECK_AND_RETURN(networkId != LOCAL_NETWORK_ID);
    int32_t res = AudioServerProxy::GetInstance().CheckRemoteDeviceStateProxy(networkId, deviceRole, true);
    CHECK_AND_RETURN_LOG(res == SUCCESS, "remote device state is invalid!");
}

void AudioCoreService::CheckScoState(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");

    bool isScoDevice = !streamDesc->newDeviceDescs_.empty() &&
        streamDesc->newDeviceDescs_[0] != nullptr &&
        streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO;
    if (isScoDevice && IS_DEVICE_ENHANCED_SUPPORTED) {
        TrackScoStreamAudioScene(streamDesc, true);
        bool hasScoRecord = pipeManager_->HasRunningNormalCapturerStream(DEVICE_TYPE_BLUETOOTH_SCO);
        bool hasScoPlay = pipeManager_->HasRunningScoOutputStream();
        bool isScoSelected = hasScoPlay || hasScoRecord;
        if (isScoSelected) {
            AudioScene highestScene = ScoAudioSceneManager::GetInstance().GetHighestPriorityAudioScene();
            Bluetooth::AudioHfpManager::UpdateAudioScene(highestScene, true);
        }
    } else if (isScoDevice) {
        bool hasScoRecord = pipeManager_->HasRunningNormalCapturerStream(DEVICE_TYPE_BLUETOOTH_SCO);
        bool hasScoPlay = pipeManager_->HasRunningScoOutputStream();
        bool isScoSelected = hasScoPlay || hasScoRecord;
        Bluetooth::AudioHfpManager::UpdateAudioScene(audioSceneManager_.GetAudioScene(true), isScoSelected);
    }

    if (streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        auto sourceType = streamDesc->capturerInfo_.sourceType;
        auto sessionId = streamDesc->sessionId_;
        if (Util::IsScoSupportSource(sourceType)) {
            audioStateManager_.SetPreferredRecognitionCaptureDevice(make_shared<AudioDeviceDescriptor>());
            Bluetooth::AudioHfpManager::HandleScoWithRecongnition(false);
        }
        audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptorBySessionID(sessionId);
    }
}

void AudioCoreService::TrackScoStreamAudioScene(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isAdd)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    
    uint32_t streamId = streamDesc->sessionId_;
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    bool isRecognition = false;
    
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
        if (streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
            audioScene = AUDIO_SCENE_PHONE_CALL;
        } else if (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
                   streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
            audioScene = AUDIO_SCENE_PHONE_CHAT;
        } else if (streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
            audioScene = AUDIO_SCENE_VOICE_RINGING;
        } else if (streamUsage == STREAM_USAGE_RINGTONE) {
            audioScene = AUDIO_SCENE_RINGING;
        }
    } else if (streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        SourceType sourceType = streamDesc->capturerInfo_.sourceType;
        isRecognition = (sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
                         sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION);
    }
    
    if (isAdd) {
        ScoAudioSceneManager::GetInstance().AddScoStream(streamId, audioScene,
            streamDesc->callerUid_, isRecognition);
        AUDIO_INFO_LOG("Track SCO stream %{public}u, audioScene %{public}d, isRecognition %{public}d",
            streamId, audioScene, isRecognition);
    } else {
        ScoAudioSceneManager::GetInstance().RemoveScoStream(streamId);
        AUDIO_INFO_LOG("Untrack SCO stream %{public}u", streamId);
    }
}

void AudioCoreService::HandlePlaybackStoppingOperations(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    std::string caller)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    CHECK_AND_RETURN(streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK);

    // Do not trigger FetchOutputDeviceAndRoute on single call stream stop,
    // FetchOutputDeviceAndRoute is triggered by AudioScene changes instead of a single call stream stop.
    if (!audioSceneManager_.IsPhoneCallOrChatSceneTriggeredByUid(streamDesc->callerUid_) ||
        streamCollector_.IsVoipStreamActive()) {
        FetchOutputDeviceAndRoute(caller);
    }

    const StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    if (enableDualHalToneState_ &&
        static_cast<int64_t>(streamDesc->sessionId_) == static_cast<int64_t>(enableDualHalToneSessionId_)) {
        FetchOutputDeviceAndRoute("UpdateTracker_ForDualHalTone");
        if ((streamDesc->streamStatus_ == STREAM_STATUS_STOPPED ||
            streamDesc->streamStatus_ == STREAM_STATUS_RELEASED) && Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
            AUDIO_INFO_LOG("disable dual hal tone when ringer/alarm renderer stop/release.");
            UpdateDualToneState(false, enableDualHalToneSessionId_);
        }
    }

    if (isRingDualToneOnPrimarySpeaker_ && Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
        bool isNotPaused = streamDesc->streamStatus_ != STREAM_STATUS_PAUSED;
        bool isPausedNotAlarm = streamDesc->streamStatus_ == STREAM_STATUS_PAUSED &&
            streamUsage != STREAM_USAGE_ALARM &&
            !AudioCoreServiceUtils::IsRingScene(AudioSceneManager::GetInstance().GetAudioScene(true));
        if (isNotPaused || isPausedNotAlarm) {
            CHECK_AND_RETURN_LOG(!AudioCoreServiceUtils::IsDualOnActive(), "Dual still on active");
            AUDIO_INFO_LOG("[ADeviceEvent] disable primary speaker dual tone when ringer renderer run over");
            isRingDualToneOnPrimarySpeaker_ = false;
            // Add delay between end of double ringtone and device switch.
            // After the ringtone ends, there may still be residual audio data in the pipeline.
            // Switching the device immediately can cause pop noise due the undrained buffers.
            usleep(RING_DUAL_END_DELAY_US);
            FetchOutputDeviceAndRoute("UpdateTracker_2");
            CHECK_AND_RETURN_LOG(!isRingDualToneOnPrimarySpeaker_, "no need to execute SetInnerStreamMute false");
            for (std::pair<uint32_t, AudioStreamType> stream :  streamsWhenRingDualOnPrimarySpeaker_) {
                audioPolicyManager_.SetDualStreamVolumeMute(stream.first, false);
            }
            streamsWhenRingDualOnPrimarySpeaker_.clear();
            AudioStreamType streamType = streamCollector_.GetStreamType(streamDesc->sessionId_);
            if (streamType == STREAM_MUSIC) {
                audioPolicyManager_.SetDualStreamVolumeMute(streamDesc->sessionId_, false);
            }
        }
    }
}

void AudioCoreService::CheckAndUpdateEffect(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_NEARLINK &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) {
        AUDIO_INFO_LOG("not nearlink or bluetooth, device is %{public}d", streamDesc->newDeviceDescs_[0]->deviceType_);
        return;
    }

    SpatialAudioSourceType sourceType = SPATIAL_AUDIO_SOURCE_TYPE_STEREO;
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = pipeManager_->GetPipeList();
    for (auto pipe : pipeList) {
        AUDIO_INFO_LOG("pipe id %{public}d name %{public}s",
            pipe->id_, pipe->name_.c_str());
        for (auto it : pipe->streamDescriptors_) {
            AUDIO_INFO_LOG("stream id %{public}d, encoding %{public}d, channels %{public}d",
                it->sessionId_, it->streamInfo_.encoding, it->streamInfo_.channels);
            if (it->streamStatus_ == STREAM_STATUS_STARTED && it->sessionId_ != streamDesc->sessionId_) {
                AUDIO_INFO_LOG("has running stream %{public}d", it->sessionId_);
                return;
            }
        }
    }
    if (streamDesc->streamInfo_.encoding == ENCODING_AUDIOVIVID) {
        sourceType = SPATIAL_AUDIO_SOURCE_TYPE_AUDIO_VIVID;
    } else if (streamDesc->streamInfo_.channels > STEREO) {
        sourceType = SPATIAL_AUDIO_SOURCE_TYPE_MULTI_CHANNEL;
    }
    AUDIO_INFO_LOG("need update spatializaitonenabled %{public}d", sourceType);
    // 通知hpaemanager音效模式变化 最终通知给audioeffectchain
    HPAE::IHpaeManager::GetHpaeManager().HandleBypassSpatializationForStereo(
        sourceType == SPATIAL_AUDIO_SOURCE_TYPE_STEREO);
    // AudioSpatializationService里面调用回调通知管家音效模式变化
    AudioSpatializationService::GetAudioSpatializationService().SetSpatialAudioSourceType(sourceType);

    if (!AudioSpatializationService::GetAudioSpatializationService().IsAdaptiveSpatialRenderingEnabled(
        streamDesc->newDeviceDescs_[0]->macAddress_)) {
        AUDIO_INFO_LOG("adaptiveSpatialRendering is not enabled");
        return;
    }
    AUDIO_INFO_LOG("IsHeadTrackingEnabled: %{public}d",
        AudioSpatializationService::GetAudioSpatializationService().IsHeadTrackingEnabled(
            streamDesc->newDeviceDescs_[0]->macAddress_));
    // 通知星闪、蓝牙空间音频、头动开关变化
    AudioSpatializationService::GetAudioSpatializationService().SetBypassSpatialForStereo(
        streamDesc->newDeviceDescs_[0], sourceType == SPATIAL_AUDIO_SOURCE_TYPE_STEREO);
}

void AudioCoreService::ForceStopInterphoneStreamsIfNecessary(const std::shared_ptr<AudioStreamDescriptor>& streamDesc)
{
    // Force stop interphone streams when VoIP/Cellular stream starts
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK &&
        (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION)) {
        if (pipeManager_->IsInterphoneStreamActive()) {
            AUDIO_INFO_LOG("Force stop interphone streams when VoIP/Cellular renderer starts");
            ForceStopInterphoneRenderer();
        }
    }
    if (streamDesc->audioMode_ == AUDIO_MODE_RECORD &&
        (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION ||
        streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_CALL)) {
        if (pipeManager_->IsInterphoneStreamActive() || pipeManager_->IsInterphoneCapturerActive()) {
            AUDIO_INFO_LOG("Force stop interphone streams when VoIP/Cellular capturer starts");
            ForceStopInterphoneRenderer();
        }
    }
}

int32_t AudioCoreService::StartClientForPlayback(uint32_t sessionId,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    AudioRouterInfra::GetInstance().UpdateOutputStreamState(GetRealUid(streamDesc), sessionId,
        streamDesc->rendererInfo_.streamUsage, RendererState::RENDERER_RUNNING);
    int32_t ret = FetchAndActivateOutputDevice(deviceDesc, streamDesc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchAndActivateOutputDevice fail!");
    deviceDesc = streamDesc->GetMainNewDeviceDesc();
    CHECK_AND_RETURN_RET_LOG(deviceDesc, ERR_NULL_POINTER, "Capturer deviceDesc is nullptr");
    CheckAndSetCurrentOutputDevice(deviceDesc, streamDesc->sessionId_);
    audioVolumeManager_.SetVolumeForSwitchDevice(deviceDesc, true, streamDesc);
    if (pipeManager_->GetUpdateRouteSupport()) {
        UpdateOutputRoute(streamDesc, pipeManager_->QueryPipeIdBySessionId(sessionId));
    }
    streamCollector_.UpdateRendererDeviceInfo(deviceDesc);
    return SUCCESS;
}

int32_t AudioCoreService::StartClientForRecord(uint32_t sessionId,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    AudioRouterInfra::GetInstance().UpdateInputStreamState(GetRealUid(streamDesc), sessionId,
        streamDesc->capturerInfo_.sourceType, CapturerState::CAPTURER_RUNNING);
    FetchInputDeviceAndRoute("StartClient");
    deviceDesc = streamDesc->GetMainNewDeviceDesc();
    CHECK_AND_RETURN_RET_LOG(deviceDesc, ERR_NULL_POINTER, "Capturer deviceDesc is nullptr");
    int32_t inputRet = ActivateInputDevice(streamDesc);
    CHECK_AND_RETURN_RET_LOG(inputRet != REFETCH_DEVICE, SUCCESS, "Activate input device failed, refetch device");
    CHECK_AND_RETURN_RET_LOG(inputRet == SUCCESS, inputRet, "Activate input device failed");
    CheckAndSetCurrentInputDevice(deviceDesc, GetRealUid(streamDesc));
    audioActiveDevice_.UpdateActiveDeviceRoute(deviceDesc->deviceType_, DeviceFlag::INPUT_DEVICES_FLAG,
        pipeManager_->QueryPipeIdBySessionId(streamDesc->sessionId_), deviceDesc->networkId_);
    streamCollector_.UpdateCapturerDeviceInfo(deviceDesc);
    return SUCCESS;
}

int32_t AudioCoreService::StartClient(uint32_t sessionId)
{
    CHECK_AND_CALL_FUNC_RETURN_RET(!pipeManager_->IsModemCommunicationIdExist(sessionId), SUCCESS,
        HILOG_COMM_ERROR("[StartClient]Modem communication ring, directly return"));
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET(streamDesc != nullptr, ERR_NULL_POINTER,
        HILOG_COMM_ERROR("[StartClient]Cannot find session %{public}u", sessionId));

    // Force stop interphone streams when VoIP/Cellular stream starts
    ForceStopInterphoneStreamsIfNecessary(streamDesc);
    CheckAndSleepBeforeRingDualDeviceSet(streamDesc);
    pipeManager_->StartClient(sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET(!streamDesc->newDeviceDescs_.empty(), ERR_INVALID_PARAM,
        HILOG_COMM_ERROR("[StartClient]newDeviceDescs_ is empty"));
    // [Capturer NOTE 1.0] be careful device may be changed.
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->GetMainNewDeviceDesc();
    CHECK_AND_CALL_FUNC_RETURN_RET(deviceDesc, ERR_NULL_POINTER, HILOG_COMM_ERROR("deviceDesc is nullptr"));
    // Update a2dp offload flag for update active route, if a2dp offload flag is not true, audioserver
    // will reset a2dp device to none.
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForStartStream(static_cast<int32_t>(sessionId));

    int32_t ret = SUCCESS;
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        ret = StartClientForPlayback(sessionId, streamDesc, deviceDesc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartClientForPlayback fail!");
    } else {
        ret = StartClientForRecord(sessionId, streamDesc, deviceDesc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartClientForRecord fail!");
    }

    streamDesc->startTimeStamp_ = ClockTime::GetCurNano();
    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc, false);
    // [Capturer NOTE 1.3] should use the new device.
    CheckForRemoteDeviceState(deviceDesc);
    CheckAndUpdateEffect(streamDesc);
    if (streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        ReloadCaptureSession(sessionId, SESSION_OPERATION_START);
        ReEvaluateVoipPrivacyMuteForCapturers("AudioCoreService::StartClient");
    }
    return SUCCESS;
}

void AudioCoreService::ForceStopInterphoneRenderer()
{
    Trace trace("AudioCoreService::ForceStopInterphoneRenderer");

    // Stop interphone renderer streams (downlink)
    std::vector<std::shared_ptr<AudioStreamDescriptor>> interphoneRenderers =
        pipeManager_->GetStreamDescsByStreamUsage(STREAM_USAGE_INTERPHONE);

    for (auto &streamDesc : interphoneRenderers) {
        if (streamDesc != nullptr && streamDesc->streamStatus_ != STREAM_STATUS_STOPPED) {
            AUDIO_INFO_LOG("Force stop interphone renderer, sessionId: %{public}u", streamDesc->sessionId_);
            pipeManager_->StopClient(streamDesc->sessionId_);
        }
    }

    // Stop interphone capturer streams (uplink)
    std::vector<std::shared_ptr<AudioStreamDescriptor>> interphoneCapturers =
        pipeManager_->GetStreamDescsBySourceType(SOURCE_TYPE_INTERPHONE);

    for (auto &streamDesc : interphoneCapturers) {
        if (streamDesc != nullptr && streamDesc->streamStatus_ != STREAM_STATUS_STOPPED) {
            AUDIO_INFO_LOG("Force stop interphone capturer, sessionId: %{public}u", streamDesc->sessionId_);
            pipeManager_->StopClient(streamDesc->sessionId_);
            ReloadCaptureSession(streamDesc->sessionId_, SESSION_OPERATION_STOP);
        }
    }
}

int32_t AudioCoreService::PauseClient(uint32_t sessionId)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        ReloadCaptureSession(sessionId, SESSION_OPERATION_PAUSE);
    }
    pipeManager_->PauseClient(sessionId);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        AudioRouterInfra::GetInstance().UpdateInputStreamState(GetRealUid(streamDesc), sessionId,
            streamDesc->capturerInfo_.sourceType, CapturerState::CAPTURER_PAUSED);
    } else if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        AudioRouterInfra::GetInstance().UpdateOutputStreamState(GetRealUid(streamDesc), sessionId,
            streamDesc->rendererInfo_.streamUsage, RendererState::RENDERER_PAUSED);
    }
    ForceRemoveSleStreamType(streamDesc);

    CheckScoState(streamDesc);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        HandlePlaybackStoppingOperations(streamDesc, "PauseClient");
    }
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        ReEvaluateVoipPrivacyMuteForCapturers("AudioCoreService::PauseClient");
    }

    return SUCCESS;
}

int32_t AudioCoreService::StandbyClient(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET(pipeManager_, SUCCESS);
    pipeManager_->StandbyClient(sessionId);
    auto streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET(streamDesc, SUCCESS);
    ForceRemoveSleStreamType(streamDesc);
    return SUCCESS;
}

void AudioCoreService::NotifyStandbyStatus(uint32_t sessionId, bool standbyStatus)
{
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager_ is nullptr");
    if (standbyStatus) {
        pipeManager_->StandbyClient(sessionId);
        auto streamDesc = pipeManager_->GetStreamDescById(sessionId);
        if (streamDesc) {
            ForceRemoveSleStreamType(streamDesc);
        }
    } else {
        pipeManager_->ClearStandbyFlag(sessionId);
    }
}

int32_t AudioCoreService::StopClient(uint32_t sessionId)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        ReloadCaptureSession(sessionId, SESSION_OPERATION_STOP);
    }
    pipeManager_->StopClient(sessionId);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        AudioRouterInfra::GetInstance().UpdateInputStreamState(GetRealUid(streamDesc), sessionId,
            streamDesc->capturerInfo_.sourceType, CapturerState::CAPTURER_STOPPED);
    }
    ForceRemoveSleStreamType(streamDesc);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        AudioRouterInfra::GetInstance().UpdateOutputStreamState(GetRealUid(streamDesc), sessionId,
            streamDesc->rendererInfo_.streamUsage, RendererState::RENDERER_STOPPED);
    }

    CheckScoState(streamDesc);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        HandlePlaybackStoppingOperations(streamDesc, "StopClient");
    }
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        ReEvaluateVoipPrivacyMuteForCapturers("AudioCoreService::StopClient");
    }

    return SUCCESS;
}

int32_t AudioCoreService::ReleaseClient(uint32_t sessionId, SessionOperationMsg opMsg, bool needRemoveFromMap)
{
    if (pipeManager_->IsModemCommunicationIdExist(sessionId)) {
        AUDIO_INFO_LOG("Modem communication, sessionId %{public}u", sessionId);
        sleAudioDeviceManager_.UpdateSleStreamTypeCount(pipeManager_->GetModemCommunicationStreamDescById(sessionId),
            true);
        pipeManager_->RemoveModemCommunicationId(sessionId);
        return SUCCESS;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        AudioRouterInfra::GetInstance().UpdateInputStreamState(GetRealUid(streamDesc), sessionId,
            streamDesc->capturerInfo_.sourceType, CapturerState::CAPTURER_RELEASED);
    }
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        AudioRouterInfra::GetInstance().UpdateOutputStreamState(GetRealUid(streamDesc), sessionId,
            streamDesc->rendererInfo_.streamUsage, RendererState::RENDERER_RELEASED);
    }
    ForceRemoveSleStreamType(streamDesc);
    pipeManager_->RemoveClient(sessionId);
    audioOffloadStream_.UnsetOffloadStatus(sessionId);

    CheckScoState(streamDesc);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        HandlePlaybackStoppingOperations(streamDesc, "ReleaseClient");
    }

    RemoveUnusedPipe();
    if (opMsg == SESSION_OP_MSG_REMOVE_PIPE) {
        RemoveUnusedRecordPipe();
    }
    DeleteSessionId(sessionId);
    if (streamDesc != nullptr && streamDesc->audioMode_ == AUDIO_MODE_RECORD) {
        if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            ReleaseCaptureInjector();
        }
        ReloadCaptureSession(sessionId, SESSION_OPERATION_RELEASE);
        ReEvaluateVoipPrivacyMuteForCapturers("AudioCoreService::ReleaseClient");
    }

    if (needRemoveFromMap) {
        pipeManager_->RemoveSwitchStream(sessionId);
    }

    return SUCCESS;
}

int32_t AudioCoreService::SetAudioScene(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    audioSceneManager_.SetAudioScenePre(audioScene);
    audioSceneManager_.SetPhoneCallOrChatSceneTriggerUid(uid);
    audioSceneManager_.SetAudioSceneOwnerUid(audioScene == 0 ? 0 : uid);
    AudioScene lastAudioScene = audioSceneManager_.GetLastAudioScene();
    bool isSameScene = audioSceneManager_.IsSameAudioScene();
    int32_t result = audioSceneManager_.SetAudioSceneAfter(audioScene, audioA2dpOffloadFlag_.GetA2dpOffloadFlag());
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "failed [%{public}d]", result);

    bool isDealStreamsWhenRingDual = HandleRingToNonRingSceneChange(lastAudioScene, audioScene);
    FetchDeviceAndRoute("SetAudioScene", AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);
    if (isDealStreamsWhenRingDual) {
        for (std::pair<uint32_t, AudioStreamType> stream : streamsWhenRingDualOnPrimarySpeaker_) {
            audioPolicyManager_.SetDualStreamVolumeMute(stream.first, false);
        }
        streamsWhenRingDualOnPrimarySpeaker_.clear();
    }

    if (!isSameScene) {
        SetSleVoiceStatusFlag(audioScene);
        OnAudioSceneChange(audioScene);
        if (audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid).deviceType_ == DEVICE_TYPE_NEARLINK &&
            lastAudioScene == AUDIO_SCENE_DEFAULT && audioScene != AUDIO_SCENE_DEFAULT) {
            OnPreferredOutputDeviceUpdated(audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid),
                AudioStreamDeviceChangeReason::UNKNOWN);
        }
        if (audioScene == AUDIO_SCENE_DEFAULT &&
            audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid).IsRemoteDevice()) {
            OnPreferredOutputDeviceUpdated(audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid),
                AudioStreamDeviceChangeReason::OVERRODE);
        }
    }

    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        // Make sure the STREAM_VOICE_CALL volume is set before the calling starts.
        audioVolumeManager_.SetVoiceCallVolume(audioVolumeManager_.GetSystemVolumeLevel(STREAM_VOICE_CALL));
    } else if (lastAudioScene == AUDIO_SCENE_PHONE_CALL && audioScene != AUDIO_SCENE_PHONE_CALL) {
        audioVolumeManager_.SetVoiceRingtoneMute(false);
    }
    if (audioSceneManager_.IsCallEnded()) {
        audioVolumeManager_.ClearLoudVolumeHoldMapForCall();
    }
    if (audioSceneManager_.IsHangUpScene()) {
        audioVolumeManager_.RefreshActiveDeviceVolume();
    }
    if (lastAudioScene == AUDIO_SCENE_RINGING && audioScene != AUDIO_SCENE_RINGING &&
        audioVolumeManager_.IsAppRingMuted(uid)) {
        audioVolumeManager_.SetAppRingMuted(uid, false); // unmute the STREAM_RING for the app.
    }
    ReloadSourceForDeviceChange(audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid),
        audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid), "SetAudioScene");
    return SUCCESS;
}

bool AudioCoreService::IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc)
{
    return audioDeviceManager_.IsArmUsbDevice(deviceDesc);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetDevices(DeviceFlag deviceFlag)
{
    return audioConnectedDevice_.GetDevicesInner(deviceFlag);
}

int32_t AudioCoreService::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid)
{
    AUDIO_INFO_LOG("[ADeviceEvent] withlock device %{public}d, active %{public}d from uid %{public}d",
        deviceType, active, uid);
    int32_t ret = AudioSelectInterfaceService::GetInstance().SetDeviceActive(deviceType, active, "", uid);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "SetDeviceActive failed", false),
        "SetDeviceActive failed");

    FetchDeviceAndRoute("SetDeviceActive", AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE);

    ReloadSourceForDeviceChange(audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid),
        audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid), "SetDeviceActive");
    return SUCCESS;
}

int32_t AudioCoreService::SetInputDevice(const DeviceType deviceType, const uint32_t sessionID, int32_t uid)
{
    return AudioSelectInterfaceService::GetInstance().SetInputDevice(deviceType, sessionID, uid);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetPreferredOutputDeviceDescInner(
    AudioRendererInfo &rendererInfo, std::string networkId, const int32_t uid, const uint32_t streamId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    RouterType bypassType = RouterType::ROUTER_TYPE_NONE;
    if (rendererInfo.streamUsage <= STREAM_USAGE_UNKNOWN ||
        rendererInfo.streamUsage > STREAM_USAGE_MAX) {
        AUDIO_WARNING_LOG("Invalid usage[%{public}d], return current device.", rendererInfo.streamUsage);
        deviceList = audioRouterSelectStrategy_.GetCurrentOutputDevice(uid);
        if (deviceList.empty()) {
            deviceList.push_back(make_shared<AudioDeviceDescriptor>());
        }
        return deviceList;
    }
    if (networkId == LOCAL_NETWORK_ID) {
        FetchDeviceInfo info = { rendererInfo.streamUsage, "GetPreferredOutputDeviceDescInner_1", bypassType };
        info.streamId = streamId;
        info.clientUID = uid;
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioRouterCenter_.FetchOutputDevices(info);
        for (size_t i = 0; i < descs.size(); i++) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*descs[i]);
            deviceList.push_back(devDesc);
        }

        info = { rendererInfo.streamUsage, rendererInfo.streamUsage, uid,
            bypassType, PIPE_TYPE_OUT_NORMAL, PRIVACY_TYPE_PUBLIC };
        info.caller = "GetPreferredOutputDeviceDescInner_2";
        info.streamId = streamId;
        descs = audioRouterCenter_.FetchDupDevices(info);
        for (size_t i = 0; i < descs.size(); i++) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*descs[i]);
            deviceList.push_back(devDesc);
        }
    } else {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetRemoteRenderDevices();
        for (const auto &desc : descs) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
            deviceList.push_back(devDesc);
        }
    }

    return deviceList;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetPreferredInputDeviceDescInner(
    AudioCapturerInfo &captureInfo, std::string networkId, const int32_t uid, const uint32_t streamId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (captureInfo.sourceType <= SOURCE_TYPE_INVALID || captureInfo.sourceType > SOURCE_TYPE_MAX) {
        deviceList = audioRouterSelectStrategy_.GetCurrentInputDevice(uid);
        if (deviceList.empty()) {
            deviceList.push_back(make_shared<AudioDeviceDescriptor>());
        }
        return deviceList;
    }

    if (captureInfo.sourceType == SOURCE_TYPE_WAKEUP) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
        devDesc->networkId_ = LOCAL_NETWORK_ID;
        deviceList.push_back(devDesc);
        return deviceList;
    }

    if (networkId == LOCAL_NETWORK_ID) {
        RouterType routerType = ROUTER_TYPE_NONE;
        std::shared_ptr<AudioDeviceDescriptor> desc = audioRouterCenter_.FetchInputDevice(
            captureInfo.sourceType, uid, routerType);
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, deviceList, "desc is nullptr");
        if (desc->deviceType_ == DEVICE_TYPE_NONE && (captureInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE ||
            captureInfo.sourceType == SOURCE_TYPE_REMOTE_CAST)) {
            desc->deviceType_ = DEVICE_TYPE_INVALID;
            desc->deviceRole_ = INPUT_DEVICE;
        }
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
        deviceList.push_back(devDesc);
    } else {
        vector<shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetRemoteCaptureDevices();
        for (const auto &desc : descs) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
            deviceList.push_back(devDesc);
        }
    }

    return deviceList;
}

int32_t AudioCoreService::GetSessionDefaultOutputDevice(const int32_t callerPid, DeviceType &deviceType)
{
    deviceType = audioSessionService_.GetSessionDefaultOutputDevice(callerPid);
    return SUCCESS;
}

int32_t AudioCoreService::SetSessionDefaultOutputDevice(const int32_t callerPid,
    const DeviceType &deviceType, bool skipForce, int32_t callerUid)
{
    vector<uint32_t> sessionIDList = pipeManager_->GetStreamIdsByPid(callerPid);
    CHECK_AND_RETURN_RET_LOG(pipeManager_->GetHasEarpiece(), ERR_NOT_SUPPORTED,
        "the device has no earpiece");
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    bool forceFetch = false;
    for (auto &changeInfo : audioRendererChangeInfos) {
        bool currentSessionID = false;
        uint32_t sessionIDListSize = sessionIDList.size();
        for (uint32_t i = 0; i < sessionIDListSize; i++) {
            if (changeInfo->sessionId == static_cast<int32_t>(sessionIDList[i])) {
                currentSessionID = true;
                break;
            }
        }
        if (currentSessionID &&
            (changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
                changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
                changeInfo->rendererInfo.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
                changeInfo->rendererInfo.streamUsage == STREAM_USAGE_INTERPHONE)) {
            CHECK_AND_CONTINUE(!skipForce);
            AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_CALL_RENDER,
                std::make_shared<AudioDeviceDescriptor>(), changeInfo->clientUID, "SetDefaultOutputDevice");
            forceFetch = true;
        }
    }
    return audioSessionService_.SetSessionDefaultOutputDevice(callerPid, deviceType, forceFetch, callerUid);
}

bool AudioCoreService::GetVolumeGroupInfos(std::vector<sptr<VolumeGroupInfo>> &infos)
{
    return audioVolumeManager_.GetVolumeGroupInfosNotWait(infos);
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::GetActiveBluetoothDevice()
{
    std::shared_ptr<AudioDeviceDescriptor> preferredDesc =
        audioRouterSelectStrategy_.GetCallOutputDevice(INVALID_UID, INVALID_STREAM_ID);
    if (preferredDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        return preferredDesc;
    }

    std::vector<shared_ptr<AudioDeviceDescriptor>> audioPrivacyDeviceDescriptors =
        audioDeviceManager_.GetCommRenderPrivacyDevices();
    std::vector<shared_ptr<AudioDeviceDescriptor>> activeDeviceDescriptors;

    for (const auto &desc : audioPrivacyDeviceDescriptors) {
        if (desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO || desc->exceptionFlag_ || !desc->isEnable_ ||
            desc->connectState_ == SUSPEND_CONNECTED || audioRouterSelectStrategy_.GetScoExcluded() ||
            audioRouterSelectStrategy_.IsDeviceExcluded(desc, AudioDeviceUsage::CALL_OUTPUT_DEVICES)) {
            continue;
        }
        activeDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }

    uint32_t btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        activeDeviceDescriptors = audioDeviceManager_.GetCommRenderBTCarDevices();
    }
    btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        return make_shared<AudioDeviceDescriptor>();
    } else if (btDeviceSize == 1) {
        shared_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[0]);
        return res;
    }

    uint32_t index = 0;
    for (uint32_t i = 1; i < btDeviceSize; ++i) {
        if (activeDeviceDescriptors[i]->connectTimeStamp_ >
            activeDeviceDescriptors[index]->connectTimeStamp_) {
            index = i;
        }
    }
    shared_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[index]);
    return res;
}

void AudioCoreService::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    audioDeviceStatus_.OnDeviceInfoUpdated(desc, command);
}

uint32_t AudioCoreService::GetPaIndexByPortName(const std::string &portName)
{
    return audioDeviceStatus_.GetPaIndexByPortName(portName);
}

int32_t AudioCoreService::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
    const int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

    int32_t ret = AudioSelectInterfaceService::GetInstance().SetDeviceActive(deviceType, active, address, uid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetCallDeviceActive failed");
    ret = FetchDeviceAndRoute("SetCallDeviceActive", AudioStreamDeviceChangeReason::OVERRODE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchDeviceAndRoute failed");
    ReloadSourceForDeviceChange(audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid),
        audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid), "SetCallDeviceActive");

    return SUCCESS;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors = audioDeviceManager_.GetAvailableDevicesByUsage(usage);
    return audioDeviceDescriptors;
}

std::vector<sptr<MicrophoneDescriptor>> AudioCoreService::GetAvailableMicrophones()
{
    return audioMicrophoneDescriptor_.GetAvailableMicrophones();
}

std::vector<sptr<MicrophoneDescriptor>> AudioCoreService::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    return audioMicrophoneDescriptor_.GetAudioCapturerMicrophoneDescriptors(sessionId);
}

int32_t AudioCoreService::GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    int32_t status = streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer Get renderer change info failed");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices =
        audioConnectedDevice_.GetDevicesInner(OUTPUT_DEVICES_FLAG);
    DeviceType activeDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    DeviceRole activeDeviceRole = OUTPUT_DEVICE;
    std::string activeDeviceMac = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();

    const auto& itr = std::find_if(outputDevices.begin(), outputDevices.end(),
        [&activeDeviceType, &activeDeviceRole, &activeDeviceMac](const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            // This A2DP device is not the active A2DP device. Skip it.
            return (activeDeviceType != DEVICE_TYPE_BLUETOOTH_A2DP && activeDeviceType != DEVICE_TYPE_BLUETOOTH_SCO) ||
                desc->macAddress_ == activeDeviceMac;
        }
        return false;
    });

    if (itr != outputDevices.end()) {
        size_t rendererInfosSize = audioRendererChangeInfos.size();
        for (size_t i = 0; i < rendererInfosSize; i++) {
            UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos[i], hasSystemPermission);
            std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(
                audioRendererChangeInfos[i]->sessionId);
            CHECK_AND_CONTINUE(streamDesc != nullptr);
            CHECK_AND_CONTINUE(streamDesc->rendererTarget_ != INJECT_TO_VOICE_COMMUNICATION_CAPTURE);
            audioDeviceCommon_.UpdateDeviceInfo(audioRendererChangeInfos[i]->outputDeviceInfo,
                streamDesc->newDeviceDescs_.front(), hasBTPermission, hasSystemPermission);
        }
    }
    return status;
}

int32_t AudioCoreService::GetCurrentCapturerChangeInfos(
    vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
    bool hasBTPermission, bool hasSystemPermission)
{
    int status = streamCollector_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer:: Get capturer change info failed");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> inputDevices = GetDevices(INPUT_DEVICES_FLAG);
#ifdef INPUT_FEATURE_INDEPENDENT_MODE
    size_t capturerInfosSize = audioCapturerChangeInfos.size();
    for (size_t i = 0; i < capturerInfosSize; i++) {
        CHECK_AND_CONTINUE(audioCapturerChangeInfos[i] != nullptr);
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, status, "pipeManager_ is nullptr");
        std::shared_ptr<AudioStreamDescriptor> streamDesc =
            pipeManager_->GetStreamDescById(audioCapturerChangeInfos[i]->sessionId);
        CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, status, "streamDesc is nullptr");
        DeviceType activeDeviceType = audioActiveDevice_.GetCurrentInputDeviceType(GetRealUid(streamDesc));
        DeviceRole activeDeviceRole = INPUT_DEVICE;
        for (std::shared_ptr<AudioDeviceDescriptor> desc : inputDevices) {
            if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
                UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos[i], hasSystemPermission);
                CHECK_AND_CONTINUE(audioRouterCenter_.IsConfigRouterStrategy(
                    audioCapturerChangeInfos[i]->capturerInfo.sourceType));
                audioDeviceCommon_.UpdateDeviceInfo(audioCapturerChangeInfos[i]->inputDeviceInfo, desc,
                    hasBTPermission, hasSystemPermission);
                break;
            }
        }
    }
#else
    DeviceType activeDeviceType = audioActiveDevice_.GetCurrentInputDeviceType();
    DeviceRole activeDeviceRole = INPUT_DEVICE;
    for (std::shared_ptr<AudioDeviceDescriptor> desc : inputDevices) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            size_t capturerInfosSize = audioCapturerChangeInfos.size();
            for (size_t i = 0; i < capturerInfosSize; i++) {
                CHECK_AND_CONTINUE(audioCapturerChangeInfos[i] != nullptr);
                UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos[i], hasSystemPermission);
                CHECK_AND_CONTINUE(audioRouterCenter_.IsConfigRouterStrategy(
                    audioCapturerChangeInfos[i]->capturerInfo.sourceType));
                audioDeviceCommon_.UpdateDeviceInfo(audioCapturerChangeInfos[i]->inputDeviceInfo, desc,
                    hasBTPermission, hasSystemPermission);
            }
            break;
        }
    }
#endif
    return status;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    return audioRouterSelectStrategy_.GetExcludedDevices(audioDevUsage);
}

int32_t AudioCoreService::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    int32_t result = AudioSelectInterfaceService::GetInstance().ExcludeOutputDevices(audioDevUsage,
        audioDeviceDescriptors);
    DeactivateA2dpAfterExclude(audioDeviceDescriptors, pipeManager_->GetAllOutputStreamDescs());
    return result;
}

int32_t AudioCoreService::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    return AudioSelectInterfaceService::GetInstance().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioCoreService::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    auto uid = SYSTEM_UID;
    if (mode == AUDIO_MODE_RECORD) {
        audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
            streamChangeInfo.audioCapturerChangeInfo.sessionId, DEVICE_TYPE_NONE);
        if (apiVersion > 0 && apiVersion < API_11) {
            CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is nullptr");
            std::shared_ptr<AudioStreamDescriptor> streamDesc =
                pipeManager_->GetStreamDescById(streamChangeInfo.audioCapturerChangeInfo.sessionId);
            CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERROR, "streamDesc is nullptr");
            uid = GetRealUid(streamDesc);
            audioDeviceCommon_.UpdateDeviceInfo(streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo,
                std::make_shared<AudioDeviceDescriptor>(audioRouterSelectStrategy_
                .Get1stCurrentInputDevice(uid)), false, false);
        }
    } else if (apiVersion > 0 && apiVersion < API_11) {
        if (pipeManager_) {
            auto streamDesc = pipeManager_->GetStreamDescById(streamChangeInfo.audioRendererChangeInfo.sessionId);
            if (streamDesc) {
                uid = GetRealUid(streamDesc);
            }
        }
        audioDeviceCommon_.UpdateDeviceInfo(streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo,
            std::make_shared<AudioDeviceDescriptor>(audioRouterSelectStrategy_
            .Get1stCurrentOutputDevice(uid)), false, false);
    }
    return streamCollector_.RegisterTracker(mode, streamChangeInfo, object);
}

void AudioCoreService::SetAudioRouteCallback(uint32_t sessionId, const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_LOG(object != nullptr, "object is nullptr");
    sptr<IStandardAudioPolicyManagerListener> listener = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_LOG(listener != nullptr, "listener is nullptr");
    std::lock_guard<std::mutex> lock(routeUpdateCallbackMutex_);
    routeUpdateCallback_[sessionId].listener = listener;
    routeUpdateCallback_[sessionId].clientUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
}

void AudioCoreService::UnsetAudioRouteCallback(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(routeUpdateCallbackMutex_);
    CHECK_AND_RETURN_LOG(routeUpdateCallback_.count(sessionId) != 0, "sessionId not exists");
    uid_t callingUid = static_cast<uid_t>(IPCSkeleton::GetCallingUid());
    CHECK_AND_RETURN_LOG(routeUpdateCallback_[sessionId].clientUid == callingUid,
        "The sessionId %{public}u does not belong to uid %{public}u", sessionId, callingUid);
    routeUpdateCallback_.erase(sessionId);
}

int32_t AudioCoreService::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    int32_t ret = streamCollector_.UpdateTracker(mode, streamChangeInfo);

    const auto &rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    if (mode == AUDIO_MODE_PLAYBACK &&
        (rendererState == RENDERER_PREPARED || rendererState == RENDERER_NEW || rendererState == RENDERER_INVALID)) {
        return ret; // only update tracker in new and prepared
    }

    UpdateTracker(mode, streamChangeInfo, rendererState);

    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(audioActiveDevice_.GetCurrentOutputDeviceType());
    }

    SendA2dpConnectedWhileRunning(rendererState, streamChangeInfo.audioRendererChangeInfo.sessionId);

    if (mode == AUDIO_MODE_PLAYBACK) {
        audioOffloadStream_.UpdateOffloadStatusFromUpdateTracker(
            streamChangeInfo.audioRendererChangeInfo.sessionId,
            streamChangeInfo.audioRendererChangeInfo.rendererState);
    }
    return ret;
}

void AudioCoreService::RegisteredTrackerClientDied(pid_t uid, pid_t pid)
{
    int32_t curUid = static_cast<int32_t>(uid);
    int32_t curPid = static_cast<int32_t>(pid);
    AudioRouterInfra::GetInstance().OnAppDied(curUid);
    UpdateDefaultOutputDeviceWhenStopping(curUid);

    audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptor(curUid);
    streamCollector_.RegisteredTrackerClientDied(curUid, curPid);
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager is nullptr");
    auto sessionIds = pipeManager_->GetStreamIdsByUidAndPid(curUid, curPid);
    for (auto sessionId : sessionIds) {
        ReleaseClient(sessionId);
        pipeManager_->RemoveSwitchStream(sessionId);
        UnsetAudioRouteCallback(sessionId);
    }
    FetchOutputDeviceAndRoute("RegisteredTrackerClientDied");
    FetchInputDeviceAndRoute("RegisteredTrackerClientDied");
    audioDeviceCommon_.ClientDiedDisconnectScoRecognition();
}

bool AudioCoreService::ConnectServiceAdapter()
{
    return audioPolicyManager_.ConnectServiceAdapter();
}

void AudioCoreService::OnReceiveUpdateDeviceNameEvent(const std::string macAddress, const std::string deviceName)
{
    audioDeviceManager_.OnReceiveUpdateDeviceNameEvent(macAddress, deviceName);
    audioConnectedDevice_.SetDisplayName(macAddress, deviceName);
}

void AudioCoreService::DumpSelectHistory(std::string &dumpString)
{
    dumpString += "Select device history infos\n";
    std::lock_guard<std::mutex> lock(hisQueueMutex_);
    dumpString += "  - TotalPipeNums: " + std::to_string(selectDeviceHistory_.size()) + "\n\n";
    for (auto &item : selectDeviceHistory_) {
        dumpString += item + "\n";
    }
    dumpString += "\n";
}

void AudioCoreService::RecordSelectDevice(const std::string &selectHistory)
{
    std::lock_guard<std::mutex> lock(hisQueueMutex_);
    if (selectDeviceHistory_.size() < SELECT_DEVICE_HISTORY_LIMIT) {
        selectDeviceHistory_.push_back(selectHistory);
        return;
    }
    while (selectDeviceHistory_.size() >= SELECT_DEVICE_HISTORY_LIMIT) {
        selectDeviceHistory_.pop_front();
    }
    selectDeviceHistory_.push_back(selectHistory);
    return;
}

int32_t AudioCoreService::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode,
    const bool isNeedNotifyBt)
{
    if (!selectedDesc.empty() && selectedDesc[0] != nullptr) {
        // eg. 2025-06-22-21:12:07:666|Uid: 6700 select output device: LOCAL_DEVICE type:2
        std::string selectHistory = GetTime() + "|Uid:" + std::to_string(IPCSkeleton::GetCallingUid()) + " Pid:" +
            std::to_string(IPCSkeleton::GetCallingPid()) + " select output device:" + selectedDesc[0]->networkId_ +
            " type:" + std::to_string(selectedDesc[0]->deviceType_);
        RecordSelectDevice(selectHistory);
    }

    return AudioSelectInterfaceService::GetInstance().SelectOutputDevice(
        audioRendererFilter, selectedDesc, audioDeviceSelectMode, isNeedNotifyBt);
}

void AudioCoreService::NotifyDistributedOutputChange(const AudioDeviceDescriptor &deviceDesc)
{
    audioDeviceCommon_.NotifyDistributedOutputChange(deviceDesc);
}

int32_t AudioCoreService::SetMediaOutputDeviceByUid(DeviceType deviceType, const int32_t uid)
{
    AUDIO_INFO_LOG("select device %{public}d from uid %{public}d", deviceType, uid);
    int32_t ret = AudioSelectInterfaceService::GetInstance().SetMediaOutputDeviceByUid(deviceType, uid);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "SetMediaOutputDeviceByUid failed", false),
        "SetMediaOutputDeviceByUid failed");
    FetchOutputDeviceAndRoute("SetMediaOutputDeviceByUid", AudioStreamDeviceChangeReason::OVERRODE);
    ReloadSourceForDeviceChange(audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid),
        audioRouterSelectStrategy_.Get1stCurrentOutputDevice(uid), "SetMediaOutputDeviceByUid");
    return SUCCESS;
}

int32_t AudioCoreService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    if (!selectedDesc.empty() && selectedDesc[0] != nullptr) {
        // eg. 2025-06-22-21:12:07:666|Uid: 6700 select input device: LOCAL_DEVICE type:15
        std::string selectHistory = GetTime() + "|Uid:" + std::to_string(IPCSkeleton::GetCallingUid()) + " Pid:" +
            std::to_string(IPCSkeleton::GetCallingPid()) + " select input device:" + selectedDesc[0]->networkId_ +
            " type:" + std::to_string(selectedDesc[0]->deviceType_);
        RecordSelectDevice(selectHistory);
    }
    return AudioSelectInterfaceService::GetInstance().SelectInputDevice(audioCapturerFilter, selectedDesc);
}

int32_t AudioCoreService::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &selectedDesc,
    int32_t uid)
{
    return AudioSelectInterfaceService::GetInstance().SelectInputDeviceByUid(selectedDesc, uid);
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::GetSelectedInputDeviceByUid(int32_t uid)
{
    return audioRouterSelectStrategy_.GetMediaInputDevice(
        uid, INVALID_STREAM_ID, SOURCE_TYPE_INVALID);
}

int32_t AudioCoreService::ClearSelectedInputDeviceByUid(int32_t uid)
{
    return AudioSelectInterfaceService::GetInstance().SelectInputDeviceByUid(
        std::make_shared<AudioDeviceDescriptor>(), uid);
}

int32_t AudioCoreService::PreferBluetoothAndNearlinkRecordByUid(int32_t uid,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    AudioRouterInfra::GetInstance().UpdatePreferredInputCategory(uid, category);
    return SUCCESS;
}

BluetoothAndNearlinkPreferredRecordCategory AudioCoreService::GetPreferBluetoothAndNearlinkRecordByUid(int32_t uid)
{
    return AudioRouterInfra::GetInstance().GetPreferredInputCategory(uid);
}

void AudioCoreService::NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value)
{
    AUDIO_INFO_LOG("device<%{public}s> condition:%{public}s value:%{public}s",
        GetEncryptStr(networkId).c_str(), condition.c_str(), value.c_str());

    vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    vector<SinkInput> targetSinkInputs = {};
    for (auto sinkInput : sinkInputs) {
        if (sinkInput.sinkName == networkId) {
            targetSinkInputs.push_back(sinkInput);
        }
    }
    AUDIO_DEBUG_LOG("move [%{public}zu] of all [%{public}zu]sink-inputs to local.",
        targetSinkInputs.size(), sinkInputs.size());
    std::shared_ptr<AudioDeviceDescriptor> localDevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN_LOG(localDevice != nullptr, "Device error: null device.");
    localDevice->networkId_ = LOCAL_NETWORK_ID;
    localDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    localDevice->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;

    int32_t ret;
    AudioDeviceDescriptor curOutputDeviceDesc = audioRouterSelectStrategy_.Get1stCurrentOutputDevice();
    if (localDevice->deviceType_ != curOutputDeviceDesc.deviceType_) {
        AUDIO_WARNING_LOG("device[%{public}d] not active, use device[%{public}d] instead.",
            static_cast<int32_t>(localDevice->deviceType_), static_cast<int32_t>(curOutputDeviceDesc.deviceType_));
        ret = audioDeviceCommon_.MoveToLocalOutputDevice(targetSinkInputs,
            std::make_shared<AudioDeviceDescriptor>(curOutputDeviceDesc));
    } else {
        ret = audioDeviceCommon_.MoveToLocalOutputDevice(targetSinkInputs, localDevice);
    }
    CHECK_AND_CALL_FUNC_RETURN_LOG(ret == SUCCESS,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "MoveToLocalOutputDevice failed", false),
        "MoveToLocalOutputDevice failed!");

    // Suspend device, notify audio stream manager that device has been changed.
    ret = audioPolicyManager_.SuspendAudioDevice(networkId, true);
    CHECK_AND_CALL_FUNC_RETURN_LOG(ret == SUCCESS,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED, "SuspendAudioDevice failed", false),
        "SuspendAudioDevice failed!");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {};
    desc.push_back(localDevice);
    UpdateTrackerDeviceChange(desc);
    audioDeviceCommon_.OnPreferredOutputDeviceUpdated(curOutputDeviceDesc,
        AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);
    AUDIO_DEBUG_LOG("Success");
}

void AudioCoreService::CloseWakeUpAudioCapturer()
{
    AUDIO_INFO_LOG("close wakeup audio capturer start");
    audioIOHandleMap_.ClosePortAndEraseIOHandle(std::string(PRIMARY_WAKEUP));
}

int32_t AudioCoreService::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    FetchOutputDeviceAndRoute("TriggerFetchDevice", reason);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredOutputDeviceUpdated();
    }
    FetchInputDeviceAndRoute("TriggerFetchDevice", reason);

    // update a2dp offload
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    ReloadSourceForDeviceChange(audioRouterSelectStrategy_.Get1stCurrentInputDevice(),
        audioRouterSelectStrategy_.Get1stCurrentOutputDevice(), "TriggerFetchDevice");
    return SUCCESS;
}

// No lock
int32_t AudioCoreService::SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object)
{
    return deviceStatusListener_->SetAudioDeviceAnahsCallback(object);
}

int32_t AudioCoreService::UnsetAudioDeviceAnahsCallback()
{
    return deviceStatusListener_->UnsetAudioDeviceAnahsCallback();
}

void AudioCoreService::OnUpdateAnahsSupport(std::string anahsShowType)
{
    AUDIO_INFO_LOG("OnUpdateAnahsSupport show type: %{public}s", anahsShowType.c_str());
    thread th([this](string &&anahsShowType) {
        for (int32_t i = 0; i < MAX_TRY; ++i) {
            if (i > 0) {
                this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
            }
            if (deviceStatusListener_) {
                deviceStatusListener_->UpdateAnahsPlatformType(anahsShowType);
                return;
            }
        }
        AUDIO_ERR_LOG("Try UpdateAnahsPlatformType over %{public}d times, failed", MAX_TRY);
    }, move(anahsShowType));
    pthread_setname_np(th.native_handle(), "OS_ANAHS_TYP");
    th.detach();
}

void AudioCoreService::RegisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::RegisterDeviceObserver(deviceStatusListener_->deviceObserver_);
    if (isBtListenerRegistered) {
        AUDIO_INFO_LOG("audio policy service already register bt listerer, return");
        return;
    }
    if (!isBtCrashed) {
        Bluetooth::AudioA2dpManager::RegisterBluetoothA2dpListener();
        Bluetooth::AudioHfpManager::RegisterBluetoothScoListener();
    }
    isBtListenerRegistered = true;
    isBtCrashed = false;
    RegisterBluetoothDeathCallback();
    AudioPolicyUtils::GetInstance().SetBtConnecting(true);
    Bluetooth::AudioA2dpManager::CheckA2dpDeviceReconnect();
    Bluetooth::AudioHfpManager::CheckHfpDeviceReconnect();
    AudioPolicyUtils::GetInstance().SetBtConnecting(false);
#endif
}

void AudioCoreService::UnregisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::UnregisterDeviceObserver();
    Bluetooth::AudioA2dpManager::UnregisterBluetoothA2dpListener();
    Bluetooth::AudioHfpManager::UnregisterBluetoothScoListener();
    isBtListenerRegistered = false;
#endif
}

void AudioCoreService::ConfigDistributedRoutingRole(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    AUDIO_INFO_LOG("[ADeviceEvent] device %{public}d, cast type %{public}d",
        (descriptor != nullptr) ? descriptor->deviceType_ : -1, type);
    StoreDistributedRoutingRoleInfo(descriptor, type);
    FetchDeviceAndRoute("ConfigDistributedRoutingRole", AudioStreamDeviceChangeReason::OVERRODE);
}


int32_t AudioCoreService::SetRingerMode(AudioRingerMode ringMode)
{
    int32_t result = audioPolicyManager_.SetRingerMode(ringMode);
    CHECK_AND_CALL_FUNC_RETURN_RET(result == SUCCESS, result,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(IPCSkeleton::GetCallingUid(),
            ERR_SET_VOLUME_FAILED, "SetRingerMode failed", false));
    if (Util::IsRingerAudioScene(audioSceneManager_.GetAudioScene(true))) {
        AUDIO_INFO_LOG("[ADeviceEvent] fetch output device after switch new ringmode");
        FetchOutputDeviceAndRoute("SetRingerMode");
        ReloadSourceForDeviceChange(audioRouterSelectStrategy_.Get1stCurrentInputDevice(),
            audioRouterSelectStrategy_.Get1stCurrentOutputDevice(), "SetRingerMode");
    }
    Volume vol = {false, 1.0f, 0};
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    vol.isMute = (ringMode == RINGER_MODE_NORMAL) ? false : true;
    vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevel(STREAM_RING));
    vol.volumeFloat = GetSystemVolumeInDb(STREAM_RING, vol.volumeInt, curOutputDeviceType);
    audioVolumeManager_.SetSharedVolume(STREAM_RING, curOutputDeviceType, vol);
    return result;
}

bool AudioCoreService::IsNoRunningStream(std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs)
{
    for (auto streamDesc : outputStreamDescs) {
        if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
            return false;
        }
    }
    return true;
}

int32_t AudioCoreService::FetchOutputDeviceAndRoute(std::string caller, const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs = pipeManager_->GetAllOutputStreamDescs();
    HILOG_COMM_INFO("[DeviceFetchStart] by %{public}s for %{public}zu output streams, in devices %{public}s",
        caller.c_str(), outputStreamDescs.size(), audioDeviceManager_.GetConnDevicesStr().c_str());

    if (outputStreamDescs.empty() && !pipeManager_->IsModemCommunicationIdExist() && !CheckRingAndVoipStreamRunning()) {
        audioActiveDevice_.UpdateStreamDeviceMap("NoStreamInPipe");
        CheckAndUpdateHearingAidCall(DEVICE_TYPE_NONE);
        return HandleFetchOutputWhenNoRunningStream(reason);
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    CheckModemScene(modemDescs, reason);
    CheckRingAndVoipScene(reason);

    AudioCoreServiceUtils::SortOutputStreamDescsForUsage(outputStreamDescs);
    for (auto &streamDesc : outputStreamDescs) {
        UpdateStreamDevicesForStart(streamDesc, caller + "FetchOutputDeviceAndRoute");
    }
    // todo fix
    HandleA2dpSuspendWhenFetch(reason, audioRouterSelectStrategy_.Get1stCurrentOutputDevice(), outputStreamDescs);
    // this will update volume device map
    audioActiveDevice_.UpdateStreamDeviceMap("FetchOutputDeviceAndRoute");
    // here will update volume must after UpdateStreamDeviceMap
    UpdateActiveDeviceAndVolumeBeforeMoveSession(outputStreamDescs, reason);

    int32_t ret = FetchRendererPipesAndExecute(outputStreamDescs, reason);
    UpdateModemRoute(modemDescs);
    if (IsNoRunningStream(outputStreamDescs)) {
        HandleFetchOutputWhenNoRunningStream(reason);
    }
    return ret;
}

bool AudioCoreService::HandleA2dpSuspendWhenLoad()
{
    if (a2dpNeedSuspend_.load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> lock(a2dpSuspendMutex_);
        if (a2dpNeedSuspend_) {
            AUDIO_INFO_LOG("keep suspend a2dp");
            AudioServerProxy::GetInstance().SuspendRenderSinkProxy("a2dp");
            return true;
        }
    }

    return false;
}

void AudioCoreService::HandleA2dpRestore()
{
    std::lock_guard<std::mutex> lock(a2dpSuspendMutex_);

    if (!a2dpNeedSuspend_) {
        return;
    }

    if (std::chrono::steady_clock::now() < a2dpSuspendUntil_) {
        return;
    }

    a2dpNeedSuspend_ = false;
    AUDIO_INFO_LOG("restore a2dp");
    if (!audioDeviceManager_.GetScoState()) {
        AudioServerProxy::GetInstance().RestoreRenderSinkProxy("a2dp");
    }
}

int32_t AudioCoreService::FetchInputDeviceAndRoute(std::string caller, const AudioStreamDeviceChangeReasonExt reason)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> inputStreamDescs = pipeManager_->GetAllInputStreamDescs();
    AUDIO_INFO_LOG("[DeviceFetchStart] by %{public}s for %{public}zu input streams, in devices %{public}s",
        caller.c_str(), inputStreamDescs.size(), audioDeviceManager_.GetConnDevicesStr().c_str());

    if (inputStreamDescs.empty()) {
        return HandleFetchInputWhenNoRunningStream();
    }

    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    for (auto streamDesc : inputStreamDescs) {
        streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
        pipeManager_->UpdateNewDeviceDesc(streamDesc, {});
        RouterType routerType = ROUTER_TYPE_NONE;
        std::shared_ptr<AudioDeviceDescriptor> inputDeviceDesc =
            audioRouterCenter_.FetchInputDevice(streamDesc->capturerInfo_.sourceType, GetRealUid(streamDesc),
                routerType, streamDesc->sessionId_);
        CHECK_AND_RETURN_RET_LOG(inputDeviceDesc != nullptr, ERR_INVALID_PARAM, "inputDeviceDesc is nullptr");
        pipeManager_->UpdateNewDeviceDesc(streamDesc, {inputDeviceDesc});
        AUDIO_INFO_LOG("[DeviceFetchInfo] device %{public}s for stream %{public}d with status %{public}u",
            streamDesc->GetNewDevicesTypeString().c_str(), streamDesc->sessionId_, streamDesc->streamStatus_);

        UpdateRecordStreamInfo(streamDesc);
        if (!HandleInputStreamInRunning(streamDesc)) {
            continue;
        }

        // handle nearlink
        int32_t inputRet = ActivateInputDevice(streamDesc, reason);
        CHECK_AND_RETURN_RET_LOG(inputRet == SUCCESS, inputRet, "Activate input device failed");
#ifdef INPUT_FEATURE_INDEPENDENT_MODE
        isUpdateActiveDevice = UpdateInputDevice(inputDeviceDesc, GetRealUid(streamDesc));
        if (isUpdateActiveDevice) {
            OnPreferredInputDeviceUpdated(
                audioActiveDevice_.GetCurrentInputDeviceType(GetRealUid(streamDesc)), "", reason);
        }
    }
#else
        if (needUpdateActiveDevice) {
            isUpdateActiveDevice = UpdateInputDevice(inputDeviceDesc, GetRealUid(streamDesc));
            needUpdateActiveDevice = false;
        }
    }
    if (isUpdateActiveDevice) {
        // networkId is not used.
        OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(), "", reason);
    }
#endif
    return FetchCapturerPipesAndExecute(inputStreamDescs);
}

void AudioCoreService::SetAudioServerProxy()
{
    AUDIO_INFO_LOG("SetAudioServerProxy Start");
    const sptr<IStandardAudioService> gsp = AudioServerProxy::GetInstance().GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "SetAudioServerProxy, Audio Server Proxy is null");
    audioPolicyManager_.SetAudioServerProxy(gsp);
}

DirectPlaybackMode AudioCoreService::GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo,
    const StreamUsage &streamUsage)
{
    FetchDeviceInfo info = { streamUsage, getuid(), "GetDirectPlaybackSupport" };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioRouterCenter_.FetchOutputDevices(info);
    CHECK_AND_RETURN_RET_LOG(!descs.empty(), DIRECT_PLAYBACK_NOT_SUPPORTED, "find output device failed");
    return pipeManager_->GetDirectPlaybackSupport(descs.front(), streamInfo);
}

#ifdef BLUETOOTH_ENABLE
void AudioCoreService::RegisterBluetoothDeathCallback()
{
    lock_guard<mutex> lock(g_btProxyMutex);
    AUDIO_INFO_LOG("Enter");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(samgr != nullptr,
        "get sa manager failed");
    sptr<IRemoteObject> object = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    CHECK_AND_RETURN_LOG(object != nullptr,
        "get audio service remote object failed");
    // register death recipent
    sptr<AudioServerDeathRecipient> asDeathRecipient =
        new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
    if (asDeathRecipient != nullptr) {
        asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) {
            BluetoothServiceCrashedCallback(pid, uid);
        });
        bool result = object->AddDeathRecipient(asDeathRecipient);
        if (!result) {
            AUDIO_ERR_LOG("failed to add deathRecipient");
        }
    }
}

void AudioCoreService::BluetoothServiceCrashedCallback(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("Bluetooth sa crashed, will restore proxy in next call");
    lock_guard<mutex> lock(g_btProxyMutex);
    isBtListenerRegistered = false;
    isBtCrashed = true;
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSink();
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSource();
    Bluetooth::AudioHfpManager::DisconnectBluetoothHfpSink();
}
#endif

void AudioCoreService::UpdateStreamPropInfo(const std::string &adapterName, const std::string &pipeName,
    const std::list<DeviceStreamInfo> &deviceStreamInfo, const std::list<std::string> &supportDevices)
{
    pipeManager_->UpdateStreamPropInfo(adapterName, pipeName, deviceStreamInfo, supportDevices);
}

void AudioCoreService::ClearStreamPropInfo(const std::string &adapterName, const std::string &pipeName)
{
    pipeManager_->ClearStreamPropInfo(adapterName, pipeName);
}

uint32_t AudioCoreService::GetStreamPropInfoSize(const std::string &adapterName, const std::string &pipeName)
{
    return pipeManager_->GetStreamPropInfoSize(adapterName, pipeName);
}

int32_t AudioCoreService::CaptureConcurrentCheck(uint32_t sessionId)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "streamDesc is null");
    if (streamDesc->audioMode_ != AUDIO_MODE_RECORD) {
        return ERR_NOT_SUPPORTED;
    }

    streamDesc->stateStartTimeStamp_ = ClockTime::GetCurNano();
    auto dfxResult = std::make_unique<struct ConcurrentCaptureDfxResult>();
    if (!WriteCapturerConcurrentMsg(streamDesc, dfxResult)) {
        return ERR_INVALID_HANDLE;
    }
    LogCapturerConcurrentResult(dfxResult);
    WriteCapturerConcurrentEvent(dfxResult);
    return SUCCESS;
}

int32_t AudioCoreService::SetCapturerMuteHint(uint32_t sessionId, bool mute)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> currentCapturerInfos;
    streamCollector_.GetCurrentCapturerChangeInfos(currentCapturerInfos);
    audioEffectService_.PruneInvalidAppMuteHintState(currentCapturerInfos);

    auto iter = std::find_if(currentCapturerInfos.begin(), currentCapturerInfos.end(),
        [sessionId](const std::shared_ptr<AudioCapturerChangeInfo> &capturerInfo) {
            return capturerInfo != nullptr && capturerInfo->sessionId == static_cast<int32_t>(sessionId);
        });
    CHECK_AND_RETURN_RET_LOG(iter != currentCapturerInfos.end(), ERR_INVALID_PARAM,
        "sessionId %{public}u not found", sessionId);
    CHECK_AND_RETURN_RET_LOG((*iter)->capturerState == CAPTURER_RUNNING, ERROR_ILLEGAL_STATE,
        "sessionId %{public}u is not running", sessionId);

    audioEffectService_.SetAppConfiguredCapturerMuteHint(sessionId, mute);
    audioEffectService_.EvaluateVoipBypassByAppMuteHint(currentCapturerInfos);
    return SUCCESS;
}

void AudioCoreService::ReEvaluateVoipBypassByAppMuteHint()
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> currentCapturerInfos;
    streamCollector_.GetCurrentCapturerChangeInfos(currentCapturerInfos);
    audioEffectService_.PruneInvalidAppMuteHintState(currentCapturerInfos);
    audioEffectService_.EvaluateVoipBypassByAppMuteHint(currentCapturerInfos);
}

bool AudioCoreService::IsVoipPrivacyMuteCandidateSource(SourceType sourceType) const
{
    bool isTarget = VOIP_PRIVACY_MUTE_TARGET_SOURCES.count(sourceType) > 0;
    return isTarget;
}

bool AudioCoreService::IsVoipPrivacyBypassedByBehavior(const std::shared_ptr<AudioStreamDescriptor> &streamDesc) const
{
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    bool enabled = streamDesc->voipNoPrivacyFlag_.load();
    if (enabled) {
    }
    return enabled;
}

bool AudioCoreService::IsVoipPrivacyMuteBypassedForUltrasonic(SourceType sourceType) const
{
    AudioInterruptCustom interruptCustom;
    bool isExempt = interruptCustom.IsUltrasonicConcurrentScene(SOURCE_TYPE_VOICE_COMMUNICATION, sourceType);
    if (isExempt) {
        return true;
    }
    return false;
}

bool AudioCoreService::ShouldMuteCapturerByVoipPrivacy(
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    bool hasActivePrivacyVoipCapturer) const
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "VoipPrivacy streamDesc is nullptr");
    uint32_t sessionId = streamDesc->GetSessionId();
    SourceType sourceType = streamDesc->capturerInfo_.sourceType;
    if (!IsVoipPrivacyMuteCandidateSource(sourceType)) {
        return false;
    }
    if (!hasActivePrivacyVoipCapturer) {
        return false;
    }
    if (IsVoipPrivacyMuteBypassedForUltrasonic(sourceType)) {
        return false;
    }
    return true;
}

void AudioCoreService::WriteVoipMutedCaptureStats(
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool mute) const
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    std::string appName = streamDesc->bundleName_;
    if (appName.empty()) {
        appName = AudioBundleManager::GetBundleNameFromUid(GetRealUid(streamDesc));
    }
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "MUTED_CAPTURE_STATS",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "APP_NAME", appName,
        "STREAM_USAGE", static_cast<uint8_t>(streamDesc->capturerInfo_.sourceType),
        "MUTE", mute);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Write event fail: MUTED_CAPTURE_STATS, ret:%{public}d", ret);
    }
}

bool AudioCoreService::UpdateVoipPrivacyMuteForSession(
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool mute)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false,
        "streamDesc is nullptr");
    uint32_t sessionId = streamDesc->GetSessionId();
    int32_t ret = audioPolicyManager_.SetSourceOutputStreamMuteByStreamId(static_cast<int32_t>(sessionId), mute);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false,
        "SetSourceOutputStreamMuteByStreamId failed, sessionId:%{public}u "
        "mute:%{public}d, ret:%{public}d",
        sessionId, mute, ret);
    WriteVoipMutedCaptureStats(streamDesc, mute);
    return true;
}

void AudioCoreService::EvaluateActivePrivacyVoipPresence(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &allCapturerStreamDescs,
    bool &hasActivePrivacyVoipCapturer) const
{
    hasActivePrivacyVoipCapturer = false;
    for (const auto &streamDesc : allCapturerStreamDescs) {
        if (streamDesc == nullptr || !streamDesc->IsRunning()) {
            continue;
        }
        uint32_t sessionId = streamDesc->GetSessionId();
        bool voipMuted = streamDesc->voipPriorityLowered_.load();
        if (streamDesc->capturerInfo_.sourceType != SOURCE_TYPE_VOICE_COMMUNICATION || voipMuted) {
            continue;
        }
        if (IsVoipPrivacyBypassedByBehavior(streamDesc)) {
            continue;
        }
        hasActivePrivacyVoipCapturer = true;
    }
}

void AudioCoreService::CollectSessionsToMuteByVoipPrivacy(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &allCapturerStreamDescs,
    bool hasActivePrivacyVoipCapturer,
    std::unordered_set<uint32_t> &targetMuteSessions) const
{
    targetMuteSessions.clear();
    for (const auto &streamDesc : allCapturerStreamDescs) {
        if (streamDesc == nullptr || !streamDesc->IsRunning()) {
            continue;
        }

        uint32_t sessionId = streamDesc->GetSessionId();
        if (ShouldMuteCapturerByVoipPrivacy(streamDesc, hasActivePrivacyVoipCapturer)) {
            targetMuteSessions.insert(sessionId);
            continue;
        }
    }
}

void AudioCoreService::SyncVoipPrivacyMuteState(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &allCapturerStreamDescs,
    const std::unordered_set<uint32_t> &targetMuteSessions)
{
    for (const auto &streamDesc : allCapturerStreamDescs) {
        if (streamDesc == nullptr) {
            continue;
        }
        uint32_t sessionId = streamDesc->GetSessionId();
        bool targetMute = targetMuteSessions.count(sessionId) > 0;
        bool currentMute = streamDesc->voipPrivacyMuted_.load();
        if (targetMute == currentMute) {
            continue;
        }
        if (UpdateVoipPrivacyMuteForSession(streamDesc, targetMute)) {
            streamDesc->voipPrivacyMuted_.store(targetMute);
            AUDIO_INFO_LOG("update sessionId:%{public}u mute:%{public}d",
                sessionId, targetMute);
        }
    }
}

void AudioCoreService::ReEvaluateVoipPrivacyMuteForCapturers(const std::string &reason)
{
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr,
        "pipeManager_ is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> allCapturerStreamDescs =
        pipeManager_->GetAllCapturerStreamDescs();
    AUDIO_INFO_LOG("reason:%{public}s descCount:%{public}zu",
        reason.c_str(), allCapturerStreamDescs.size());

    bool hasActivePrivacyVoipCapturer = false;
    EvaluateActivePrivacyVoipPresence(allCapturerStreamDescs, hasActivePrivacyVoipCapturer);

    std::unordered_set<uint32_t> targetMuteSessions;
    CollectSessionsToMuteByVoipPrivacy(allCapturerStreamDescs, hasActivePrivacyVoipCapturer,
        targetMuteSessions);
    SyncVoipPrivacyMuteState(allCapturerStreamDescs, targetMuteSessions);
}

void AudioCoreService::SetFirstScreenOn()
{
    isFirstScreenOn_ = true;
}

bool AudioCoreService::IsA2dpOffloadStream(uint sessionId)
{
    auto streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "can't find sessionId: %{public}d", sessionId);
    return streamDesc->IsA2dpOffloadStream();
}

int32_t AudioCoreService::SetRendererTarget(RenderTarget target, RenderTarget lastTarget, uint32_t sessionId)
{
    int32_t ret = ERROR;
    if (lastTarget == NORMAL_PLAYBACK && target == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
        ret = PlayBackToInjection(sessionId);
        if (ret == SUCCESS) {
            AudioInjectorPolicy::GetInstance().AddInjectorStreamId(sessionId);
        }
    } else if (lastTarget == INJECT_TO_VOICE_COMMUNICATION_CAPTURE && target == NORMAL_PLAYBACK) {
        ret = InjectionToPlayBack(sessionId);
        if (ret == SUCCESS) {
            AudioInjectorPolicy::GetInstance().DeleteInjectorStreamId(sessionId);
        }
    }
    return ret;
}

int32_t AudioCoreService::StartInjection(uint32_t streamId)
{
    bool isConnected = audioInjectorPolicy_.GetIsConnected();
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERR_NULL_POINTER, "Injector::pipeManager_ is null");
    if (!isConnected && pipeManager_->IsCaptureVoipCall() == NO_VOIP) {
        return ERR_ILLEGAL_STATE;
    }
    int32_t ret = ERROR;
    ret = audioInjectorPolicy_.AddCaptureInjector();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Injector::AddCaptureInjector failed");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(streamId);
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERROR, "Injector::get streamDesc failed");
    streamDesc->rendererTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    ret = FetchOutputDeviceAndRoute("OnForcedDeviceSelected",
        AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Injector::move stream in failed");
    audioInjectorPolicy_.AddStreamDescriptor(streamId, streamDesc);
    return SUCCESS;
}

void AudioCoreService::RemoveIdForInjector(uint32_t streamId)
{
    audioInjectorPolicy_.RemoveStreamDescriptor(streamId);
}

void AudioCoreService::ReleaseCaptureInjector()
{
    audioInjectorPolicy_.ReleaseCaptureInjector();
}

void AudioCoreService::RebuildCaptureInjector(uint32_t streamId)
{
    audioInjectorPolicy_.RebuildCaptureInjector(streamId);
}

int32_t AudioCoreService::A2dpOffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize,
                                                       uint32_t &timeStamp)
{
    Trace trace("AudioCoreService::A2dpOffloadGetRenderPosition");
#ifdef BLUETOOTH_ENABLE
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    AUDIO_DEBUG_LOG("GetRenderPosition, deviceType: %{public}d, a2dpOffloadFlag: %{public}d",
        audioA2dpOffloadFlag_.GetA2dpOffloadFlag(), curOutputDeviceType);
    int32_t ret = SUCCESS;
    if (curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP &&
        audioActiveDevice_.GetCurrentOutputDeviceNetworkId() == LOCAL_NETWORK_ID &&
        audioA2dpOffloadFlag_.GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
        ret = Bluetooth::AudioA2dpManager::GetRenderPosition(delayValue, sendDataSize, timeStamp);
    } else {
        delayValue = 0;
        sendDataSize = 0;
        timeStamp = 0;
    }
    return ret;
#else
    return SUCCESS;
#endif
}

void AudioCoreService::RestoreDistributedDeviceInfo()
{
    AUDIO_INFO_LOG("try to restore distributed device");
    CHECK_AND_RETURN_LOG(deviceStatusListener_ != nullptr, "deviceStatusListener_ is nullptr");

    std::vector<Media::MediaMonitor::MonitorDmDeviceInfo> dmDeviceInfos;
    Media::MediaMonitor::MediaMonitorManager::GetInstance().GetDmDeviceInfo(dmDeviceInfos);
    for (const auto &dmDeviceInfo : dmDeviceInfos) {
        DmDevice dmDev;
        dmDev.deviceName_ = dmDeviceInfo.deviceName_;
        dmDev.networkId_ = dmDeviceInfo.networkId_;
        dmDev.dmDeviceType_ = dmDeviceInfo.dmDeviceType_;
        AudioConnectedDevice::GetInstance().UpdateDmDeviceMap(std::move(dmDev), true);
    }

    std::vector<std::string> deviceInfos;
    Media::MediaMonitor::MediaMonitorManager::GetInstance().GetDistributedDeviceInfo(deviceInfos);
    CHECK_AND_RETURN_LOG(!deviceInfos.empty(), "no distributed device info");

    for (const auto &deviceInfo : deviceInfos) {
        deviceStatusListener_->SendDistributeInfo(deviceInfo);
    }
}

bool AudioCoreService::IsDistributeServiceOnline()
{
    CHECK_AND_RETURN_RET_LOG(deviceStatusListener_ != nullptr, false, "deviceStatusListener_ is null");
    return deviceStatusListener_->IsDistributeServiceOnline();
}

bool AudioCoreService::InVideoCommFastBlockList(const std::string& bundleName)
{
    CHECK_AND_RETURN_RET_LOG(queryBundleNameListCallback_ != nullptr, false, "queryBundleNameListCallback_ is null");
    bool isBundleNameExist = false;
    queryBundleNameListCallback_->OnQueryBundleNameIsInList(bundleName, CHECK_VIDEO_COMM_SELECTION,
        isBundleNameExist);
    return isBundleNameExist;
}
int32_t AudioCoreService::SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object)
{
    queryBundleNameListCallback_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_RET_LOG(queryBundleNameListCallback_ != nullptr, ERR_CALLBACK_NOT_REGISTERED,
        "Query bundle name list callback is null");
    return SUCCESS;
}

void AudioCoreService::OnCheckActiveMusicTime(const std::string &reason)
{
    AudioVolumeManager::GetInstance().OnCheckActiveMusicTime(reason);
}

void AudioCoreService::HandleDeviceConfigChanged(const std::shared_ptr<AudioDeviceDescriptor>
    &selectedAudioDevice)
{
    CHECK_AND_RETURN_LOG(selectedAudioDevice != nullptr, "selectedAudioDevice is nullptr");
    std::shared_ptr<AudioDeviceDescriptor> device = selectedAudioDevice;
    if (audioDeviceManager_.ExistsByTypeAndAddress(DEVICE_TYPE_NEARLINK, device->macAddress_)) {
        FetchOutputDeviceAndRoute("HandleDeviceConfigChanged");
    }
}

void AudioCoreService::DeactivateRemoteDevice(const std::string &networkId, DeviceType deviceType)
{
    CHECK_AND_RETURN(networkId != LOCAL_NETWORK_ID);
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId,
        AudioPolicyUtils::GetInstance().GetDeviceRole(deviceType));
    audioPolicyManager_.StopAudioPort(moduleName);
}

void AudioCoreService::NotifyRemoteRouteStateChange(const std::string &networkId, DeviceType deviceType, bool enable)
{
    CHECK_AND_RETURN(networkId != LOCAL_NETWORK_ID);
    std::shared_ptr<AudioDeviceDescriptor> desc = audioConnectedDevice_.GetConnectedDeviceByType(networkId,
        deviceType);
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");

    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    auto preferredMediaDevice = strategy.GetMediaOutputDevice(INVALID_UID, INVALID_STREAM_ID);
    CHECK_AND_RETURN_LOG(preferredMediaDevice != nullptr, "preferredMediaDevice is nullptr");
    if (!enable && (preferredMediaDevice->dmDeviceType_ == DM_DEVICE_TYPE_WIFI_SOUNDBOX)) {
        AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER,
            std::make_shared<AudioDeviceDescriptor>(), INVALID_UID, "NotifyRemoteRouteStateChange");
    }

    desc->connectState_ = enable ? CONNECTED : VIRTUAL_CONNECTED;
    OnDeviceInfoUpdated(*desc, CONNECTSTATE_UPDATE);
    CHECK_AND_RETURN(!enable);
    DeactivateRemoteDevice(networkId, deviceType);
}

void AudioCoreService::NotifyRemoteDeviceStatusUpdate(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is nullptr");
    CHECK_AND_RETURN(desc->networkId_ != LOCAL_NETWORK_ID);
    audioActiveDevice_.NotifyUserDisSelectionEventToRemote(desc);
    desc->connectState_ = VIRTUAL_CONNECTED;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    audioDeviceManager_.UpdateDevicesListInfo(desc, CONNECTSTATE_UPDATE, reason);
    DeactivateRemoteDevice(desc->networkId_, desc->deviceType_);
}

int32_t AudioCoreService::FetchAndActivateOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    int32_t ret = FetchOutputDeviceAndRoute("StartClient");
    JUDGE_AND_WARNING_LOG(ret != SUCCESS, "FetchOutputDeviceAndRoute failed");
    int32_t outputRet = ActivateOutputDevice(streamDesc);
    CHECK_AND_CALL_FUNC_RETURN_RET(outputRet != REFETCH_DEVICE, SUCCESS,
        HILOG_COMM_ERROR("[FetchAndActivateOutputDevice]Activate output device failed, refetch device"));
    CHECK_AND_CALL_FUNC_RETURN_RET(outputRet == SUCCESS, outputRet,
        HILOG_COMM_ERROR("[FetchAndActivateOutputDevice]Activate output device failed"));
    return SUCCESS;
}

bool AudioCoreService::CheckStaticModeAndSelectFlag(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (streamDesc->rendererInfo_.isStatic) {
        if (streamDesc->rendererInfo_.originalFlag == AUDIO_FLAG_MMAP) {
            streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_FAST;
        } else {
            streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        }
        return true;
    }
    return false;
}

uint32_t ConvertToHDIAudioInputType(SourceType sourceType)
{
    if (FWKTYPE_TO_HDITYPE_MAP.find(sourceType) != FWKTYPE_TO_HDITYPE_MAP.end()) {
        auto iter = FWKTYPE_TO_HDITYPE_MAP.find(sourceType);
        return static_cast<uint32_t>(iter->second);
    }

    return static_cast<uint32_t>(AUDIO_INPUT_MIC_TYPE);
}

bool AudioCoreService::IsHigherPrioritySourceType(SourceType newSource, SourceType currentSource)
{
    AUDIO_INFO_LOG("newSource sourceType:%{public}d currentSource sourceType:%{public}d", newSource, currentSource);

    if (!isEcFeatureEnable_ &&
        (ConvertToHDIAudioInputType(newSource) == ConvertToHDIAudioInputType(currentSource))) {
        return false;
    }

    auto newIter = NORMAL_SOURCETYPE_PRIORITY.find(newSource);
    auto currIter = NORMAL_SOURCETYPE_PRIORITY.find(currentSource);
    if (newIter == NORMAL_SOURCETYPE_PRIORITY.end() || currIter == NORMAL_SOURCETYPE_PRIORITY.end() ||
        newSource == currentSource) {
        return false;
    }

    return newIter->second >= currIter->second;
}

std::pair<SourceType, uint32_t> AudioCoreService::GetTargetSessionForEc()
{
    uint32_t targetSessionId = GetOpenedNormalSourceSessionId();
    SourceType targetSourceType = GetSourceOpened();
    AudioStreamDescriptor runningStream = {};
    bool hasRunningSession = FindRunningNormalSession(targetSessionId, runningStream);
    if (hasRunningSession &&
        IsHigherPrioritySourceType(runningStream.capturerInfo_.sourceType, targetSourceType)) {
        targetSessionId = runningStream.sessionId_;
        targetSourceType = runningStream.capturerInfo_.sourceType;
    }

    AUDIO_INFO_LOG("session for EC sourceType:%{public}d, sessionId:%{public}u",
        targetSourceType, targetSessionId);
    return std::make_pair(targetSourceType, targetSessionId);
}

void AudioCoreService::LoadInnerCapturerSink(std::string moduleName, AudioStreamInfo streamInfo)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    AUDIO_INFO_LOG("Start");
    uint32_t bufferSize = streamInfo.samplingRate *
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(streamInfo.format) *
        streamInfo.channels * RENDER_FRAME_INTERVAL_IN_SECONDS;

    AudioModuleInfo moduleInfo = {};
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    moduleInfo.format = AudioPolicyUtils::GetInstance().ConvertToHDIAudioFormat(streamInfo.format);
    moduleInfo.name = moduleName;
    moduleInfo.networkId = "LocalDevice";
    moduleInfo.channels = std::to_string(streamInfo.channels);
    moduleInfo.rate = std::to_string(streamInfo.samplingRate);
    moduleInfo.bufferSize = std::to_string(bufferSize);

    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
#endif
}

void AudioCoreService::UnloadInnerCapturerSink(std::string moduleName)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleName);
#endif
}

void AudioCoreService::HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    AUDIO_INFO_LOG("Is connected: %{public}d", isConnected);
    AudioDeviceDescriptor updatedDesc = AudioDeviceDescriptor(
        DEVICE_TYPE_REMOTE_CAST, AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_REMOTE_CAST));
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb = {};
    if (isConnected) {
        // If device already in list, remove it else do not modify the list
        audioConnectedDevice_.DelConnectedDevice(updatedDesc.networkId_, updatedDesc.deviceType_,
            updatedDesc.macAddress_);
        audioDeviceCommon_.UpdateConnectedDevicesWhenConnecting(updatedDesc, descForCb);
        LoadInnerCapturerSink(REMOTE_CAST_INNER_CAPTURER_SINK_NAME, streamInfo);
    } else {
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
        FetchOutputDeviceAndRoute("HandleRemoteCastDevice_1",
            AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE_EXT);
        UnloadInnerCapturerSink(REMOTE_CAST_INNER_CAPTURER_SINK_NAME);
    }
    // remove device from global when device has been added to audio zone in superlanch-dual
    int32_t res  = AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(
        audioConnectedDevice_.GetConnectedDeviceByType(LOCAL_NETWORK_ID, DEVICE_TYPE_REMOTE_CAST));
    if (res == SUCCESS) {
        AUDIO_INFO_LOG("Enable remotecast device for audio zone, remove from global list");
        audioDeviceCommon_.UpdateConnectedDevicesWhenDisconnecting(updatedDesc, descForCb);
    }
    FetchOutputDeviceAndRoute("HandleRemoteCastDevice_2");
    FetchInputDeviceAndRoute("HandleRemoteCastDevice_2");

    // update a2dp offload
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    }
#endif
}

bool AudioCoreService::GetTargetSessionIdForInputPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    uint32_t originSessionId, uint32_t &targetSessionId, SessionOperation operation)
{
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, false, "pipe is null");
    AudioStreamDescriptor maxRunningDesc = {};
    AudioStreamDescriptor maxRemainingDesc = {};
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    CHECK_AND_RETURN_RET_LOG(sourceStrategyMap != nullptr, false, "sourceStrategyMap is null");

    uint32_t maxRunningPriority = GetMaxPriorityForInputPipe(pipeInfo, originSessionId, maxRunningDesc, true);
    uint32_t maxRemainingPriority = GetMaxPriorityForInputPipe(pipeInfo, originSessionId, maxRemainingDesc, false);
    auto maxRunningSource = maxRunningDesc.capturerInfo_.sourceType;
    auto maxRemainingSource = maxRemainingDesc.capturerInfo_.sourceType;
    auto openSource = pipeInfo->moduleInfo_.sourceType;
    bool hasRunningExpectOrigin = (maxRunningSource != SOURCE_TYPE_INVALID) ? true : false;
    AUDIO_INFO_LOG("openSource:%{public}s, maxRunningDesc:<%{public}u,%{public}d>,"
        "maxRemainingDesc:<%{public}u,%{public}d>", openSource.c_str(), maxRunningDesc.sessionId_,
        maxRunningSource, maxRemainingDesc.sessionId_, maxRemainingSource);

    auto targetSource = (hasRunningExpectOrigin) ? maxRunningSource : maxRemainingSource;
    if ((operation == SESSION_OPERATION_RELEASE) && (targetSource != SOURCE_TYPE_INVALID)
        && (openSource != std::to_string(targetSource))) {
        targetSessionId = (hasRunningExpectOrigin) ? maxRunningDesc.sessionId_ : maxRemainingDesc.sessionId_;
        return true;
    }

    CHECK_AND_RETURN_RET_LOG(pipeInfo->streamDescMap_.count(originSessionId) > 0, false, "can not find stream on pipe");
    std::shared_ptr<AudioStreamDescriptor> originDescPtr = pipeInfo->streamDescMap_[originSessionId];
    auto originSource = originDescPtr->capturerInfo_.sourceType;
    auto originStreategy = sourceStrategyMap->find(originSource);
    CHECK_AND_RETURN_RET_LOG(originStreategy != sourceStrategyMap->end(), false, "can not find originStreategy");
    bool originHigher = originStreategy->second.priority >= maxRunningPriority;
    AUDIO_INFO_LOG("originStreamDesc:<%{public}u, %{public}d> priority:%{public}u",
        originSessionId, originSource, originStreategy->second.priority);

    if ((operation == SESSION_OPERATION_START) && ((hasRunningExpectOrigin && originHigher)
        || (!hasRunningExpectOrigin && openSource != std::to_string(originSource)))) {
        targetSessionId = originSessionId;
        return true;
    }
    if ((operation == SESSION_OPERATION_PAUSE || operation == SESSION_OPERATION_STOP)
        && (hasRunningExpectOrigin && openSource == std::to_string(originSource))) {
        targetSessionId = maxRunningDesc.sessionId_;
        return true;
    }
    return false;
}

uint32_t AudioCoreService::GetMaxPriorityForInputPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    uint32_t sessionId, AudioStreamDescriptor &maxPriorityDesc, bool onlyRunning)
{
    uint32_t maxPriority = 0;
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, maxPriority, "pipe is null");
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    CHECK_AND_RETURN_RET_LOG(sourceStrategyMap != nullptr, maxPriority, "sourceStrategyMap is null");

    for (const auto &stream : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE(stream != nullptr && stream->sessionId_ != sessionId);

        auto strategyIt = sourceStrategyMap->find(stream->capturerInfo_.sourceType);
        CHECK_AND_CONTINUE(strategyIt != sourceStrategyMap->end() &&
            stream->audioFlag_ == strategyIt->second.audioFlag);
        if (onlyRunning) {
            if ((stream->streamStatus_ == STREAM_STATUS_STARTED)
                && (strategyIt->second.priority >= maxPriority)) {
                maxPriority = strategyIt->second.priority;
                stream->CopyToStruct(maxPriorityDesc);
            }
        } else {
            if (strategyIt->second.priority >= maxPriority) {
                maxPriority = strategyIt->second.priority;
                stream->CopyToStruct(maxPriorityDesc);
            }
        }
    }
    return maxPriority;
}

bool AudioCoreService::IsVirtualAudioRecognitionSession(uint32_t sessionId)
{
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = pipeManager_->GetPipeList();
    std::shared_ptr<AudioPipeInfo> pipe = pipeManager_->FindPipeBySessionId(pipeList, sessionId);
    std::shared_ptr<AudioStreamDescriptor> stream = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET_LOG(pipe != nullptr, false, "pipe is nullptr");

    bool isVARecognitionSession =
        pipe->adapterName_ == ADAPTER_TYPE_VA && stream->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION;
    AUDIO_INFO_LOG("sessionID %{public}d isVARecognitionSession: %{public}s",
        sessionId, isVARecognitionSession ? "true" : "false");
    return isVARecognitionSession;
}

int32_t AudioCoreService::ReloadCapturerSessionForInputPipe(uint32_t sessionId, SessionOperation operation)
{
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = pipeManager_->GetPipeList();
    std::shared_ptr<AudioPipeInfo> pipeInfo = nullptr;
    if (operation != SESSION_OPERATION_RELEASE) {
        pipeInfo = pipeManager_->FindPipeBySessionId(pipeList, sessionId);
    } else {
        pipeInfo = AudioPipeManager::GetPipeManager()->GetPipeinfoByNameAndFlag(
            "primary", sessionWithInputPipeRouteFlag_[sessionId]);
    }
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, ERROR, "pipe is null");

    AUDIO_INFO_LOG("reload input pipe:%{public}s flag:%{public}u Id:%{public}u with opt:%{public}d",
        pipeInfo->name_.c_str(), pipeInfo->routeFlag_, sessionId, operation);

    uint32_t targetSessionId = sessionId;
    bool needReload = GetTargetSessionIdForInputPipe(pipeInfo, sessionId, targetSessionId, operation);
    CHECK_AND_RETURN_RET_LOG(needReload, ERROR, "need not reload");
    return ReloadSourceForInputPipe(pipeInfo, targetSessionId);
}

bool AudioCoreService::FindRemainingNormalSession(uint32_t sessionId, bool findRunningSessionRet,
    uint32_t runningSessionId, uint32_t &targetSessionId)
{
    bool hasRemainSession = false;
    targetSessionId = runningSessionId;
    CHECK_AND_RETURN_RET(!findRunningSessionRet, true);
    SourceType targetSourceType = SOURCE_TYPE_INVALID;

    const std::vector<std::shared_ptr<AudioPipeInfo>> pipes = pipeManager_->GetPipeList();
    for (const auto &pipe : pipes) {
        if (pipe == nullptr || pipe->pipeRole_ != PIPE_ROLE_INPUT || pipe->adapterName_ == ADAPTER_TYPE_VA) {
            continue;
        }

        for (const auto &stream : pipe->streamDescriptors_) {
            if (stream == nullptr || stream->sessionId_ == sessionId) {
                continue;
            }

            if (!specialSourceTypeSet_.count(stream->capturerInfo_.sourceType)) {
                continue;
            }
            hasRemainSession = true;
            CHECK_AND_CONTINUE(IsHigherPrioritySourceType(stream->capturerInfo_.sourceType, targetSourceType));
            targetSessionId = stream->sessionId_;
        }
    }
    AUDIO_INFO_LOG("hasRemainSession:%{public}d, targetSessionId:%{public}u", hasRemainSession, targetSessionId);
    return hasRemainSession;
}

bool AudioCoreService::IsPipeInSourceStrategyMap(std::shared_ptr<AudioPipeInfo> pipeInfo, uint64_t sessionId)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    for (auto tmpStreamDesc : pipeInfo->streamDescriptors_) {
        CHECK_AND_CONTINUE(tmpStreamDesc->sessionId_ == sessionId);
        streamDesc = tmpStreamDesc;
    }
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, false, "streamDesc is nullptr");

    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    CHECK_AND_RETURN_RET_LOG(sourceStrategyMap != nullptr, false, "sourceStrategyMap is nullptr");

    auto strategyIt = sourceStrategyMap->find(streamDesc->capturerInfo_.sourceType);
    CHECK_AND_RETURN_RET(strategyIt != sourceStrategyMap->end(), false);
    return true;
}

bool AudioCoreService::ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo,
    AudioModuleInfo &audioModuleInfo)
{
    if (!pipeManager_->GetAdapterInfoFlag()) {
        return false;
    }

    std::shared_ptr<PolicyAdapterInfo> info;
    AudioAdapterType type = static_cast<AudioAdapterType>(AudioPolicyUtils::portStrToEnum[std::string(PRIMARY_WAKEUP)]);
    bool ret = pipeManager_->GetAdapterInfoByType(type, info);
    if (!ret) {
        AUDIO_ERR_LOG("can not find adapter info");
        return false;
    }

    std::shared_ptr<AdapterPipeInfo> pipeInfo = info->GetPipeInfoByName(PIPE_WAKEUP_INPUT);
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("wakeup pipe info is nullptr");
        return false;
    }

    if (!FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo)) {
        AUDIO_ERR_LOG("failed to fill pipe stream prop info");
        return false;
    }

    audioModuleInfo.adapterName = info->adapterName;
    audioModuleInfo.name = pipeInfo->paProp_.moduleName_;
    audioModuleInfo.lib = pipeInfo->paProp_.lib_;
    audioModuleInfo.role = "source";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.className = "primary";
    audioModuleInfo.fileName = "";
    audioModuleInfo.OpenMicSpeaker = "1";
    audioModuleInfo.sourceType = std::to_string(SourceType::SOURCE_TYPE_WAKEUP);

    AUDIO_INFO_LOG("wakeup auido module info, adapter name:%{public}s, name:%{public}s, lib:%{public}s",
        audioModuleInfo.adapterName.c_str(), audioModuleInfo.name.c_str(), audioModuleInfo.lib.c_str());
    return true;
}

bool AudioCoreService::FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo,
    std::shared_ptr<AdapterPipeInfo> pipeInfo, AudioModuleInfo &audioModuleInfo)
{
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("wakeup pipe info is nullptr");
        return false;
    }

    if (pipeInfo->streamPropInfos_.size() == 0) {
        AUDIO_ERR_LOG("no stream prop info");
        return false;
    }
    auto targetIt = *pipeInfo->streamPropInfos_.begin();
    for (auto it : pipeInfo->streamPropInfos_) {
        if (it -> channels_ == static_cast<uint32_t>(streamInfo.channels)) {
            targetIt = it;
            break;
        }
    }

    audioModuleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetIt->format_];
    audioModuleInfo.channels = std::to_string(targetIt->channels_);
    audioModuleInfo.rate = std::to_string(targetIt->sampleRate_);
    audioModuleInfo.bufferSize =  std::to_string(targetIt->bufferSize_);

    AUDIO_INFO_LOG("stream prop info, format:%{public}s, channels:%{public}s, rate:%{public}s, buffer size:%{public}s",
        audioModuleInfo.format.c_str(), audioModuleInfo.channels.c_str(),
        audioModuleInfo.rate.c_str(), audioModuleInfo.bufferSize.c_str());
    return true;
}

int32_t AudioCoreService::SetWakeUpAudioCapturer(InternalAudioCapturerOptions options)
{
    AUDIO_INFO_LOG("set wakeup audio capturer start");
    AudioModuleInfo moduleInfo = {};
    if (!ConstructWakeupAudioModuleInfo(options.streamInfo, moduleInfo)) {
        AUDIO_ERR_LOG("failed to construct wakeup audio module info");
        return ERROR;
    }
    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);

    AUDIO_DEBUG_LOG("set wakeup audio capturer end");
    return SUCCESS;
}

int32_t AudioCoreService::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    InternalAudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo = config.streamInfo;
    return SetWakeUpAudioCapturer(capturerOptions);
}

bool AudioCoreService::HandleIndependentInputpipe(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
    uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession)
{
    for (const auto &pipe : pipeList) {
        if (pipe && pipe->pipeRole_ == PIPE_ROLE_INPUT && pipe->routeFlag_ == AUDIO_INPUT_FLAG_AI) {
            AUDIO_INFO_LOG("In Independent pipe");
            return CompareIndependentxmlPriority(pipe, sessionId, runningSessionInfo, hasSession);
        }
    }
    return hasSession;
}

bool AudioCoreService::CompareIndependentxmlPriority(const std::shared_ptr<AudioPipeInfo> &pipe,
    uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession)
{
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    if (sourceStrategyMap == nullptr) {
        return false;
    }

    uint32_t maxPriority = 0;
    for (const auto &stream : pipe->streamDescriptors_) {
        if (stream == nullptr || stream->sessionId_ == sessionId || stream->streamStatus_ != STREAM_STATUS_STARTED) {
            continue;
        }

        auto strategyIt = sourceStrategyMap->find(stream->capturerInfo_.sourceType);
        if (strategyIt == sourceStrategyMap->end()) {
            continue;
        }

        if (strategyIt->second.priority > maxPriority) {
            maxPriority = strategyIt->second.priority;
            stream->CopyToStruct(runningSessionInfo);
            hasSession = true;
        }
    }
    AUDIO_INFO_LOG("Independent find ret: %{public}d, session: %{public}d, sourceType: %{public}d",
        static_cast<int32_t>(hasSession), runningSessionInfo.sessionId_, runningSessionInfo.capturerInfo_.sourceType);
    return hasSession;
}

bool AudioCoreService::HandleNormalInputPipes(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
    uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession)
{
    AUDIO_INFO_LOG("normal input");
    for (const auto &pipe : pipeList) {
        if (pipe == nullptr) {
            AUDIO_WARNING_LOG("pipe is nullptr.");
            continue;
        }
        if (pipe->pipeRole_ != PIPE_ROLE_INPUT || pipe->adapterName_ == ADAPTER_TYPE_VA) {
            AUDIO_WARNING_LOG("pip role is not input, or pipe adapter is not va.");
            continue;
        }

        uint32_t flagMask = AUDIO_INPUT_FLAG_AI | AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP_FAST |
            AUDIO_INPUT_FLAG_WAKEUP | AUDIO_INPUT_FLAG_UNPROCESS | AUDIO_INPUT_FLAG_ULTRASONIC |
            AUDIO_INPUT_FLAG_VOICE_RECOGNITION | AUDIO_INPUT_FLAG_RAW_AI | AUDIO_INPUT_FLAG_INTERPHONE |
            AUDIO_INPUT_FLAG_LIVE;

        if ((pipe->routeFlag_ & flagMask) != 0) {
            continue;
        }

        for (const auto &stream : pipe->streamDescriptors_) {
            if (stream == nullptr || stream->sessionId_ == sessionId ||
                !(stream->streamStatus_ == STREAM_STATUS_STARTED &&
                  specialSourceTypeSet_.count(stream->capturerInfo_.sourceType) == 0)) {
                continue;
            }

            // Check voipPriorityLowered flag: if true, treat VoIP stream as non-running
            // so it won't be counted in priority comparison
            if (stream->voipPriorityLowered_.load() &&
                stream->GetSourceType() == SOURCE_TYPE_VOICE_COMMUNICATION) {
                AUDIO_INFO_LOG("VoIP stream %{public}u is excluded from priority comparison due to mute callback",
                    stream->GetSessionId());
                continue;
            }

            if (IsHigherPrioritySourceType(
                stream->capturerInfo_.sourceType,
                runningSessionInfo.capturerInfo_.sourceType)) {
                hasSession = true;
                stream->CopyToStruct(runningSessionInfo);
            }
        }
    }
    AUDIO_INFO_LOG("find ret: %{public}d, session: %{public}d, sourceType: %{public}d",
        static_cast<int32_t>(hasSession), runningSessionInfo.sessionId_, runningSessionInfo.capturerInfo_.sourceType);
    return hasSession;
}

bool AudioCoreService::FindRunningNormalSession(uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo)
{
    bool hasSession = false;
    SourceType tmpSource = SOURCE_TYPE_INVALID;

    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = pipeManager_->GetPipeList();
    std::shared_ptr<AudioPipeInfo> incommingPipe = pipeManager_->FindPipeBySessionId(pipeList, sessionId);
    if (incommingPipe == nullptr) {
        return false;
    }

    AUDIO_INFO_LOG("incommingPipe: %{public}s", incommingPipe->name_.c_str());
    if (incommingPipe->pipeRole_ != PIPE_ROLE_INPUT) {
        return false;
    }

    if (incommingPipe->routeFlag_ == AUDIO_INPUT_FLAG_AI) {
        return HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    }

    return HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
}

int32_t AudioCoreService::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    AUDIO_INFO_LOG("sessionID: %{public}" PRIu64 " source: %{public}d", sessionID, sessionInfo.sourceType);
    CHECK_AND_RETURN_RET_LOG(isPolicyConfigParsered_ && audioVolumeManager_.GetLoadFlag(), ERROR,
        "policyConfig not loaded");

    if (capturerSessionIdisRemovedSet_.count(sessionID) > 0) {
        capturerSessionIdisRemovedSet_.erase(sessionID);
        AUDIO_INFO_LOG("sessionID: %{public}" PRIu64 " had already been removed earlier", sessionID);
        return SUCCESS;
    }

    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = pipeManager_->GetPipeList();
    std::shared_ptr<AudioPipeInfo> pipeInfo = pipeManager_->FindPipeBySessionId(pipeList, sessionID);
    if (pipeInfo != nullptr && IsPipeInSourceStrategyMap(pipeInfo, sessionID)) {
        AUDIO_WARNING_LOG("pipe:%{public}s routeFlag:%{public}u need not add",
            pipeInfo->name_.c_str(), pipeInfo->routeFlag_);
            sessionWithInputPipeRouteFlag_[sessionID] = pipeInfo->routeFlag_;
            return SUCCESS;
    }
    sessionSourceTypeMap_[sessionID] = sessionInfo.sourceType;
    if (specialSourceTypeSet_.count(sessionInfo.sourceType) == 0) {
        if (GetSourceOpened() == SOURCE_TYPE_INVALID) {
            // normal source is not opened before -- it should not be happen!!
            AUDIO_WARNING_LOG("Record route should not be opened here!");
            return SUCCESS;
        }
    } else if (sessionInfo.sourceType == SOURCE_TYPE_REMOTE_CAST) {
        HandleRemoteCastDevice(true, streamInfo);
    }
    return SUCCESS;
}

SessionInfo ConstructSessionByStream(AudioStreamDescriptor &desc)
{
    SessionInfo sessionInfo;
    sessionInfo.sourceType = desc.capturerInfo_.sourceType;
    sessionInfo.rate = desc.streamInfo_.samplingRate;
    sessionInfo.channels = desc.streamInfo_.channels;
    return sessionInfo;
}

SessionInfo ConstructSessionByStream(std::shared_ptr<AudioStreamDescriptor> descPtr)
{
    SessionInfo sessionInfo;
    sessionInfo.sourceType = descPtr->capturerInfo_.sourceType;
    sessionInfo.rate = descPtr->streamInfo_.samplingRate;
    sessionInfo.channels = descPtr->streamInfo_.channels;
    return sessionInfo;
}

int32_t AudioCoreService::ReloadCaptureSessionSoftLink()
{
    hearingAidReloadFlag_ = false;
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipes = pipeManager_->GetPipeList();
    CHECK_AND_RETURN_RET_LOG(!pipes.empty(), ERR_INVALID_OPERATION, "pipes invalid");
    AudioStreamDescriptor targetStream = {};
    bool hasSession = false;
    hasSession = HandleNormalInputPipes(pipes, SESSION_ID_INVALID, targetStream, hasSession);

    CHECK_AND_RETURN_RET_LOG(hasSession, SUCCESS, "no need to reload session");
    AUDIO_INFO_LOG("start reload session: %{public}u", targetStream.sessionId_);

    SessionInfo targetSession = ConstructSessionByStream(targetStream);
    ReloadSourceForSession(targetSession, targetStream.sessionId_);
    SetOpenedNormalSourceSessionId(targetStream.sessionId_);
    return SUCCESS;
}

void AudioCoreService::NotifyVoipPriorityLoweredByMute(uint32_t streamId, bool lowered)
{
    AUDIO_INFO_LOG("NotifyVoipPriorityLoweredByMute: streamId %{public}u, lowered %{public}d",
        streamId, lowered);
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager_ is nullptr");

    auto streamDesc = pipeManager_->GetStreamDescById(streamId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "stream descriptor is nullptr for streamId %{public}u", streamId);

    streamDesc->voipPriorityLowered_.store(lowered);
    TriggerVoipPriorityReload(streamDesc, lowered);
}

bool AudioCoreService::UpdateVoipNoPrivacyFlagBySessionId(uint32_t sessionId, bool enabled)
{
    AUDIO_INFO_LOG("UpdateVoipNoPrivacyFlagBySessionId sessionId:%{public}u enabled:%{public}d",
        sessionId, enabled);
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, false, "pipeManager_ is nullptr");
    auto streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET(streamDesc != nullptr, false);
    CHECK_AND_RETURN_RET(streamDesc->IsRecording(), false);
    CHECK_AND_RETURN_RET(streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION, false);
    CHECK_AND_RETURN_RET(streamDesc->voipNoPrivacyFlag_.load() != enabled, false);
    streamDesc->voipNoPrivacyFlag_.store(enabled);
    return true;
}

bool AudioCoreService::UpdateVoipNoPrivacyFlagByPid(int32_t callerPid, bool enabled)
{
    AUDIO_INFO_LOG("UpdateVoipNoPrivacyFlagByPid callerPid:%{public}d enabled:%{public}d",
        callerPid, enabled);
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, false, "pipeManager_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(callerPid > 0, false, "callerPid is invalid");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> allCapturerStreamDescs =
        pipeManager_->GetAllCapturerStreamDescs();
    size_t updateCount = 0;
    for (const auto &streamDesc : allCapturerStreamDescs) {
        if (streamDesc == nullptr) {
            continue;
        }
        int32_t streamCallerPid = streamDesc->callerPid_ > 0 ? streamDesc->callerPid_ : streamDesc->appInfo_.appPid;
        if (streamCallerPid != callerPid) {
            continue;
        }
        if (streamDesc->capturerInfo_.sourceType != SOURCE_TYPE_VOICE_COMMUNICATION ||
            streamDesc->voipNoPrivacyFlag_.load() == enabled) {
            continue;
        }
        streamDesc->voipNoPrivacyFlag_.store(enabled);
        ++updateCount;
    }
    AUDIO_INFO_LOG("UpdateVoipNoPrivacyFlagByPid done callerPid:%{public}d updateCount:%{public}zu",
        callerPid, updateCount);
    return updateCount > 0;
}

void AudioCoreService::TriggerVoipPriorityReload(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool lowered)
{
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    AUDIO_INFO_LOG("TriggerVoipPriorityReload: sessionId %{public}u, lowered %{public}d",
        streamDesc->GetSessionId(), lowered);
    if (!streamDesc->IsRunning()) {
        AUDIO_INFO_LOG("streamId %{public}u is not started, skip reload", streamDesc->GetSessionId());
        return;
    }

    SessionOperation operation = lowered ? SESSION_OPERATION_PAUSE : SESSION_OPERATION_START;
    (void)ReloadCaptureSession(streamDesc->GetSessionId(), operation);
}

bool AudioCoreService::HasNormalTypeCapturerSession(
    const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList)
{
    auto sourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    if (sourceStrategyMap == nullptr) {
        sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    }
    for (const auto &pipe : pipeList) {
        if (pipe == nullptr) {
            continue;
        }

        for (const auto &stream : pipe->streamDescriptors_) {
            CHECK_AND_RETURN_RET(stream == nullptr || !stream->IsRecording() ||
                specialSourceTypeSet_.count(stream->capturerInfo_.sourceType) > 0 ||
                sourceStrategyMap->count(stream->capturerInfo_.sourceType) > 0, true);
        }
    }
    return false;
}

void AudioCoreService::OnCapturerSessionRemoved(uint64_t sessionID)
{
    AUDIO_INFO_LOG("sessionid:%{public}" PRIu64, sessionID);
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = pipeManager_->GetPipeList();
    auto it = sessionSourceTypeMap_.find(sessionID);
    if (it != sessionSourceTypeMap_.end()) {
        SourceType sourceType = it->second;
        sessionSourceTypeMap_.erase(it);
        if (specialSourceTypeSet_.count(sourceType) > 0) {
            if (sourceType == SOURCE_TYPE_REMOTE_CAST) {
                HandleRemoteCastDevice(false);
            }
            return;
        } else {
            if (sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
                ResetAudioEcInfo();
            }
            CHECK_AND_RETURN_LOG(!HasNormalTypeCapturerSession(pipeList), "Has normal type capturer session");
            CloseNormalSource();
            return;
        }
    }
    if (sessionWithInputPipeRouteFlag_.count(sessionID) > 0) {
        sessionWithInputPipeRouteFlag_.erase(sessionID);
        return;
    }
    AUDIO_INFO_LOG("Sessionid:%{public}" PRIu64 " not added, directly placed into capturerSessionIdisRemovedSet_",
                   sessionID);
    capturerSessionIdisRemovedSet_.insert(sessionID);
}

bool AudioCoreService::IsSourceTypeValidForEc(SourceType sourceType)
{
    return sourceType == SOURCE_TYPE_VOICE_COMMUNICATION || sourceType == SOURCE_TYPE_MIC;
}

bool AudioCoreService::IsVoipDeviceChanged(const AudioDeviceDescriptor &inputDevice,
    const AudioDeviceDescriptor &outputDevice)
{
    AudioDeviceDescriptor realInputDevice = inputDevice;
    AudioDeviceDescriptor realOutputDevice = outputDevice;
    RouterType routerType = ROUTER_TYPE_NONE;
    shared_ptr<AudioDeviceDescriptor> inputDesc =
        audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_VOICE_COMMUNICATION, -1, routerType);
    if (inputDesc != nullptr) {
        realInputDevice = *inputDesc;
    }
    FetchDeviceInfo info = { STREAM_USAGE_VOICE_COMMUNICATION, "IsVoipDeviceChanged" };
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDesc =
        audioRouterCenter_.FetchOutputDevices(info);
    if (outputDesc.size() > 0 && outputDesc.front() != nullptr) {
        realOutputDevice = *outputDesc.front();
    }
    if (!inputDevice.IsSameDeviceDesc(realInputDevice) || !outputDevice.IsSameDeviceDesc(realOutputDevice)) {
        AUDIO_INFO_LOG("target device is not ready, so ignore reload");
        return false;
    }
    AudioEcInfo lastEcInfo = GetAudioEcInfo();
    AUDIO_INFO_LOG("curInDevice: %{public}d, curOutDevice: %{public}d", lastEcInfo.inputDevice.deviceType_,
        lastEcInfo.outputDevice.deviceType_);
    if (!lastEcInfo.inputDevice.IsSameDeviceDesc(realInputDevice) ||
        !lastEcInfo.outputDevice.IsSameDeviceDesc(realOutputDevice)) {
        return true;
    }
    return false;
}

int32_t AudioCoreService::ReloadCaptureSoftLink(std::shared_ptr<AudioPipeInfo> &pipeInfo,
    const AudioModuleInfo &moduleInfo)
{
    int32_t ret = ReloadSourceSoftLink(pipeInfo, moduleInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "reload softLink failed");
    hearingAidReloadFlag_ = true;
    return SUCCESS;
}

int32_t AudioCoreService::ReloadCaptureSession(uint32_t sessionId, SessionOperation operation)
{
    std::string adapterName = GetAdapterNameBySessionId(sessionId);
    AUDIO_INFO_LOG("prepare reload session: %{public}u at adapter: %{public}s with operation: %{public}d",
        sessionId, adapterName.c_str(), operation);
    CHECK_AND_RETURN_RET_LOG(!hearingAidReloadFlag_, SUCCESS, "no need to reload session for hearingAid");
    CHECK_AND_RETURN_RET_LOG(
        adapterName.empty() || adapterName == ADAPTER_TYPE_PRIMARY, SUCCESS, "Skip reload for non-primary adapter.");
    if (sessionWithInputPipeRouteFlag_.count(sessionId) != 0) {
        return ReloadCapturerSessionForInputPipe(sessionId, operation);
    }
    uint32_t targetSessionId = sessionId;
    std::shared_ptr<AudioStreamDescriptor> newStreamDesc = pipeManager_->GetStreamDescById(sessionId);
    if (newStreamDesc == nullptr || specialSourceTypeSet_.count(newStreamDesc->capturerInfo_.sourceType) != 0) {
        AUDIO_ERR_LOG("The session of %{public}u is special or no stream, no need reload.", sessionId);
        return ERROR;
    }

    SessionInfo targetSession = ConstructSessionByStream(newStreamDesc);
    AudioStreamDescriptor runningStreamDesc = {};
    bool findRunningSessionRet = FindRunningNormalSession(targetSessionId, runningStreamDesc);
    CHECK_AND_RETURN_RET_LOG(!(runningStreamDesc.capturerInfo_.sourceType == SOURCE_TYPE_MIC &&
                               IsVirtualAudioRecognitionSession(sessionId)),
                             ERROR, "skipping reload: va recognition stream detected");

    bool needReload = false;
    switch (operation) {
        case SESSION_OPERATION_START:
            needReload = ReloadCheckForStart(findRunningSessionRet, newStreamDesc, runningStreamDesc);
            break;
        case SESSION_OPERATION_PAUSE:
        case SESSION_OPERATION_STOP:
            if (findRunningSessionRet && (newStreamDesc->capturerInfo_.sourceType == GetSourceOpened())) {
                needReload = true;
                targetSession = ConstructSessionByStream(runningStreamDesc);
            }
            break;
        case SESSION_OPERATION_RELEASE:
            CHECK_AND_BREAK_LOG((newStreamDesc->capturerInfo_.sourceType == GetSourceOpened()) &&
                FindRemainingNormalSession(sessionId, findRunningSessionRet,
                runningStreamDesc.sessionId_, targetSessionId), "no remain stream.");
            needReload = true;
            break;
        default:
            AUDIO_ERR_LOG("operation parameter error!");
            break;
    }

    CHECK_AND_RETURN_RET_LOG(needReload, ERROR, "no need to reload session");
    AUDIO_INFO_LOG("start reload session: %{public}u", targetSessionId);
    ReloadSourceForSession(targetSession, targetSessionId);
    SetOpenedNormalSourceSessionId(targetSessionId);
    return SUCCESS;
}

bool AudioCoreService::ReloadCheckForStart(
    bool findRunningSessionRet, std::shared_ptr<AudioStreamDescriptor> newStream, AudioStreamDescriptor &runningStream)
{
    if (findRunningSessionRet && IsHigherPrioritySourceType(
        newStream->capturerInfo_.sourceType, runningStream.capturerInfo_.sourceType)) {
        return true;
    } else if (!findRunningSessionRet && (GetSourceOpened() != newStream->capturerInfo_.sourceType)) {
        return true;
    }
    return false;
}

void AudioCoreService::ReloadSourceForDeviceChange(const AudioDeviceDescriptor &inputDevice,
    const AudioDeviceDescriptor &outputDevice, const std::string &caller)
{
    AUDIO_DEBUG_LOG("form caller: %{public}s, inDevice: %{public}d, outDevice: %{public}d", caller.c_str(),
        inputDevice.deviceType_, outputDevice.deviceType_);
    if (!isEcFeatureEnable_) {
        AUDIO_DEBUG_LOG("reload ignore for feature not enable");
        return;
    }

    std::pair<SourceType, uint32_t> targetSessionInfo = GetTargetSessionForEc();
    CHECK_AND_RETURN_LOG(IsSourceTypeValidForEc(targetSessionInfo.first),
        "reload ignore for source not voip or mic");

    if (targetSessionInfo.first == SOURCE_TYPE_VOICE_COMMUNICATION) {
        if (!IsVoipDeviceChanged(inputDevice, outputDevice)) {
            AUDIO_INFO_LOG("voip reload ignore for device not change");
            return;
        }
    } else {
        if (inputDevice.deviceType_ != DEVICE_TYPE_DEFAULT &&
            inputDeviceForReload_.deviceType_ == DEVICE_TYPE_DEFAULT) {
            inputDeviceForReload_ = inputDevice;
            AUDIO_INFO_LOG("mic source reload ignore for inputDeviceForReload_ not update");
            return;
        }
        if (inputDevice.deviceType_ == DEVICE_TYPE_DEFAULT ||
            inputDevice.IsSameDeviceDesc(inputDeviceForReload_)) {
            AUDIO_INFO_LOG("mic source reload ignore for device not changed");
            return;
        }
    }

    // reload for device change, used session is not changed
    CHECK_AND_RETURN_LOG(specialSourceTypeSet_.count(targetSessionInfo.first) == 0,
        "target session: %{public}u not found", targetSessionInfo.second);
    inputDeviceForReload_ = inputDevice;
    AUDIO_INFO_LOG("start reload session: %{public}u for device change", targetSessionInfo.second);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(targetSessionInfo.second);
    if (streamDesc == nullptr) {
        return;
    }
    SessionInfo targetSession = ConstructSessionByStream(streamDesc);
    ReloadSourceForSession(targetSession, targetSessionInfo.second);
    SetOpenedNormalSourceSessionId(targetSessionInfo.second);
}

std::string AudioCoreService::GetEnhancePropByName(const AudioEffectPropertyArray &propertyArray,
    const std::string &propName)
{
    std::string propValue = "";
    auto iter = std::find_if(propertyArray.property.begin(), propertyArray.property.end(),
        [&propName](const AudioEffectProperty &prop) {
            return prop.name == propName;
        });
    if (iter != propertyArray.property.end()) {
        propValue = iter->category;
    }
    return propValue;
}

void AudioCoreService::ReloadSourceForEffect(const AudioEffectPropertyArray &oldPropertyArray,
    const AudioEffectPropertyArray &newPropertyArray)
{
    if (!isMicRefFeatureEnable_) {
        AUDIO_INFO_LOG("reload ignore for feature not enable");
        return;
    }
    std::pair<SourceType, uint32_t> targetSessionInfo = GetTargetSessionForEc();
    CHECK_AND_RETURN_LOG(IsSourceTypeValidForEc(targetSessionInfo.first),
        "reload ignore for source not voip or mic");

    std::string oldRecordProp = GetEnhancePropByName(oldPropertyArray, "record");
    std::string oldVoipUpProp = GetEnhancePropByName(oldPropertyArray, "voip_up");
    std::string newRecordProp = GetEnhancePropByName(newPropertyArray, "record");
    std::string newVoipUpProp = GetEnhancePropByName(newPropertyArray, "voip_up");
    if ((!newVoipUpProp.empty() && ((oldVoipUpProp == "PNR") ^ (newVoipUpProp == "PNR"))) ||
        (!newRecordProp.empty() && oldRecordProp != newRecordProp)) {
        CHECK_AND_RETURN_LOG(specialSourceTypeSet_.count(targetSessionInfo.first) == 0,
            "target sessionId: %{public}u not found", targetSessionInfo.second);
        AUDIO_INFO_LOG("start reload sessionId: %{public}u for effect change", targetSessionInfo.second);
        std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(targetSessionInfo.second);
        if (streamDesc == nullptr) {
            return;
        }
        SessionInfo targetSession = ConstructSessionByStream(streamDesc);
        ReloadSourceForSession(targetSession, targetSessionInfo.second);
        SetOpenedNormalSourceSessionId(targetSessionInfo.second);
    }
}

void AudioCoreService::UpdateEnhanceEffectState(SourceType source)
{
    AudioEffectPropertyArray effectPropertyArray = {};
    RouterType routerType = ROUTER_TYPE_NONE;
    std::shared_ptr<AudioDeviceDescriptor> inputDesc = audioRouterCenter_.FetchInputDevice(source, -1, routerType);
    CHECK_AND_RETURN_LOG(inputDesc != nullptr, "inputDesc is nullptr");
    int32_t ret = AudioServerProxy::GetInstance().GetAudioEffectPropertyProxy(effectPropertyArray,
        inputDesc->deviceType_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("get enhance property fail, ret: %{public}d", ret);
        return;
    }
    std::string recordProp = "";
    std::string voipUpProp = "";
    for (const auto &prop : effectPropertyArray.property) {
        if (prop.name == "record") {
            recordProp = prop.category;
        }
        if (prop.name == "voip_up") {
            voipUpProp = prop.category ;
        }
    }
    isMicRefRecordOn_ = (recordProp == "NRON");
    isMicRefVoipUpOn_ = (voipUpProp == "PNR");

    AUDIO_INFO_LOG("ecEnableState: %{public}d, micRefEnableState: %{public}d, "
        "isMicRefRecordOn_: %{public}d, isMicRefVoipUp: %{public}d",
        isEcFeatureEnable_, isMicRefFeatureEnable_, isMicRefRecordOn_, isMicRefVoipUpOn_);
}

void AudioCoreService::CloseNormalSource()
{
    AUDIO_INFO_LOG("close all sources");
    audioIOHandleMap_.ClosePortAndEraseIOHandle(BLUETOOTH_MIC);
    audioIOHandleMap_.ClosePortAndEraseIOHandle(PRIMARY_MIC);

    audioIOHandleMap_.ClosePortAndEraseIOHandle(VIRTUAL_AUDIO);
    if (isEcFeatureEnable_) {
        audioIOHandleMap_.ClosePortAndEraseIOHandle(USB_MIC);
    }
    normalSourceOpened_ = SOURCE_TYPE_INVALID;
}

void AudioCoreService::GetTargetSourceTypeAndMatchingFlag(SourceType source,
    SourceType &targetSource, bool &useMatchingPropInfo)
{
    switch (source) {
        case SOURCE_TYPE_VOICE_RECOGNITION:
            targetSource = SOURCE_TYPE_VOICE_RECOGNITION;
            useMatchingPropInfo = true;
            break;
        case SOURCE_TYPE_VOICE_COMMUNICATION:
        case SOURCE_TYPE_VOICE_TRANSCRIPTION:
            targetSource = SOURCE_TYPE_VOICE_COMMUNICATION;
            useMatchingPropInfo = isEcFeatureEnable_ ? false : true;
            break;
        case SOURCE_TYPE_VOICE_CALL:
            targetSource = SOURCE_TYPE_VOICE_CALL;
            break;
        case SOURCE_TYPE_CAMCORDER:
            targetSource = SOURCE_TYPE_CAMCORDER;
            break;
        case SOURCE_TYPE_UNPROCESSED:
            targetSource = SOURCE_TYPE_UNPROCESSED;
            useMatchingPropInfo = true;
            break;
        case SOURCE_TYPE_LIVE:
            targetSource = SOURCE_TYPE_LIVE;
            break;
        default:
            targetSource = SOURCE_TYPE_MIC;
            break;
    }
}

int32_t AudioCoreService::FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo, PipeStreamPropInfo &targetInfo,
    SourceType &targetSourceType)
{
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = nullptr;
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    bool ret = pipeManager_->GetAdapterInfoByType(AudioAdapterType::TYPE_PRIMARY, adapterInfo);
    if (ret) {
        pipeInfoPtr = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    }
    CHECK_AND_RETURN_RET_LOG(pipeInfoPtr != nullptr, ERROR, "pipeInfoPtr is null");

    const auto &streamPropInfoList = pipeInfoPtr->streamPropInfos_;
    if (streamPropInfoList.empty()) {
        AUDIO_ERR_LOG("supportedRate or supportedChannels is empty");
        return ERROR;
    }

    bool useMatchingPropInfo = false;
    RouterType routerType = ROUTER_TYPE_NONE;
    GetTargetSourceTypeAndMatchingFlag(sessionInfo.sourceType, targetSourceType, useMatchingPropInfo);
    shared_ptr<AudioDeviceDescriptor> inputDesc =
        audioRouterCenter_.FetchInputDevice(targetSourceType, -1, routerType);
    CHECK_AND_RETURN_RET_LOG(inputDesc != nullptr, ERROR, "inputDesc is null");
    std::shared_ptr<PipeStreamPropInfo> targetStreamPropInfo =
        AudioCoreConfigManager::GetInstance().GetAvailableStreamPropInfoForDevice(
            streamPropInfoList, inputDesc->deviceType_);
    CHECK_AND_RETURN_RET_LOG(targetStreamPropInfo != nullptr, ERROR, "targetStreamPropInfo is null");
    if (useMatchingPropInfo) {
        for (const auto &streamPropInfo : streamPropInfoList) {
            if (sessionInfo.channels == streamPropInfo->channels_ &&
                sessionInfo.rate == streamPropInfo->sampleRate_) {
                targetStreamPropInfo = streamPropInfo;
                break;
            }
        }
    }
    targetInfo = *targetStreamPropInfo;
    uint32_t targetBufferMs = CalculateTargetBufferDurationMs(targetInfo);

    if (isEcFeatureEnable_) {
        if (inputDesc != nullptr && inputDesc->deviceType_ != DEVICE_TYPE_MIC &&
            targetInfo.channels_ == PC_MIC_CHANNEL_NUM) {
            // only built-in mic can use 4 channel, update later by using xml to describe
            targetInfo.channels_ = static_cast<AudioChannel>(HEADPHONE_CHANNEL_NUM);
            targetInfo.channelLayout_ = CH_LAYOUT_STEREO;
        }
    }

#ifndef IS_EMULATOR
    // need change to use profile for all devices later
    if (primaryMicModuleInfo_.OpenMicSpeaker == "1") {
        uint32_t sampleFormatBits = AudioPolicyUtils::GetInstance().PcmFormatToBytes(targetInfo.format_);
        targetInfo.bufferSize_ = targetBufferMs * targetInfo.sampleRate_ / static_cast<uint32_t>(MS_PER_S)
            * targetInfo.channels_ * sampleFormatBits;
    }
#endif

    return SUCCESS;
}

void AudioCoreService::UpdateArmModuleInfo(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    AudioModuleInfo& moduleInfo)
{
    CHECK_AND_RETURN_LOG(deviceDesc != nullptr, "AudioDeviceDescriptor is nullptr");
    uint32_t buffCalcMs = BUFFER_CALC_20MS;
    CHECK_AND_RETURN_LOG(CalculateArmBufferDurationMs(moduleInfo, buffCalcMs),
        "calculate arm buffer duration ms failed");
    CHECK_AND_RETURN_LOG(deviceDesc->GetAudioStreamInfo().size() != 0, "AudioStreamInfo is empty");
    DeviceStreamInfo deviceStreamInfo = deviceDesc->GetAudioStreamInfo().back();
    CHECK_AND_RETURN_LOG(!deviceStreamInfo.samplingRate.empty(), "samplingRate is empty");
    std::string halRate = std::to_string(*deviceStreamInfo.samplingRate.begin());
    std::string halFormat = formatEnumToStr[deviceStreamInfo.format];
    AUDIO_INFO_LOG("RATE from hal is: %{public}s, format from hal is: %{public}s, format ori: %{public}u",
        halRate.c_str(), halFormat.c_str(), deviceStreamInfo.format);
    if (!halRate.empty()) {
        moduleInfo.rate = halRate;
    }
    if (!halFormat.empty()) {
        moduleInfo.format = halFormat;
    }

    if (!moduleInfo.rate.empty() && !moduleInfo.format.empty() && !moduleInfo.channels.empty()) {
        uint32_t rateValue = 0;
        uint32_t channelValue = 0;
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.rate, rateValue),
            "convert invalid moduleInfo.rate: %{public}s", moduleInfo.rate.c_str());
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.channels, channelValue),
            "convert invalid moduleInfo.channels: %{public}s", moduleInfo.channels.c_str());

        uint32_t bufferSize = rateValue * channelValue *
            AudioPolicyUtils::GetInstance().PcmFormatToBytes(formatStrToEnum[moduleInfo.format]) *
            buffCalcMs / static_cast<uint32_t>(MS_PER_S);
        moduleInfo.bufferSize = std::to_string(bufferSize);
        AUDIO_INFO_LOG("update arm usb buffer size: %{public}s", moduleInfo.bufferSize.c_str());
    }

    moduleInfo.macAddress = deviceDesc->GetMacAddress();
}

void AudioCoreService::PresetArmIdleInput(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    std::string address = deviceDesc->GetMacAddress();
    AUDIO_INFO_LOG("Entry. address=%{public}s", AudioPolicyUtils::GetInstance().EncUsbAddr(address).c_str());
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = pipeManager_->GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
    CHECK_AND_RETURN_LOG(ret, "GetModuleListByType empty");
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        if (configRole != INPUT_DEVICE) {continue;}
        UpdateArmModuleInfo(deviceDesc, moduleInfo);
        if (isEcFeatureEnable_) {
            usbSourceModuleInfo_ = moduleInfo;
        }
        pipeManager_->UpdateDynamicCapturerConfig(ClassType::TYPE_USB, moduleInfo);
    }
}

void AudioCoreService::CloseUsbArmDevice(const AudioDeviceDescriptor &device)
{
    auto deviceDescs = { std::make_shared<AudioDeviceDescriptor>(device) };
    AudioDeviceFactory::GetInstance().DeactivateDevice(deviceDescs);
}

void AudioCoreService::UpdatePrimaryMicModuleInfo(std::shared_ptr<AudioPipeInfo> &pipeInfo, SourceType sourceType)
{
    if (pipeInfo->adapterName_ != "primary") {
        return;
    }
    if (!isEcFeatureEnable_) {
        return;
    }
    RouterType routerType = ROUTER_TYPE_NONE;
    shared_ptr<AudioDeviceDescriptor> inputDesc =
        audioRouterCenter_.FetchInputDevice(sourceType, -1, routerType);
    if (inputDesc == nullptr || inputDesc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET
            || inputDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        return;
    }

    // update primary info for ec config to get later
    primaryMicModuleInfo_.channels = pipeInfo->moduleInfo_.channels;
    primaryMicModuleInfo_.rate = pipeInfo->moduleInfo_.rate;
    primaryMicModuleInfo_.format = pipeInfo->moduleInfo_.format;
    AUDIO_INFO_LOG("channels: %{public}s, rate: %{public}s, format: %{public}s",
        primaryMicModuleInfo_.channels.c_str(), primaryMicModuleInfo_.rate.c_str(),
        primaryMicModuleInfo_.format.c_str());
}

void AudioCoreService::UpdateModuleInfoForEc(AudioModuleInfo &moduleInfo)
{
    std::lock_guard<std::mutex> lock(audioEcInfoMutex_);
    moduleInfo.ecType = std::to_string(audioEcInfo_.ecType);
    moduleInfo.ecAdapter = audioEcInfo_.ecOutputAdapter;
    moduleInfo.ecSamplingRate = audioEcInfo_.samplingRate;
    moduleInfo.ecFormat = audioEcInfo_.format;
    moduleInfo.ecChannels = audioEcInfo_.channels;
}

void AudioCoreService::ClearModuleInfoForEc(AudioModuleInfo &moduleInfo)
{
    moduleInfo.ecType ="";
    moduleInfo.ecAdapter = "";
    moduleInfo.ecSamplingRate = "";
    moduleInfo.ecFormat = "";
    moduleInfo.ecChannels = "";
}

std::shared_ptr<PipeStreamPropInfo> AudioCoreService::GetMicStreamPropInfoForMicRef()
{
    std::shared_ptr<AdapterPipeInfo> pipeInfo;
    int32_t result = GetPipeInfoByDeviceTypeForEc(ROLE_SOURCE, DEVICE_TYPE_MIC, pipeInfo);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS && pipeInfo != nullptr, nullptr, "mic pipe not found");
    return AudioCoreConfigManager::GetInstance().GetAvailableStreamPropInfoForDevice(
        pipeInfo->streamPropInfos_, DEVICE_TYPE_MIC);
}

void AudioCoreService::UpdateModuleInfoForMicRef(AudioModuleInfo &moduleInfo, SourceType source)
{
    moduleInfo.openMicRef = ShouldOpenMicRef(source);
    moduleInfo.micRefRate = "48000";
    moduleInfo.micRefFormat = "s16le";
    std::string micRefChannels = "4";
    auto micStreamPropInfo = GetMicStreamPropInfoForMicRef();
    if (micStreamPropInfo != nullptr) {
        moduleInfo.micRefChannels = std::to_string(micStreamPropInfo->channels_);
    } else {
        moduleInfo.micRefChannels = micRefChannels;
    }
}

void AudioCoreService::ClearModuleInfoForMicRef(AudioModuleInfo &moduleInfo)
{
    moduleInfo.openMicRef = "0";
    moduleInfo.micRefRate = "";
    moduleInfo.micRefFormat = "";
    moduleInfo.micRefChannels = "";
}

void AudioCoreService::UpdateAudioEcInfo(const AudioDeviceDescriptor &inputDevice,
    const AudioDeviceDescriptor &outputDevice)
{
    if (!isEcFeatureEnable_) {
        AUDIO_INFO_LOG("UpdateModuleForEc ignore for feature not enable");
        return;
    }
    std::lock_guard<std::mutex> lock(audioEcInfoMutex_);
    if (audioEcInfo_.inputDevice.IsSameDeviceDesc(inputDevice) &&
        audioEcInfo_.outputDevice.IsSameDeviceDesc(outputDevice)) {
        AUDIO_INFO_LOG("UpdateModuleForEc abort, no device changed");
        return;
    }
    audioEcInfo_.inputDevice = inputDevice;
    audioEcInfo_.outputDevice = outputDevice;
    audioEcInfo_.ecType = GetEcType(inputDevice.deviceType_, outputDevice.deviceType_);
    audioEcInfo_.ecInputAdapter = GetHalNameForDevice(ROLE_SOURCE, inputDevice.deviceType_);
    audioEcInfo_.ecOutputAdapter = GetHalNameForDevice(ROLE_SINK, outputDevice.deviceType_);
    std::shared_ptr<AdapterPipeInfo> pipeInfo;
    int32_t result = GetPipeInfoByDeviceTypeForEc(ROLE_SINK, outputDevice.deviceType_, pipeInfo);
    CHECK_AND_RETURN_LOG(result == SUCCESS && pipeInfo != nullptr, "Ec stream not update for no pipe found");
    audioEcInfo_.samplingRate = GetEcSamplingRate(audioEcInfo_.ecOutputAdapter, pipeInfo->streamPropInfos_.front());
    audioEcInfo_.format = GetEcFormat(audioEcInfo_.ecOutputAdapter, pipeInfo->streamPropInfos_.front());
    audioEcInfo_.channels = GetEcChannels(audioEcInfo_.ecOutputAdapter, pipeInfo->streamPropInfos_.front());
    AUDIO_INFO_LOG("inputDevice: %{public}d, outputDevice: %{public}d, ecType: %{public}d, ecInputAdapter: %{public}s"
        "ecOutputAdapter:%{public}s, samplingRate: %{public}s, format: %{public}s, channels: %{public}s",
        audioEcInfo_.inputDevice.deviceType_, audioEcInfo_.outputDevice.deviceType_, audioEcInfo_.ecType,
        audioEcInfo_.ecInputAdapter.c_str(), audioEcInfo_.ecOutputAdapter.c_str(), audioEcInfo_.samplingRate.c_str(),
        audioEcInfo_.format.c_str(), audioEcInfo_.channels.c_str());
}

void AudioCoreService::UpdateStreamEcInfo(AudioModuleInfo &moduleInfo, SourceType sourceType)
{
    if (sourceType != SOURCE_TYPE_VOICE_COMMUNICATION && sourceType != SOURCE_TYPE_VOICE_TRANSCRIPTION) {
        ClearModuleInfoForEc(moduleInfo);
        AUDIO_INFO_LOG("sourceType: %{public}d need clear ec data", sourceType);
        return;
    }

    FetchDeviceInfo info = { STREAM_USAGE_VOICE_COMMUNICATION, "UpdateStreamEcInfo" };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDesc =
        audioRouterCenter_.FetchOutputDevices(info);
    RouterType routerType = ROUTER_TYPE_NONE;
    std::shared_ptr<AudioDeviceDescriptor> inputDesc =
        audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_VOICE_COMMUNICATION, -1, routerType);

    CHECK_AND_RETURN_LOG(inputDesc && !outputDesc.empty() && outputDesc.front(), "Device is nullptr");
    UpdateAudioEcInfo(*inputDesc, *outputDesc.front());
    UpdateModuleInfoForEc(moduleInfo);
}

void AudioCoreService::UpdateStreamMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType)
{
    if (sourceType != SOURCE_TYPE_VOICE_COMMUNICATION && sourceType != SOURCE_TYPE_MIC) {
        ClearModuleInfoForMicRef(moduleInfo);
        AUDIO_INFO_LOG("sourceType: %{public}d need clear micref data", sourceType);
        return;
    }

    UpdateModuleInfoForMicRef(moduleInfo, sourceType);
}

void AudioCoreService::UpdateStreamEcAndMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType)
{
    UpdateStreamEcInfo(moduleInfo, sourceType);
    UpdateStreamMicRefInfo(moduleInfo, sourceType);
}

void AudioCoreService::SetOpenedNormalSource(SourceType sourceType)
{
    normalSourceOpened_ = sourceType;
}

void AudioCoreService::PrepareNormalSource(std::shared_ptr<AudioPipeInfo> &pipeInfo,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    SourceType sourceType = streamDesc->capturerInfo_.sourceType;
    AUDIO_INFO_LOG("prepare normal source for source type: %{public}d", sourceType);
    // coreService
    UpdateEnhanceEffectState(sourceType);
    UpdatePrimaryMicModuleInfo(pipeInfo, sourceType);
    UpdateStreamEcAndMicRefInfo(pipeInfo->moduleInfo_, sourceType);
    SetOpenedNormalSource(sourceType);
    SetOpenedNormalSourceSessionId(streamDesc->sessionId_);
}

void AudioCoreService::UpdateModuleInfoDeviceType(AudioModuleInfo &moduleInfo,
    const std::shared_ptr<AudioDeviceDescriptor> &inputDesc)
{
    CHECK_AND_RETURN_LOG(inputDesc != nullptr, "inputDesc is nullptr");
    moduleInfo.deviceType = std::to_string(static_cast<int32_t>(inputDesc->deviceType_));
}

void AudioCoreService::UpdateStreamCommonInfo(AudioModuleInfo &moduleInfo, PipeStreamPropInfo &targetInfo,
    SourceType sourceType)
{
    RouterType routerType = ROUTER_TYPE_NONE;
    shared_ptr<AudioDeviceDescriptor> inputDesc =
        audioRouterCenter_.FetchInputDevice(sourceType, -1, routerType);
    if (!isEcFeatureEnable_) {
        moduleInfo = primaryMicModuleInfo_;
        // current layout represents the number of channel. This will need to be modify in the future.
        moduleInfo.channels = std::to_string(targetInfo.channels_);
        moduleInfo.rate = std::to_string(targetInfo.sampleRate_);
        moduleInfo.bufferSize = std::to_string(targetInfo.bufferSize_);
        moduleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetInfo.format_];
        moduleInfo.sourceType = std::to_string(sourceType);
        moduleInfo.channelLayout = std::to_string(targetInfo.channelLayout_);
        UpdateModuleInfoDeviceType(moduleInfo, inputDesc);
    } else {
        if (inputDesc != nullptr && inputDesc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
            moduleInfo = usbSourceModuleInfo_;
            moduleInfo.sourceType = std::to_string(sourceType);
            moduleInfo.deviceType = std::to_string(static_cast<int32_t>(DEVICE_TYPE_USB_ARM_HEADSET));
            moduleInfo.macAddress = inputDesc->macAddress_;
        } else {
            moduleInfo = primaryMicModuleInfo_;
            // current layout represents the number of channel. This will need to be modify in the future.
            moduleInfo.channels = std::to_string(targetInfo.channels_);
            moduleInfo.rate = std::to_string(targetInfo.sampleRate_);
            moduleInfo.bufferSize = std::to_string(targetInfo.bufferSize_);
            moduleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetInfo.format_];
            moduleInfo.sourceType = std::to_string(sourceType);
            moduleInfo.channelLayout = std::to_string(targetInfo.channelLayout_);
            if (inputDesc != nullptr) {
                moduleInfo.deviceType = std::to_string(static_cast<int32_t>(inputDesc->deviceType_));
            }
            // update primary info for ec config to get later
            primaryMicModuleInfo_.channels = std::to_string(targetInfo.channels_);
            primaryMicModuleInfo_.rate = std::to_string(targetInfo.sampleRate_);
            primaryMicModuleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetInfo.format_];
            primaryMicModuleInfo_.channelLayout = std::to_string(targetInfo.channelLayout_);
        }
    }
}

int32_t AudioCoreService::ReloadNormalSource(SessionInfo &sessionInfo,
    PipeStreamPropInfo &targetInfo, SourceType targetSource, const int32_t uid)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = pipeManager_->GetNormalSourceInfo(isEcFeatureEnable_);
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, ERROR, "Get normal source info failed");

    AudioModuleInfo moduleInfo;
    // coreService
    UpdateEnhanceEffectState(targetSource);
    UpdateStreamCommonInfo(moduleInfo, targetInfo, targetSource);
    UpdateStreamEcInfo(moduleInfo, targetSource);
    UpdateStreamMicRefInfo(moduleInfo, targetSource);

    AUDIO_INFO_LOG("rate: %{public}s, channels: %{public}s, bufferSize: %{public}s format: %{public}s, "
        "sourceType: %{public}s",
        moduleInfo.rate.c_str(), moduleInfo.channels.c_str(), moduleInfo.bufferSize.c_str(),
        moduleInfo.format.c_str(), moduleInfo.sourceType.c_str());

    uint32_t capturePortIdx = audioInjectorPolicy_.GetCapturePortIdx();
    bool removeFlag = false;
    if (pipeInfo->pipeRole_ == PIPE_ROLE_INPUT && targetSource == SOURCE_TYPE_VOICE_COMMUNICATION &&
        pipeInfo->paIndex_ != UINT32_INVALID_VALUE && pipeInfo->paIndex_ == capturePortIdx) {
        audioInjectorPolicy_.RemoveCaptureInjector(true);
        removeFlag = true;
    }

    audioIOHandleMap_.ReloadPortAndUpdateIOHandle(pipeInfo, moduleInfo);
    audioPolicyManager_.SetDeviceActive(audioActiveDevice_.GetCurrentInputDeviceType(uid), moduleInfo.name,
        true, INPUT_DEVICES_FLAG);

    normalSourceOpened_ = targetSource;

    CHECK_AND_RETURN_RET_LOG(pipeInfo->paIndex_ != UINT32_INVALID_VALUE, SUCCESS, "no need to AddCaptureInjector");
    if (removeFlag) {
        audioInjectorPolicy_.SetCapturePortIdx(pipeInfo->paIndex_);
        audioInjectorPolicy_.SetVoipType(VoipType::NORMAL_VOIP);
        audioInjectorPolicy_.AddCaptureInjector();
    }
    return SUCCESS;
}

int32_t AudioCoreService::ReloadSourceForInputPipe(std::shared_ptr<AudioPipeInfo> &pipeInfo, uint32_t targetSessionId)
{
    CHECK_AND_RETURN_RET_LOG((pipeInfo != nullptr && pipeInfo->streamDescMap_.count(targetSessionId) > 0), ERROR,
        "pipe is null or can not find stream");
    auto streamDesc = pipeInfo->streamDescMap_[targetSessionId];
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERROR, "streamDesc is null");
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    pipeManager_->GetStreamPropInfo(streamDesc, streamPropInfo);
    PipeStreamPropInfo targetInfo = *streamPropInfo;

    AudioModuleInfo moduleInfo = pipeInfo->moduleInfo_;
    moduleInfo.channels = std::to_string(targetInfo.channels_);
    moduleInfo.rate = std::to_string(targetInfo.sampleRate_);
    moduleInfo.bufferSize = std::to_string(targetInfo.bufferSize_);
    moduleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetInfo.format_];
    moduleInfo.channelLayout = std::to_string(targetInfo.channelLayout_);
    moduleInfo.sourceType = std::to_string(streamDesc->capturerInfo_.sourceType);
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT) {
        moduleInfo.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
        moduleInfo.ecSamplingRate = std::to_string(streamDesc->ecStreamInfo_.samplingRate);
        moduleInfo.ecChannels = std::to_string(streamDesc->ecStreamInfo_.channels);
        moduleInfo.ecFormat = formatToHpaeStr[streamDesc->ecStreamInfo_.format];
    } else if (IsVoiceRecognitionMicInEcRequested(streamDesc)) {
        UpdateMicInEcModuleInfoByStreamDesc(streamDesc, moduleInfo);
    } else if (IsCamcorderMicInRequested(streamDesc)) {
        UpdateCamcorderMicInModuleInfo(streamDesc, moduleInfo);
    }
    AUDIO_INFO_LOG("sessionId:%{public}u rate:%{public}s channels:%{public}s bufferSize:%{public}s"
        "format:%{public}s channelLayout:%{public}s sourceType: %{public}s", targetSessionId,
        moduleInfo.rate.c_str(), moduleInfo.channels.c_str(), moduleInfo.bufferSize.c_str(),
        moduleInfo.format.c_str(), moduleInfo.channelLayout.c_str(), moduleInfo.sourceType.c_str());
    int32_t uid = streamDesc->GetRealUid();
    int32_t result = audioIOHandleMap_.ReloadPortAndUpdateIOHandle(pipeInfo, moduleInfo);
    audioPolicyManager_.SetDeviceActive(audioActiveDevice_.GetCurrentInputDeviceType(uid),
        pipeInfo->moduleInfo_.name, true, INPUT_DEVICES_FLAG);
    if (result == SUCCESS) {
        audioActiveDevice_.UpdateActiveDeviceRoute(audioActiveDevice_.GetCurrentInputDeviceType(uid),
            DeviceFlag::INPUT_DEVICES_FLAG, pipeManager_->QueryPipeIdBySessionId(targetSessionId),
            audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid).networkId_);
    }
    return result;
}

void AudioCoreService::ReloadSourceForSession(SessionInfo sessionInfo, uint32_t targetSessionId)
{
    AUDIO_INFO_LOG("reload session for source: %{public}d", sessionInfo.sourceType);

    PipeStreamPropInfo targetInfo;
    SourceType targetSource = sessionInfo.sourceType;
    int32_t res = FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSource);
    CHECK_AND_RETURN_LOG(res == SUCCESS, "fetch target source info error");
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager_ is nullptr");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(targetSessionId);
    CHECK_AND_RETURN_LOG(streamDesc != nullptr, "streamDesc is nullptr");
    int32_t uid = streamDesc->GetRealUid();
    ReloadNormalSource(sessionInfo, targetInfo, targetSource, uid);

    audioActiveDevice_.UpdateActiveDeviceRoute(audioActiveDevice_.GetCurrentInputDeviceType(uid),
        DeviceFlag::INPUT_DEVICES_FLAG, pipeManager_->QueryPipeIdBySessionId(targetSessionId),
        audioRouterSelectStrategy_.Get1stCurrentInputDevice(uid).networkId_);
}

int32_t AudioCoreService::ReloadSourceSoftLink(std::shared_ptr<AudioPipeInfo> &pipeInfo,
    const AudioModuleInfo &moduleInfo)
{
    int32_t ret = audioIOHandleMap_.ReloadPortAndUpdateIOHandle(pipeInfo, moduleInfo, true);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "reload softLink failed");
    normalSourceOpened_ = SOURCE_TYPE_VOICE_CALL;
    AUDIO_INFO_LOG("reload hearingAid");
    return SUCCESS;
}

std::string AudioCoreService::ShouldOpenMicRef(SourceType source)
{
    std::string shouldOpen = "0";
    if (!isMicRefFeatureEnable_) {
        AUDIO_INFO_LOG("isMicRefFeatureEnable_ is off");
        return shouldOpen;
    }

    RouterType routerType = ROUTER_TYPE_NONE;
    std::shared_ptr<AudioDeviceDescriptor> inputDesc = audioRouterCenter_.FetchInputDevice(source, -1, routerType);
    CHECK_AND_RETURN_RET_LOG(inputDesc != nullptr, shouldOpen, "inputDesc is nullptr");
    auto iter = std::find(MIC_REF_DEVICES.begin(), MIC_REF_DEVICES.end(), inputDesc->deviceType_);
    if ((source == SOURCE_TYPE_VOICE_COMMUNICATION && isMicRefVoipUpOn_ && iter != MIC_REF_DEVICES.end()) ||
        (source == SOURCE_TYPE_MIC && isMicRefRecordOn_ && iter != MIC_REF_DEVICES.end())) {
        shouldOpen = "1";
    }

    AUDIO_INFO_LOG("source: %{public}d, voipUpMicOn: %{public}d, recordMicOn: %{public}d, device: %{public}d",
        source, isMicRefVoipUpOn_, isMicRefRecordOn_, inputDesc->deviceType_);
    return shouldOpen;
}

std::string AudioCoreService::GetEcFormat(const std::string &halName,
    std::shared_ptr<PipeStreamPropInfo> &outModuleInfo)
{
    if (halName == DP_CLASS) {
        if (!dpSinkModuleInfo_.format.empty()) {
            AUDIO_INFO_LOG("use dp cust param");
            return dpSinkModuleInfo_.format;
        }
        return AudioDefinitionPolicyUtils::enumToFormatStr[outModuleInfo->format_];
    } else if (halName == USB_CLASS) {
        if (!usbSinkModuleInfo_.format.empty()) {
            AUDIO_INFO_LOG("use arm usb cust param");
            return usbSinkModuleInfo_.format;
        }
        return AudioDefinitionPolicyUtils::enumToFormatStr[outModuleInfo->format_];
    } else {
        return primaryMicModuleInfo_.format;
    }
}

std::string AudioCoreService::GetEcChannels(const std::string &halName,
    std::shared_ptr<PipeStreamPropInfo> &outModuleInfo)
{
    if (halName == DP_CLASS) {
        if (!dpSinkModuleInfo_.channels.empty()) {
            AUDIO_INFO_LOG("use dp cust param");
            return dpSinkModuleInfo_.channels;
        }
        return std::to_string(outModuleInfo->channels_);
    } else if (halName == USB_CLASS) {
        if (!usbSinkModuleInfo_.channels.empty()) {
            AUDIO_INFO_LOG("use arm usb cust param");
            return usbSinkModuleInfo_.channels;
        }
        return std::to_string(outModuleInfo->channels_);
    } else {
        return std::to_string(HEADPHONE_CHANNEL_NUM);
    }
}

std::string AudioCoreService::GetEcSamplingRate(const std::string &halName,
    std::shared_ptr<PipeStreamPropInfo> &outModuleInfo)
{
    if (halName == DP_CLASS) {
        if (!dpSinkModuleInfo_.rate.empty()) {
            AUDIO_INFO_LOG("use dp cust param");
            return dpSinkModuleInfo_.rate;
        }
        return std::to_string(outModuleInfo->sampleRate_);
    } else if (halName == USB_CLASS) {
        if (!usbSinkModuleInfo_.rate.empty()) {
            AUDIO_INFO_LOG("use arm usb cust param");
            return usbSinkModuleInfo_.rate;
        }
        return std::to_string(outModuleInfo->sampleRate_);
    } else {
        return primaryMicModuleInfo_.rate;
    }
}

std::string AudioCoreService::GetPipeNameByDeviceForEc(const std::string &role, const DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_SPEAKER:
            return PIPE_PRIMARY_OUTPUT;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_NEARLINK:
        case DEVICE_TYPE_NEARLINK_IN:
            if (role == ROLE_SOURCE) {
                return PIPE_PRIMARY_INPUT;
            }
            return PIPE_PRIMARY_OUTPUT;
        case DEVICE_TYPE_MIC:
            return PIPE_PRIMARY_INPUT;
        case DEVICE_TYPE_USB_ARM_HEADSET:
            if (role == ROLE_SOURCE) {
                return PIPE_USB_ARM_INPUT;
            }
            return PIPE_USB_ARM_OUTPUT;
        case DEVICE_TYPE_DP:
            return PIPE_DP_OUTPUT;
        case DEVICE_TYPE_ACCESSORY:
            return PIPE_ACCESSORY_INPUT;
        default:
            AUDIO_ERR_LOG("invalid deevice type %{public}d for role %{public}s", deviceType, role.c_str());
            return PIPE_PRIMARY_OUTPUT;
    }
}

EcType AudioCoreService::GetEcType(const DeviceType inputDevice, const DeviceType outputDevice)
{
    EcType ecType = EC_TYPE_NONE;
    auto element = DEVICE_TO_EC_TYPE.find(std::make_pair(inputDevice, outputDevice));
    if (element != DEVICE_TO_EC_TYPE.end()) {
        ecType = element->second;
    }
    AUDIO_INFO_LOG("GetEcType ecType: %{public}d", ecType);
    return ecType;
}


int32_t AudioCoreService::GetPipeInfoByDeviceTypeForEc(const std::string &role, const DeviceType deviceType,
    std::shared_ptr<AdapterPipeInfo> &pipeInfo)
{
    std::string portName;
    if (role == ROLE_SOURCE) {
        portName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    } else {
        portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    }
    std::shared_ptr<PolicyAdapterInfo> info;
    bool ret = pipeManager_->GetAdapterInfoByType(static_cast<AudioAdapterType>(
        AudioPolicyUtils::portStrToEnum[portName]), info);
    CHECK_AND_RETURN_RET_LOG(ret && info != nullptr, ERR_NOT_SUPPORTED,
        "no adapter found for deviceType: %{public}d, portName: %{public}s", deviceType, portName.c_str());
    std::string pipeName = GetPipeNameByDeviceForEc(role, deviceType);
    pipeInfo = info->GetPipeInfoByName(pipeName);
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("no pipe info found for pipeName: %{public}s, deviceType: %{public}d, portName: %{public}s",
            pipeName.c_str(), deviceType, portName.c_str());
        return ERROR;
    }
    AUDIO_INFO_LOG("pipe name: %{public}s, moduleName: %{public}s found for device: %{public}d",
        pipeInfo->name_.c_str(), pipeInfo->paProp_.moduleName_.c_str(), deviceType);
    return SUCCESS;
}

std::string AudioCoreService::GetHalNameForDevice(const std::string &role, const DeviceType deviceType)
{
    std::string halName = "";
    std::string portName;
    if (role == ROLE_SOURCE) {
        portName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    } else {
        portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    }
    std::shared_ptr<PolicyAdapterInfo> info;
    bool ret = pipeManager_->GetAdapterInfoByType(static_cast<AudioAdapterType>(
        AudioPolicyUtils::portStrToEnum[portName]), info);
    if (ret) {
        halName = info->adapterName;
    }
    AUDIO_INFO_LOG("role: %{public}s, device: %{public}d, halName: %{public}s",
        role.c_str(), deviceType, halName.c_str());
    return halName;
}

int32_t AudioCoreService::GetActiveStreamsVolumeInfo(std::vector<ActiveStreamVolumeInfo> &activeStreamsVolumeInfo)
{
    std::vector<std::pair<int32_t, AudioStreamType>> activeStreams = streamCollector_.GetActiveStreams();
    for (const auto &[uid, streamType] : activeStreams) {
        int32_t systemAppVolumePercentage = 0;
        audioVolumeManager_.GetSystemAppVolumePercentage(uid, systemAppVolumePercentage);
        AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
        ActiveStreamVolumeInfo activeStreamVolumeInfo {volumeType, systemAppVolumePercentage, uid};
        if (std::find(activeStreamsVolumeInfo.begin(), activeStreamsVolumeInfo.end(),
            activeStreamVolumeInfo) == activeStreamsVolumeInfo.end()) {
            activeStreamsVolumeInfo.push_back(activeStreamVolumeInfo);
            AUDIO_INFO_LOG("clientUID: %{public}d, volumeType: %{public}d, volumePercentage: %{public}d",
                uid, volumeType, systemAppVolumePercentage);
        }
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
