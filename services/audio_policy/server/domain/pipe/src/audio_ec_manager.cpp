/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEcManager"
#endif

#include "audio_ec_manager.h"
#include "parameter.h"
#include "parameters.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "audio_pipe_manager.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t MS_PER_S = 1000;
static const unsigned int BUFFER_CALC_20MS = 20;
const uint32_t PC_MIC_CHANNEL_NUM = 4;
const uint32_t HEADPHONE_CHANNEL_NUM = 2;
static const char* PIPE_PRIMARY_OUTPUT = "primary_output";
static const char* PIPE_PRIMARY_INPUT = "primary_input";
static const char* PIPE_USB_ARM_OUTPUT = "usb_arm_output";
static const char* PIPE_USB_ARM_INPUT = "usb_arm_input";
static const char* PIPE_DP_OUTPUT = "dp_output";
static const char* PIPE_ACCESSORY_INPUT = "accessory_input";
const float RENDER_FRAME_INTERVAL_IN_SECONDS = 0.02;

static std::map<std::string, uint32_t> formatFromParserStrToEnum = {
    {"s16le", SAMPLE_S16LE},
    {"s24le", SAMPLE_S24LE},
    {"s32le", SAMPLE_S32LE},
};

static const std::vector<DeviceType> MIC_REF_DEVICES = {
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN
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


static const std::map<std::pair<DeviceType, DeviceType>, EcType> DEVICE_TO_EC_TYPE = {
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_MIC, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_SPEAKER}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_SPEAKER}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_USB_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_WIRED_HEADSET}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_USB_ARM_HEADSET}, EC_TYPE_DIFF_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_SCO}, EC_TYPE_SAME_ADAPTER},
    {{DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_DP}, EC_TYPE_DIFF_ADAPTER},

    {{DEVICE_TYPE_ACCESSORY, DEVICE_TYPE_SPEAKER}, EC_TYPE_DIFF_ADAPTER},
};

static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

static string ParseAudioFormat(string format)
{
    if (format == "AUDIO_FORMAT_PCM_16_BIT") {
        return "s16le";
    } else if (format == "AUDIO_FORMAT_PCM_24_BIT" || format == "AUDIO_FORMAT_PCM_24_BIT_PACKED") {
        return "s24le";
    } else if (format == "AUDIO_FORMAT_PCM_32_BIT") {
        return "s32le";
    } else {
        return "s16le";
    }
}

static void GetUsbModuleInfo(string deviceInfo, AudioModuleInfo &moduleInfo)
{
    if (moduleInfo.role == "sink") {
        auto sinkRate_begin = deviceInfo.find("sink_rate:");
        auto sinkRate_end = deviceInfo.find_first_of(";", sinkRate_begin);
        moduleInfo.rate = deviceInfo.substr(sinkRate_begin + std::strlen("sink_rate:"),
            sinkRate_end - sinkRate_begin - std::strlen("sink_rate:"));
        auto sinkFormat_begin = deviceInfo.find("sink_format:");
        auto sinkFormat_end = deviceInfo.find_first_of(";", sinkFormat_begin);
        string format = deviceInfo.substr(sinkFormat_begin + std::strlen("sink_format:"),
            sinkFormat_end - sinkFormat_begin - std::strlen("sink_format:"));
        moduleInfo.format = ParseAudioFormat(format);
    } else {
        auto sourceRate_begin = deviceInfo.find("source_rate:");
        auto sourceRate_end = deviceInfo.find_first_of(";", sourceRate_begin);
        moduleInfo.rate = deviceInfo.substr(sourceRate_begin + std::strlen("source_rate:"),
            sourceRate_end - sourceRate_begin - std::strlen("source_rate:"));
        auto sourceFormat_begin = deviceInfo.find("source_format:");
        auto sourceFormat_end = deviceInfo.find_first_of(";", sourceFormat_begin);
        string format = deviceInfo.substr(sourceFormat_begin + std::strlen("source_format:"),
            sourceFormat_end - sourceFormat_begin - std::strlen("source_format:"));
        moduleInfo.format = ParseAudioFormat(format);
    }

    if (!moduleInfo.rate.empty() && !moduleInfo.format.empty() && !moduleInfo.channels.empty()) {
        uint32_t rateValue = 0;
        uint32_t channelValue = 0;
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.rate, rateValue),
            "convert invalid moduleInfo.rate: %{public}s", moduleInfo.rate.c_str());
        CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.channels, channelValue),
            "convert invalid moduleInfo.channels: %{public}s", moduleInfo.channels.c_str());

        uint32_t bufferSize = rateValue * channelValue *
            AudioPolicyUtils::GetInstance().PcmFormatToBytes(static_cast<AudioSampleFormat>(
                formatFromParserStrToEnum[moduleInfo.format])) * BUFFER_CALC_20MS / static_cast<uint32_t>(MS_PER_S);
        moduleInfo.bufferSize = std::to_string(bufferSize);
    }
}

