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
#define LOG_TAG "AudioPolicyConfigManager"
#endif

#include "audio_policy_config_manager.h"
#include "audio_policy_config_parser.h"
#include "audio_policy_utils.h"
#include "audio_policy_service.h"
#include "audio_ec_manager.h"

namespace OHOS {
namespace AudioStandard {

constexpr int32_t MS_PER_S = 1000;
static const unsigned int BUFFER_CALC_20MS = 20;
static const char* MAX_RENDERERS_NAME = "maxRenderers";
static const char* MAX_CAPTURERS_NAME = "maxCapturers";
static const char* MAX_FAST_RENDERERS_NAME = "maxFastRenderers";

const int32_t DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES = 128;
const int32_t DEFAULT_MAX_INPUT_NORMAL_INSTANCES = 16;
const int32_t DEFAULT_MAX_FAST_NORMAL_INSTANCES = 6;

const uint32_t PC_MIC_CHANNEL_NUM = 4;
const uint32_t HEADPHONE_CHANNEL_NUM = 2;

bool AudioPolicyConfigManager::Init(bool isRefresh)
{
    if (xmlHasLoaded_ && !isRefresh) {
        AUDIO_WARNING_LOG("Unexpected Duplicate Load AudioPolicyConfig!");
        return false;
    }
    std::unique_ptr<AudioPolicyConfigParser> audioPolicyConfigParser = make_unique<AudioPolicyConfigParser>(this);
    CHECK_AND_RETURN_RET_LOG(audioPolicyConfigParser != nullptr, false, "AudioPolicyConfigParser create failed");
    bool ret = audioPolicyConfigParser->LoadConfiguration();
    if (!ret) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Policy Config Load Configuration failed");
        AUDIO_ERR_LOG("Audio Policy Config Load Configuration failed");
        return ret;
    }
    xmlHasLoaded_ = true;
    return ret;
}

void AudioPolicyConfigManager::OnAudioPolicyConfigXmlParsingCompleted()
{
    AUDIO_INFO_LOG("AdapterInfo num [%{public}zu]", audioPolicyConfig_.adapterInfoMap.size());
    CHECK_AND_RETURN_LOG(!audioPolicyConfig_.adapterInfoMap.empty(),
        "Parse audio policy xml failed, received data is empty");

    audioPolicyConfig_.Reorganize();

    isAdapterInfoMap_.store(true);

    OnHasEarpiece();
}

void AudioPolicyConfigManager::OnXmlParsingCompleted(
    const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmlData)
{
    AUDIO_INFO_LOG("device class num [%{public}zu]", xmlData.size());
    CHECK_AND_RETURN_LOG(!xmlData.empty(), "failed to parse xml file. Received data is empty");

    deviceClassInfo_ = xmlData;
}

void AudioPolicyConfigManager::OnAudioLatencyParsed(uint64_t latency)
{
    audioLatencyInMsec_ = latency;
}

void AudioPolicyConfigManager::OnSinkLatencyParsed(uint32_t latency)
{
    sinkLatencyInMsec_ = latency;
}

void AudioPolicyConfigManager::OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData)
{
    AUDIO_INFO_LOG("group data num [%{public}zu]", volumeGroupData.size());
    CHECK_AND_RETURN_LOG(!volumeGroupData.empty(), "failed to parse xml file. Received data is empty");

    volumeGroupData_ = volumeGroupData;
}

void AudioPolicyConfigManager::OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData)
{
    AUDIO_INFO_LOG("group data num [%{public}zu]", interruptGroupData.size());
    CHECK_AND_RETURN_LOG(!interruptGroupData.empty(), "failed to parse xml file. Received data is empty");

    interruptGroupData_ = interruptGroupData;
}

void AudioPolicyConfigManager::OnUpdateRouteSupport(bool isSupported)
{
    isUpdateRouteSupported_ = isSupported;
}