void AudioEcManager::Init(int32_t ecEnableState, int32_t micRefEnableState)
{
    isEcFeatureEnable_ = ecEnableState != 0;
    isMicRefFeatureEnable_ = micRefEnableState != 0;
}

void AudioEcManager::CloseNormalSource()
{
    AUDIO_INFO_LOG("close all sources");
    audioIOHandleMap_.ClosePortAndEraseIOHandle(BLUETOOTH_MIC);
    audioIOHandleMap_.ClosePortAndEraseIOHandle(PRIMARY_MIC);
    if (isEcFeatureEnable_) {
        audioIOHandleMap_.ClosePortAndEraseIOHandle(USB_MIC);
    }
    normalSourceOpened_ = SOURCE_TYPE_INVALID;
}

void AudioEcManager::UpdateEnhanceEffectState(SourceType source)
{
    AudioEnhancePropertyArray enhancePropertyArray = {};
    std::shared_ptr<AudioDeviceDescriptor> inputDesc = audioRouterCenter_.FetchInputDevice(source, -1);
    CHECK_AND_RETURN_LOG(inputDesc != nullptr, "inputDesc is nullptr");
    int32_t ret = AudioServerProxy::GetInstance().GetAudioEnhancePropertyProxy(enhancePropertyArray,
        inputDesc->deviceType_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("get enhance property fail, ret: %{public}d", ret);
        return;
    }
    std::string recordProp = "";
    std::string voipUpProp = "";
    for (const AudioEnhanceProperty &prop : enhancePropertyArray.property) {
        if (prop.enhanceClass == "record") {
            recordProp = prop.enhanceProp;
        }
        if (prop.enhanceClass == "voip_up") {
            voipUpProp = prop.enhanceProp;
        }
    }
    isMicRefRecordOn_ = (recordProp == "NRON");
    isMicRefVoipUpOn_ = (voipUpProp == "PNR");

    AUDIO_INFO_LOG("ecEnableState: %{public}d, micRefEnableState: %{public}d, "
        "isMicRefRecordOn_: %{public}d, isMicRefVoipUp: %{public}d",
        isEcFeatureEnable_, isMicRefFeatureEnable_, isMicRefRecordOn_, isMicRefVoipUpOn_);
}

void AudioEcManager::UpdateStreamCommonInfo(AudioModuleInfo &moduleInfo, PipeStreamPropInfo &targetInfo,
    SourceType sourceType)
{
    if (!isEcFeatureEnable_) {
        moduleInfo = primaryMicModuleInfo_;
        // current layout represents the number of channel. This will need to be modify in the future.
        moduleInfo.channels = std::to_string(targetInfo.channels_);
        moduleInfo.rate = std::to_string(targetInfo.sampleRate_);
        moduleInfo.bufferSize = std::to_string(targetInfo.bufferSize_);
        moduleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetInfo.format_];
        moduleInfo.sourceType = std::to_string(sourceType);
    } else {
        shared_ptr<AudioDeviceDescriptor> inputDesc = audioRouterCenter_.FetchInputDevice(sourceType, -1);
        if (inputDesc != nullptr && inputDesc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
            moduleInfo = usbSourceModuleInfo_;
            moduleInfo.sourceType = std::to_string(sourceType);
            moduleInfo.deviceType = std::to_string(static_cast<int32_t>(DEVICE_TYPE_USB_ARM_HEADSET));
        } else {
            moduleInfo = primaryMicModuleInfo_;
            // current layout represents the number of channel. This will need to be modify in the future.
            moduleInfo.channels = std::to_string(targetInfo.channels_);
            moduleInfo.rate = std::to_string(targetInfo.sampleRate_);
            moduleInfo.bufferSize = std::to_string(targetInfo.bufferSize_);
            moduleInfo.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetInfo.format_];
            moduleInfo.sourceType = std::to_string(sourceType);
            if (inputDesc != nullptr) {
                moduleInfo.deviceType = std::to_string(static_cast<int32_t>(inputDesc->deviceType_));
            }
            // update primary info for ec config to get later
            primaryMicModuleInfo_.channels = std::to_string(targetInfo.channels_);
            primaryMicModuleInfo_.rate = std::to_string(targetInfo.sampleRate_);
            primaryMicModuleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[targetInfo.format_];
        }
    }
}

void AudioEcManager::UpdateStreamEcInfo(AudioModuleInfo &moduleInfo, SourceType sourceType)
{
    if (sourceType != SOURCE_TYPE_VOICE_COMMUNICATION && sourceType != SOURCE_TYPE_VOICE_TRANSCRIPTION) {
        AUDIO_INFO_LOG("sourceType: %{public}d not need ec data", sourceType);
        return;
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDesc =
        audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_COMMUNICATION, -1, "UpdateStreamEcInfo");
    std::shared_ptr<AudioDeviceDescriptor> inputDesc =
        audioRouterCenter_.FetchInputDevice(SOURCE_TYPE_VOICE_COMMUNICATION, -1);

    CHECK_AND_RETURN_LOG(inputDesc && !outputDesc.empty() && outputDesc.front(), "Device is nullptr");
    UpdateAudioEcInfo(*inputDesc, *outputDesc.front());
    UpdateModuleInfoForEc(moduleInfo);
}

void AudioEcManager::UpdateStreamMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType)
{
    if (sourceType != SOURCE_TYPE_VOICE_COMMUNICATION && sourceType != SOURCE_TYPE_MIC) {
        AUDIO_INFO_LOG("sourceType: %{public}d not need micref data", sourceType);
        return;
    }

    UpdateModuleInfoForMicRef(moduleInfo, sourceType);
}