void AudioPolicyConfigManager::OnUpdateDefaultAdapter(bool isEnable)
{
    isDefaultAdapterEnable_ = isEnable;
}

bool AudioPolicyConfigManager::GetDefaultAdapterEnable()
{
    return isDefaultAdapterEnable_;
}

void AudioPolicyConfigManager::OnGlobalConfigsParsed(PolicyGlobalConfigs &globalConfigs)
{
    globalConfigs_ = globalConfigs;
}

void AudioPolicyConfigManager::OnVoipConfigParsed(bool enableFastVoip)
{
    enableFastVoip_ = enableFastVoip;
}

void AudioPolicyConfigManager::OnUpdateAnahsSupport(std::string anahsShowType)
{
    AUDIO_INFO_LOG("OnUpdateAnahsSupport show type: %{public}s", anahsShowType.c_str());
    AudioPolicyService::GetAudioPolicyService().OnUpdateAnahsSupport(anahsShowType);
}

void AudioPolicyConfigManager::OnUpdateEac3Support(bool isSupported)
{
    isSupportEac3_ = isSupported;
}

void AudioPolicyConfigManager::OnHasEarpiece()
{
    for (const auto &adapterInfo : audioPolicyConfig_.adapterInfoMap) {
        hasEarpiece_ = std::any_of(adapterInfo.second->deviceInfos.begin(), adapterInfo.second->deviceInfos.end(),
            [](const auto& deviceInfo) {
                return deviceInfo->type_ == DEVICE_TYPE_EARPIECE;
            });
        if (hasEarpiece_) {
            break;
        }
    }
    audioDeviceManager_.UpdateEarpieceStatus(hasEarpiece_);
}

void AudioPolicyConfigManager::SetNormalVoipFlag(const bool &normalVoipFlag)
{
    normalVoipFlag_ = normalVoipFlag;
}


bool AudioPolicyConfigManager::GetNormalVoipFlag()
{
    return normalVoipFlag_;
}

bool AudioPolicyConfigManager::GetModuleListByType(ClassType type, std::list<AudioModuleInfo>& moduleList)
{
    auto modulesPos = deviceClassInfo_.find(type);
    if (modulesPos != deviceClassInfo_.end()) {
        moduleList = modulesPos->second;
        return true;
    }
    return false;
}

void AudioPolicyConfigManager::GetDeviceClassInfo(
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo)
{
    deviceClassInfo = deviceClassInfo_;
}

std::string AudioPolicyConfigManager::GetGroupName(const std::string& deviceName, const GroupType type)
{
    std::string groupName = GROUP_NAME_NONE;
    if (type == VOLUME_TYPE) {
        auto iter = volumeGroupData_.find(deviceName);
        if (iter != volumeGroupData_.end()) {
            groupName = iter->second;
        }
    } else {
        auto iter = interruptGroupData_.find(deviceName);
        if (iter != interruptGroupData_.end()) {
            groupName = iter->second;
        }
    }
    return groupName;
}

int32_t AudioPolicyConfigManager::GetMaxRendererInstances()
{
    for (auto commonConfig : globalConfigs_.commonConfigs_) {
        if (commonConfig.name_ != MAX_RENDERERS_NAME) {
            continue;
        }
        int32_t convertValue = 0;
        AUDIO_INFO_LOG("Max output normal instance is %{public}s", commonConfig.value_.c_str());
        CHECK_AND_RETURN_RET_LOG(StringConverter(commonConfig.value_, convertValue),
            DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES,
            "convert invalid configInfo.value_: %{public}s", commonConfig.value_.c_str());
        return convertValue;
    }
    return DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES;
}