std::string AudioEcManager::GetEcSamplingRate(const std::string &halName,
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

std::string AudioEcManager::GetEcFormat(const std::string &halName,
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

std::string AudioEcManager::GetEcChannels(const std::string &halName,
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

std::string AudioEcManager::GetPipeNameByDeviceForEc(const std::string &role, const DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_SPEAKER:
            return PIPE_PRIMARY_OUTPUT;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_BLUETOOTH_SCO:
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
            AUDIO_ERR_LOG("invalid device type %{public}d for role %{public}s", deviceType, role.c_str());
            return PIPE_PRIMARY_OUTPUT;
    }
}

int32_t AudioEcManager::GetPipeInfoByDeviceTypeForEc(const std::string &role, const DeviceType deviceType,
    std::shared_ptr<AdapterPipeInfo> &pipeInfo)
{
    std::string portName;
    if (role == ROLE_SOURCE) {
        portName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    } else {
        portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    }
    std::shared_ptr<PolicyAdapterInfo> info;
    bool ret = audioConfigManager_.GetAdapterInfoByType(static_cast<AudioAdapterType>(
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

EcType AudioEcManager::GetEcType(const DeviceType inputDevice, const DeviceType outputDevice)
{
    EcType ecType = EC_TYPE_NONE;
    auto element = DEVICE_TO_EC_TYPE.find(std::make_pair(inputDevice, outputDevice));
    if (element != DEVICE_TO_EC_TYPE.end()) {
        ecType = element->second;
    }
    AUDIO_INFO_LOG("GetEcType ecType: %{public}d", ecType);
    return ecType;
}

void AudioEcManager::UpdateAudioEcInfo(const AudioDeviceDescriptor &inputDevice,
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
    CHECK_AND_RETURN_LOG(result == SUCCESS, "Ec stream not update for no pipe found");
    audioEcInfo_.samplingRate = GetEcSamplingRate(audioEcInfo_.ecOutputAdapter, pipeInfo->streamPropInfos_.front());
    audioEcInfo_.format = GetEcFormat(audioEcInfo_.ecOutputAdapter, pipeInfo->streamPropInfos_.front());
    audioEcInfo_.channels = GetEcChannels(audioEcInfo_.ecOutputAdapter, pipeInfo->streamPropInfos_.front());
    AUDIO_INFO_LOG("inputDevice: %{public}d, outputDevice: %{public}d, ecType: %{public}d, ecInputAdapter: %{public}s"
        "ecOutputAdapter: %{public}s, samplingRate: %{public}s, format: %{public}s, channels: %{public}s",
        audioEcInfo_.inputDevice.deviceType_, audioEcInfo_.outputDevice.deviceType_, audioEcInfo_.ecType,
        audioEcInfo_.ecInputAdapter.c_str(), audioEcInfo_.ecOutputAdapter.c_str(), audioEcInfo_.samplingRate.c_str(),
        audioEcInfo_.format.c_str(), audioEcInfo_.channels.c_str());
}

void AudioEcManager::UpdateModuleInfoForEc(AudioModuleInfo &moduleInfo)
{
    std::lock_guard<std::mutex> lock(audioEcInfoMutex_);
    moduleInfo.ecType = std::to_string(audioEcInfo_.ecType);
    moduleInfo.ecAdapter = audioEcInfo_.ecOutputAdapter;
    moduleInfo.ecSamplingRate = audioEcInfo_.samplingRate;
    moduleInfo.ecFormat = audioEcInfo_.format;
    moduleInfo.ecChannels = audioEcInfo_.channels;
}

std::string AudioEcManager::ShouldOpenMicRef(SourceType source)
{
    std::string shouldOpen = "0";
    if (!isMicRefFeatureEnable_) {
        AUDIO_INFO_LOG("isMicRefFeatureEnable_ is off");
        return shouldOpen;
    }

    std::shared_ptr<AudioDeviceDescriptor> inputDesc = audioRouterCenter_.FetchInputDevice(source, -1);
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

void AudioEcManager::UpdateModuleInfoForMicRef(AudioModuleInfo &moduleInfo, SourceType source)
{
    moduleInfo.openMicRef = ShouldOpenMicRef(source);
    moduleInfo.micRefRate = "48000";
    moduleInfo.micRefFormat = "s16le";
    moduleInfo.micRefChannels = "4";
}

AudioEcInfo AudioEcManager::GetAudioEcInfo()
{
    std::lock_guard<std::mutex> lock(audioEcInfoMutex_);
    return audioEcInfo_;
}

void AudioEcManager::ResetAudioEcInfo()
{
    std::lock_guard<std::mutex> lock(audioEcInfoMutex_);
    audioEcInfo_.inputDevice.deviceType_ = DEVICE_TYPE_NONE;
    audioEcInfo_.outputDevice.deviceType_ = DEVICE_TYPE_NONE;
}

void AudioEcManager::PresetArmIdleInput(const string& address)
{
    AUDIO_INFO_LOG("Entry. address=%{public}s", GetEncryptAddr(address).c_str());
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
    CHECK_AND_RETURN_LOG(ret, "GetModuleListByType empty");
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        if (configRole != INPUT_DEVICE) {continue;}
        UpdateArmModuleInfo(address, INPUT_DEVICE, moduleInfo);
        if (isEcFeatureEnable_) {
            usbSourceModuleInfo_ = moduleInfo;
        }
        audioConfigManager_.UpdateDynamicCapturerConfig(ClassType::TYPE_USB, moduleInfo);
    }
}

void AudioEcManager::ActivateArmDevice(const string& address, const DeviceRole role)
{
    AUDIO_INFO_LOG("address=%{public}s, role=%{public}d", GetEncryptAddr(address).c_str(), role);
    string &activeArmAddr = role == INPUT_DEVICE ? activeArmInputAddr_ : activeArmOutputAddr_;
    CHECK_AND_RETURN_LOG(address != activeArmAddr, "usb device addr already active");
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
    CHECK_AND_RETURN_LOG(ret, "GetModuleListByType empty");
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        if (configRole != role) {continue;}
        AUDIO_INFO_LOG("[module_reload]: module[%{public}s], role[%{public}d]", moduleInfo.name.c_str(), role);
        if (!(isEcFeatureEnable_ && role == INPUT_DEVICE) && audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name)) {
            audioIOHandleMap_.MuteDefaultSinkPort(audioActiveDevice_.GetCurrentOutputDeviceNetworkId(),
                AudioPolicyUtils::GetInstance().GetSinkPortName(audioActiveDevice_.GetCurrentOutputDeviceType()));
            audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleInfo.name);
        }
        UpdateArmModuleInfo(address, role, moduleInfo);
        if (isEcFeatureEnable_) {
            if (role == OUTPUT_DEVICE) {
                int32_t ret = audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
                CHECK_AND_RETURN_LOG(ret == SUCCESS,
                    "Load usb %{public}s failed %{public}d", moduleInfo.role.c_str(), ret);
                usbSinkModuleInfo_ = moduleInfo;
            } else {
                AUDIO_INFO_LOG("just save arm usb source module info, rate=%{public}s", moduleInfo.rate.c_str());
                usbSourceModuleInfo_ = moduleInfo;
            }
        } else {
            int32_t ret = audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
            CHECK_AND_RETURN_LOG(ret == SUCCESS,
                "Load usb %{public}s failed %{public}d", moduleInfo.role.c_str(), ret);
        }
    }
    activeArmAddr = address;
}

void AudioEcManager::CloseUsbArmDevice(const AudioDeviceDescriptor &device)
{
    AUDIO_INFO_LOG("address=%{public}s, role=%{public}d",
        GetEncryptAddr(device.macAddress_).c_str(), device.deviceRole_);
    string &activeArmAddr = device.deviceRole_ == INPUT_DEVICE ? activeArmInputAddr_ : activeArmOutputAddr_;
    CHECK_AND_RETURN_LOG(device.macAddress_ == activeArmAddr, "usb device addr not match");
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = audioConfigManager_.GetModuleListByType(ClassType::TYPE_USB, moduleInfoList);
    CHECK_AND_RETURN_LOG(ret, "GetModuleListByType Failed");
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "sink" ? OUTPUT_DEVICE : INPUT_DEVICE;
        if (configRole != device.deviceRole_) {continue;}
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name)) {
            audioIOHandleMap_.ClosePortAndEraseIOHandle(moduleInfo.name);
        }
    }
    activeArmAddr = "";
}

void AudioEcManager::UpdateArmModuleInfo(const string& address, const DeviceRole role, AudioModuleInfo& moduleInfo)
{
    string condition = string("address=") + address + " role=" + to_string(role);
    string deviceInfo = AudioServerProxy::GetInstance().GetAudioParameterProxy(LOCAL_NETWORK_ID, USB_DEVICE,
        condition);
    AUDIO_INFO_LOG("device info from usb hal is %{public}s", deviceInfo.c_str());
    if (!deviceInfo.empty()) {
        GetUsbModuleInfo(deviceInfo, moduleInfo);
        if (isEcFeatureEnable_) {
            uint32_t rateValue = 0;
            uint32_t channelValue = 0;
            CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.rate, rateValue),
                "convert invalid moduleInfo.rate: %{public}s", moduleInfo.rate.c_str());
            CHECK_AND_RETURN_LOG(StringConverter(moduleInfo.channels, channelValue),
                "convert invalid moduleInfo.channels: %{public}s", moduleInfo.channels.c_str());
            uint32_t bufferSize = rateValue * channelValue *
                AudioPolicyUtils::GetInstance().PcmFormatToBytes(formatStrToEnum[moduleInfo.format]) *
                RENDER_FRAME_INTERVAL_IN_SECONDS;
            moduleInfo.bufferSize = std::to_string(bufferSize);
            AUDIO_INFO_LOG("update arm usb buffer size: %{public}s", moduleInfo.bufferSize.c_str());
        }
    }
}