int32_t AudioPolicyConfigManager::GetMaxCapturersInstances()
{
    for (auto commonConfig : globalConfigs_.commonConfigs_) {
        if (commonConfig.name_ != MAX_CAPTURERS_NAME) {
            continue;
        }
        int32_t convertValue = 0;
        AUDIO_INFO_LOG("Max input normal instance is %{public}s", commonConfig.value_.c_str());
        CHECK_AND_RETURN_RET_LOG(StringConverter(commonConfig.value_, convertValue),
            DEFAULT_MAX_INPUT_NORMAL_INSTANCES,
            "convert invalid configInfo.value_: %{public}s", commonConfig.value_.c_str());
        return convertValue;
    }
    return DEFAULT_MAX_INPUT_NORMAL_INSTANCES;
}

int32_t AudioPolicyConfigManager::GetMaxFastRenderersInstances()
{
    for (auto commonConfig : globalConfigs_.commonConfigs_) {
        if (commonConfig.name_ != MAX_FAST_RENDERERS_NAME) {
            continue;
        }
        int32_t convertValue = 0;
        AUDIO_INFO_LOG("Max Fast Renderer instance is %{public}s", commonConfig.value_.c_str());
        CHECK_AND_RETURN_RET_LOG(StringConverter(commonConfig.value_, convertValue),
            DEFAULT_MAX_FAST_NORMAL_INSTANCES,
            "convert invalid configInfo.value_: %{public}s", commonConfig.value_.c_str());
        return convertValue;
    }
    return DEFAULT_MAX_FAST_NORMAL_INSTANCES;
}

int32_t AudioPolicyConfigManager::GetVoipRendererFlag(const std::string &sinkPortName, const std::string &networkId,
    const AudioSamplingRate &samplingRate)
{
    // VoIP stream has three mode for different products.
    if (enableFastVoip_ && (sinkPortName == PRIMARY_SPEAKER && networkId == LOCAL_NETWORK_ID)) {
        if (samplingRate != SAMPLE_RATE_48000 && samplingRate != SAMPLE_RATE_16000) {
            return AUDIO_FLAG_NORMAL;
        }
        return AUDIO_FLAG_VOIP_FAST;
    } else if (!normalVoipFlag_ && (sinkPortName == PRIMARY_SPEAKER) && (networkId == LOCAL_NETWORK_ID)) {
        AUDIO_INFO_LOG("Direct VoIP mode is supported for the device");
        return AUDIO_FLAG_VOIP_DIRECT;
    }

    return AUDIO_FLAG_NORMAL;
}

int32_t AudioPolicyConfigManager::GetAudioLatencyFromXml() const
{
    return audioLatencyInMsec_;
}

uint32_t AudioPolicyConfigManager::GetSinkLatencyFromXml() const
{
    return sinkLatencyInMsec_;
}

void AudioPolicyConfigManager::GetAudioAdapterInfos(
    std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>> &adapterInfoMap)
{
    adapterInfoMap = audioPolicyConfig_.adapterInfoMap;
}

void AudioPolicyConfigManager::GetVolumeGroupData(std::unordered_map<std::string, std::string>& volumeGroupData)
{
    volumeGroupData = volumeGroupData_;
}

void AudioPolicyConfigManager::GetInterruptGroupData(std::unordered_map<std::string, std::string>& interruptGroupData)
{
    interruptGroupData = interruptGroupData_;
}

void AudioPolicyConfigManager::GetGlobalConfigs(PolicyGlobalConfigs &globalConfigs)
{
    globalConfigs = globalConfigs_;
}

bool AudioPolicyConfigManager::GetVoipConfig()
{
    return enableFastVoip_;
}

bool AudioPolicyConfigManager::GetUpdateRouteSupport()
{
    return isUpdateRouteSupported_;
}

bool AudioPolicyConfigManager::GetAdapterInfoFlag()
{
    return isAdapterInfoMap_.load();
}

bool AudioPolicyConfigManager::GetAdapterInfoByType(AudioAdapterType type, std::shared_ptr<PolicyAdapterInfo> &info)
{
    auto it = audioPolicyConfig_.adapterInfoMap.find(type);
    if (it == audioPolicyConfig_.adapterInfoMap.end()) {
        AUDIO_ERR_LOG("can not find adapter info");
        return false;
    }
    info = it->second;
    return true;
}

bool AudioPolicyConfigManager::GetHasEarpiece()
{
    return hasEarpiece_;
}

uint32_t AudioPolicyConfigManager::GetRouteFlag(std::shared_ptr<AudioStreamDescriptor> &desc)
{
    // device -> adapter -> flag -> stream
    uint32_t flag = AUDIO_FLAG_NONE; // input or output? default?
    auto newDeviceDesc = desc->newDeviceDescs_.front();
    std::shared_ptr<AdapterDeviceInfo> deviceInfo = audioPolicyConfig_.GetAdapterDeviceInfo(newDeviceDesc->deviceType_,
        newDeviceDesc->deviceRole_, newDeviceDesc->networkId_, desc->audioFlag_, newDeviceDesc->a2dpOffloadFlag_);
    CHECK_AND_RETURN_RET_LOG(deviceInfo != nullptr, flag, "Find device failed; use none flag");

    for (auto &pipeIt : deviceInfo->supportPipeMap_) {
        if ((desc->audioMode_ == static_cast<AudioMode>(pipeIt.second->role_)) && (desc->audioFlag_ & pipeIt.first)) {
            flag = pipeIt.first;
            break;
        }
    }
    if (flag == AUDIO_FLAG_NONE) {
        flag = desc->audioMode_ == AUDIO_MODE_PLAYBACK ?
            AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL;
    }
    AUDIO_INFO_LOG("flag:0x%{public}x, target flag:0x%{public}x", desc->audioFlag_, flag);
    return flag;
}

void AudioPolicyConfigManager::GetTargetSourceTypeAndMatchingFlag(SourceType source, bool &useMatchingPropInfo)
{
    switch (source) {
        case SOURCE_TYPE_VOICE_RECOGNITION:
            useMatchingPropInfo = true;
            break;
        case SOURCE_TYPE_VOICE_COMMUNICATION:
        case SOURCE_TYPE_VOICE_TRANSCRIPTION:
            useMatchingPropInfo = AudioEcManager::GetInstance().GetEcFeatureEnable() ? false : true;
            break;
        case SOURCE_TYPE_VOICE_CALL:
            break;
        case SOURCE_TYPE_CAMCORDER:
            break;
        case SOURCE_TYPE_UNPROCESSED:
            break;
        default:
            break;
    }
}

void AudioPolicyConfigManager::HandleGetStreamPropInfoForRecord(
    std::shared_ptr<AudioStreamDescriptor> &desc, std::shared_ptr<AdapterPipeInfo> &pipeInfo,
    std::shared_ptr<PipeStreamPropInfo> &info, const AudioChannel &tempChannel)
{
    //if not match, choose first
    info = pipeInfo->streamPropInfos_.front();
    bool useMatchingPropInfo = false;
    GetTargetSourceTypeAndMatchingFlag(desc->capturerInfo_.sourceType, useMatchingPropInfo);
    if (useMatchingPropInfo) {
        auto streamProp = GetStreamPropInfoFromPipe(pipeInfo, desc->streamInfo_.format,
            desc->streamInfo_.samplingRate, tempChannel);
        if (streamProp != nullptr) {
            info = streamProp;
        }
    }

    if (AudioEcManager::GetInstance().GetEcFeatureEnable()) {
        if (desc->newDeviceDescs_.front() != nullptr &&
            desc->newDeviceDescs_.front()->deviceType_ != DEVICE_TYPE_MIC &&
            info->channelLayout_ == PC_MIC_CHANNEL_NUM) {
            // only built-in mic can use 4 channel, update later by using xml to describe
            info->channels_ = static_cast<AudioChannel>(HEADPHONE_CHANNEL_NUM);
            info->channelLayout_ = CH_LAYOUT_STEREO;
        }
    }

#ifndef IS_EMULATOR
    // need change to use profile for all devices later
    if (isUpdateRouteSupported_) {
        uint32_t sampleFormatBits = AudioPolicyUtils::GetInstance().PcmFormatToBytes(info->format_);
        info->bufferSize_ = BUFFER_CALC_20MS * info->sampleRate_ / static_cast<uint32_t>(MS_PER_S)
            * info->channels_ * sampleFormatBits;
    }
#endif
}