void AudioEcManager::GetTargetSourceTypeAndMatchingFlag(SourceType source,
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

void AudioEcManager::ReloadSourceForSession(SessionInfo sessionInfo)
{
    AUDIO_INFO_LOG("reload session for source: %{public}d", sessionInfo.sourceType);

    PipeStreamPropInfo targetInfo;
    SourceType targetSource = sessionInfo.sourceType;
    int32_t res = FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSource);
    CHECK_AND_RETURN_LOG(res == SUCCESS, "fetch target source info error");

    ReloadNormalSource(sessionInfo, targetInfo, targetSource);

    audioActiveDevice_.UpdateActiveDeviceRoute(audioActiveDevice_.GetCurrentInputDeviceType(),
        DeviceFlag::INPUT_DEVICES_FLAG, audioActiveDevice_.GetCurrentInputDevice().deviceName_,
        audioActiveDevice_.GetCurrentInputDevice().networkId_);
}

int32_t AudioEcManager::FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo, PipeStreamPropInfo &targetInfo,
    SourceType &targetSourceType)
{
    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = nullptr;
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    bool ret = audioConfigManager_.GetAdapterInfoByType(AudioAdapterType::TYPE_PRIMARY, adapterInfo);
    if (ret) {
        pipeInfoPtr = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    }
    CHECK_AND_RETURN_RET_LOG(pipeInfoPtr != nullptr, ERROR, "pipeInfoPtr is null");

    const auto &streamPropInfoList = pipeInfoPtr->streamPropInfos_;

    if (streamPropInfoList.empty()) {
        AUDIO_ERR_LOG("supportedRate or supportedChannels is empty");
        return ERROR;
    }

    // use first profile as default
    std::shared_ptr<PipeStreamPropInfo> targetStreamPropInfo = *streamPropInfoList.begin();
    bool useMatchingPropInfo = false;
    GetTargetSourceTypeAndMatchingFlag(sessionInfo.sourceType, targetSourceType, useMatchingPropInfo);

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

    if (isEcFeatureEnable_) {
        std::shared_ptr<AudioDeviceDescriptor> inputDesc = audioRouterCenter_.FetchInputDevice(targetSourceType, -1);
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
        targetInfo.bufferSize_ = BUFFER_CALC_20MS * targetInfo.sampleRate_ / static_cast<uint32_t>(MS_PER_S)
            * targetInfo.channels_ * sampleFormatBits;
    }
#endif

    return SUCCESS;
}

void AudioEcManager::SetDpSinkModuleInfo(const AudioModuleInfo &moduleInfo)
{
    dpSinkModuleInfo_ = moduleInfo;
}