void AudioPolicyConfigManager::GetStreamPropInfo(std::shared_ptr<AudioStreamDescriptor> &desc,
    std::shared_ptr<PipeStreamPropInfo> &info)
{
    auto newDeviceDesc = desc->newDeviceDescs_.front();
    std::shared_ptr<AdapterDeviceInfo> deviceInfo = audioPolicyConfig_.GetAdapterDeviceInfo(newDeviceDesc->deviceType_,
        newDeviceDesc->deviceRole_, newDeviceDesc->networkId_, desc->audioFlag_, newDeviceDesc->a2dpOffloadFlag_);
    CHECK_AND_RETURN_LOG(deviceInfo != nullptr, "Find device failed, none streamProp");

    auto pipeIt = deviceInfo->supportPipeMap_.find(desc->routeFlag_);
    CHECK_AND_RETURN_LOG(pipeIt != deviceInfo->supportPipeMap_.end(), "Find pipeInfo failed;none streamProp");

    AudioChannel tempChannel = desc->streamInfo_.channels;
    if ((desc->routeFlag_ == (AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST)) ||
        (desc->routeFlag_ == (AUDIO_OUTPUT_FLAG_VOIP | AUDIO_OUTPUT_FLAG_FAST))) {
        tempChannel = desc->streamInfo_.channels == MONO ? STEREO : desc->streamInfo_.channels;
    }

    if (desc->audioMode_ == AUDIO_MODE_RECORD) {
        HandleGetStreamPropInfoForRecord(desc, pipeIt->second, info, tempChannel);
        return;
    }

    auto streamProp = GetStreamPropInfoFromPipe(pipeIt->second, desc->streamInfo_.format,
        desc->streamInfo_.samplingRate, tempChannel);
    if (streamProp != nullptr) {
        info = streamProp;
        return;
    }

    if (SupportImplicitConversion(desc->routeFlag_)) {
        AUDIO_INFO_LOG("Select first attribute");
        info = pipeIt->second->streamPropInfos_.front();
    }

    if (info->format_ == INVALID_WIDTH && info->sampleRate_ == 0 && info->channelLayout_ == CH_LAYOUT_UNKNOWN &&
        desc->routeFlag_ != (AUDIO_OUTPUT_FLAG_NORMAL || AUDIO_INPUT_FLAG_NORMAL)) {
        AUDIO_INFO_LOG("Find streamPropInfo failed, choose normal flag");
        desc->routeFlag_ = desc->audioMode_ == AUDIO_MODE_PLAYBACK ?
            AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL;
        auto streamProp = GetStreamPropInfoFromPipe(pipeIt->second, desc->streamInfo_.format,
            desc->streamInfo_.samplingRate, desc->streamInfo_.channels);
        if (streamProp != nullptr) {
            info = streamProp;
            return;
        }
    }
    if (info->format_ == INVALID_WIDTH && info->sampleRate_ == 0 && info->channelLayout_ == CH_LAYOUT_UNKNOWN &&
        desc->routeFlag_ == (AUDIO_OUTPUT_FLAG_NORMAL || AUDIO_INPUT_FLAG_NORMAL) &&
        !pipeIt->second->streamPropInfos_.empty()) {
        info = pipeIt->second->streamPropInfos_.front();
    } // if not match, choose first?
}