void AudioEcManager::SetPrimaryMicModuleInfo(const AudioModuleInfo &moduleInfo)
{
    primaryMicModuleInfo_ = moduleInfo;
}

SourceType AudioEcManager::GetSourceOpened()
{
    return normalSourceOpened_;
}

bool AudioEcManager::GetEcFeatureEnable()
{
    return isEcFeatureEnable_;
}

bool AudioEcManager::GetMicRefFeatureEnable()
{
    return isMicRefFeatureEnable_;
}

void AudioEcManager::UpdateStreamEcAndMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType)
{
    UpdateStreamEcInfo(moduleInfo, sourceType);
    UpdateStreamMicRefInfo(moduleInfo, sourceType);
}

std::string AudioEcManager::GetHalNameForDevice(const std::string &role, const DeviceType deviceType)
{
    std::string halName = "";
    std::string portName;
    if (role == ROLE_SOURCE) {
        portName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    } else {
        portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    }
    std::shared_ptr<PolicyAdapterInfo> info;
    bool ret = audioConfigManager_.GetAdapterInfoByType(static_cast<AudioAdapterType>(
        AudioPolicyUtils::portStrToEnum[portName]), info);
    if (ret) {
        halName = info->adapterName;
    }
    AUDIO_INFO_LOG("role: %{public}s, device: %{public}d, halName: %{public}s",
        role.c_str(), deviceType, halName.c_str());
    return halName;
}

void AudioEcManager::SetOpenedNormalSource(SourceType sourceType)
{
    normalSourceOpened_ = sourceType;
}

void AudioEcManager::PrepareNormalSource(AudioModuleInfo &moduleInfo,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    SourceType sourceType = streamDesc->capturerInfo_.sourceType;
    AUDIO_INFO_LOG("prepare normal source for source type: %{public}d", sourceType);
    UpdateEnhanceEffectState(sourceType);
    UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
    SetOpenedNormalSource(sourceType);
    SetOpenedNormalSourceSessionId(streamDesc->sessionId_);
}

void AudioEcManager::SetOpenedNormalSourceSessionId(uint64_t sessionId)
{
    AUDIO_INFO_LOG("set normal source sessionId: %{public}" PRIu64, sessionId);
    sessionIdUsedToOpenSource_ = sessionId;
}

uint64_t AudioEcManager::GetOpenedNormalSourceSessionId()
{
    return sessionIdUsedToOpenSource_;
}

int32_t AudioEcManager::ReloadNormalSource(SessionInfo &sessionInfo,
    PipeStreamPropInfo &targetInfo, SourceType targetSource)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo =
        AudioPipeManager::GetPipeManager()->GetNormalSourceInfo(isEcFeatureEnable_);
    CHECK_AND_RETURN_RET_LOG(pipeInfo != nullptr, ERROR, "Get normal source info failed");

    AudioModuleInfo moduleInfo;
    UpdateEnhanceEffectState(targetSource);
    UpdateStreamCommonInfo(moduleInfo, targetInfo, targetSource);
    UpdateStreamEcInfo(moduleInfo, targetSource);
    UpdateStreamMicRefInfo(moduleInfo, targetSource);

    AUDIO_INFO_LOG("rate: %{public}s, channels: %{public}s, bufferSize: %{public}s format: %{public}s, "
        "sourceType: %{public}s",
        moduleInfo.rate.c_str(), moduleInfo.channels.c_str(), moduleInfo.bufferSize.c_str(),
        moduleInfo.format.c_str(), moduleInfo.sourceType.c_str());

    audioIOHandleMap_.ReloadPortAndUpdateIOHandle(pipeInfo, moduleInfo);
    audioPolicyManager_.SetDeviceActive(audioActiveDevice_.GetCurrentInputDeviceType(), moduleInfo.name,
        true, INPUT_DEVICES_FLAG);

    normalSourceOpened_ = targetSource;
    return SUCCESS;
}
}
}