std::shared_ptr<PipeStreamPropInfo> AudioPolicyConfigManager::GetStreamPropInfoFromPipe(
    std::shared_ptr<AdapterPipeInfo> &info, AudioSampleFormat format, uint32_t sampleRate, AudioChannel channels)
{
    for (auto &streamProp : info->streamPropInfos_) {
        if (streamProp->format_ == format &&
            streamProp->sampleRate_ == sampleRate &&
            streamProp->channels_ == channels) {
            return streamProp;
        }
    }
    return nullptr;
}

bool AudioPolicyConfigManager::SupportImplicitConversion(uint32_t routeFlag)
{
    if ((routeFlag & AUDIO_OUTPUT_FLAG_NORMAL) ||
        ((routeFlag & AUDIO_OUTPUT_FLAG_DIRECT) && (routeFlag & AUDIO_OUTPUT_FLAG_HD)) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_MULTICHANNEL) ||
        (routeFlag & AUDIO_OUTPUT_FLAG_LOWPOWER) ||
        (routeFlag & AUDIO_INPUT_FLAG_NORMAL) ||
        (routeFlag & AUDIO_INPUT_FLAG_WAKEUP)) {
        return true;
    }
    return false;
}

DirectPlaybackMode AudioPolicyConfigManager::GetDirectPlaybackSupport(std::shared_ptr<AudioDeviceDescriptor> desc,
    const AudioStreamInfo &streamInfo)
{
    std::shared_ptr<AdapterDeviceInfo> deviceInfo = audioPolicyConfig_.GetAdapterDeviceInfo(
        desc->deviceType_, desc->deviceRole_, desc->networkId_, AUDIO_FLAG_NONE);
    CHECK_AND_RETURN_RET_LOG(deviceInfo != nullptr, DIRECT_PLAYBACK_NOT_SUPPORTED, "Find device failed");
    CHECK_AND_RETURN_RET_LOG(streamInfo.encoding != ENCODING_EAC3 || desc->deviceType_ == DEVICE_TYPE_HDMI ||
        desc->deviceType_ == DEVICE_TYPE_LINE_DIGITAL, DIRECT_PLAYBACK_NOT_SUPPORTED, "Not support eac3");

    if ((streamInfo.encoding == ENCODING_EAC3) &&
        (desc->deviceType_ == DEVICE_TYPE_HDMI || desc->deviceType_ == DEVICE_TYPE_LINE_DIGITAL)) {
        for (auto &pipeIt : deviceInfo->supportPipeMap_) {
            if (pipeIt.second != nullptr && pipeIt.second->supportEncodingEac3_ &&
                IsStreamPropMatch(streamInfo, pipeIt.second->streamPropInfos_)) {
                AUDIO_INFO_LOG("Support encoding type eac3");
                return DIRECT_PLAYBACK_BITSTREAM_SUPPORTED;
            }
        }
        AUDIO_INFO_LOG("Not support eac3");
    }

    if (streamInfo.encoding == ENCODING_PCM) {
        for (auto &pipeIt : deviceInfo->supportPipeMap_) {
            if ((pipeIt.first & AUDIO_OUTPUT_FLAG_DIRECT) && pipeIt.second != nullptr &&
                IsStreamPropMatch(streamInfo, pipeIt.second->streamPropInfos_)) {
                AUDIO_INFO_LOG("Support encoding type pcm");
                return DIRECT_PLAYBACK_PCM_SUPPORTED;
            }
        }
        AUDIO_INFO_LOG("Not support pcm");
    }

    return DIRECT_PLAYBACK_NOT_SUPPORTED;
}

bool AudioPolicyConfigManager::IsStreamPropMatch(const AudioStreamInfo &streamInfo,
    std::list<std::shared_ptr<PipeStreamPropInfo>> &infos)
{
    for (auto info : infos) {
        if (info != nullptr && info->format_ == streamInfo.format && info->sampleRate_ == streamInfo.samplingRate &&
            info->channels_ == streamInfo.channels) {
            return true;
        }
    }
    return false;
}
}
}
