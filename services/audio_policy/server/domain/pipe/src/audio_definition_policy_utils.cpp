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
#define LOG_TAG "AudioDefinitionPolicyUtils"
#endif

#include "audio_definition_policy_utils.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
std::unordered_map<std::string, DeviceRole> AudioDefinitionPolicyUtils::deviceRoleStrToEnum = {
    {"input", INPUT_DEVICE},
    {"output", OUTPUT_DEVICE},
};

std::unordered_map<std::string, AudioPipeRole> AudioDefinitionPolicyUtils::pipeRoleStrToEnum = {
    {"input", PIPE_ROLE_INPUT},
    {"output", PIPE_ROLE_OUTPUT},
};

std::unordered_map<std::string, DeviceType> AudioDefinitionPolicyUtils::deviceTypeStrToEnum = {
    {"DEVICE_TYPE_EARPIECE", DEVICE_TYPE_EARPIECE},
    {"DEVICE_TYPE_SPEAKER", DEVICE_TYPE_SPEAKER},
    {"DEVICE_TYPE_WIRED_HEADSET", DEVICE_TYPE_WIRED_HEADSET},
    {"DEVICE_TYPE_WIRED_HEADPHONES", DEVICE_TYPE_WIRED_HEADPHONES},
    {"DEVICE_TYPE_BLUETOOTH_SCO", DEVICE_TYPE_BLUETOOTH_SCO},
    {"DEVICE_TYPE_BLUETOOTH_A2DP", DEVICE_TYPE_BLUETOOTH_A2DP},
    {"DEVICE_TYPE_BLUETOOTH_A2DP_IN", DEVICE_TYPE_BLUETOOTH_A2DP_IN},
    {"DEVICE_TYPE_MIC", DEVICE_TYPE_MIC},
    {"DEVICE_TYPE_WAKEUP", DEVICE_TYPE_WAKEUP},
    {"DEVICE_TYPE_USB_HEADSET", DEVICE_TYPE_USB_HEADSET},
    {"DEVICE_TYPE_DP", DEVICE_TYPE_DP},
    {"DEVICE_TYPE_REMOTE_CAST", DEVICE_TYPE_REMOTE_CAST},
    {"DEVICE_TYPE_HDMI", DEVICE_TYPE_HDMI},
    {"DEVICE_TYPE_LINE_DIGITAL", DEVICE_TYPE_LINE_DIGITAL},
    {"DEVICE_TYPE_FILE_SINK", DEVICE_TYPE_FILE_SINK},
    {"DEVICE_TYPE_FILE_SOURCE", DEVICE_TYPE_FILE_SOURCE},
    {"DEVICE_TYPE_EXTERN_CABLE", DEVICE_TYPE_EXTERN_CABLE},
    {"DEVICE_TYPE_DEFAULT", DEVICE_TYPE_DEFAULT},
    {"DEVICE_TYPE_USB_ARM_HEADSET", DEVICE_TYPE_USB_ARM_HEADSET},
    {"DEVICE_TYPE_WIRED_MICIN", DEVICE_TYPE_WIRED_MICIN},
    {"DEVICE_TYPE_ACCESSORY", DEVICE_TYPE_ACCESSORY},
    {"DEVICE_TYPE_NEARLINK", DEVICE_TYPE_NEARLINK},
    {"DEVICE_TYPE_NEARLINK_IN", DEVICE_TYPE_NEARLINK_IN},
    {"DEVICE_TYPE_HEARING_AID", DEVICE_TYPE_HEARING_AID},
    {"DEVICE_TYPE_REMOTE_DAUDIO", DEVICE_TYPE_REMOTE_DAUDIO},
    {"DEVICE_TYPE_BT_SPP", DEVICE_TYPE_BT_SPP},
};

std::unordered_map<std::string, AudioPin> AudioDefinitionPolicyUtils::pinStrToEnum = {
    {"PIN_OUT_SPEAKER", AUDIO_PIN_OUT_SPEAKER},
    {"PIN_OUT_HEADSET", AUDIO_PIN_OUT_HEADSET},
    {"PIN_OUT_LINEOUT", AUDIO_PIN_OUT_LINEOUT},
    {"PIN_OUT_HDMI", AUDIO_PIN_OUT_HDMI},
    {"PIN_OUT_USB", AUDIO_PIN_OUT_USB},
    {"PIN_OUT_USB_EXT", AUDIO_PIN_OUT_USB_EXT},
    {"PIN_OUT_EARPIECE", AUDIO_PIN_OUT_EARPIECE},
    {"PIN_OUT_BLUETOOTH_SCO", AUDIO_PIN_OUT_BLUETOOTH_SCO},
    {"PIN_OUT_DAUDIO_DEFAULT", AUDIO_PIN_OUT_DAUDIO_DEFAULT},
    {"PIN_OUT_HEADPHONE", AUDIO_PIN_OUT_HEADPHONE},
    {"PIN_OUT_USB_HEADSET", AUDIO_PIN_OUT_USB_HEADSET},
    {"PIN_OUT_BLUETOOTH_A2DP", AUDIO_PIN_OUT_BLUETOOTH_A2DP},
    {"PIN_OUT_DP", AUDIO_PIN_OUT_DP},
    {"PIN_OUT_NEARLINK", AUDIO_PIN_OUT_NEARLINK},
    {"PIN_OUT_HEARING_AID", AUDIO_PIN_OUT_HEARING_AID},
    {"PIN_IN_MIC", AUDIO_PIN_IN_MIC},
    {"PIN_IN_HS_MIC", AUDIO_PIN_IN_HS_MIC},
    {"PIN_IN_LINEIN", AUDIO_PIN_IN_LINEIN},
    {"PIN_IN_USB_EXT", AUDIO_PIN_IN_USB_EXT},
    {"PIN_IN_BLUETOOTH_SCO_HEADSET", AUDIO_PIN_IN_BLUETOOTH_SCO_HEADSET},
    {"PIN_IN_DAUDIO_DEFAULT", AUDIO_PIN_IN_DAUDIO_DEFAULT},
    {"PIN_IN_PENCIL", AUDIO_PIN_IN_PENCIL},
    {"PIN_IN_UWB", AUDIO_PIN_IN_UWB},
    {"PIN_IN_NEARLINK", AUDIO_PIN_IN_NEARLINK},
};

std::unordered_map<std::string, AudioSampleFormat> AudioDefinitionPolicyUtils::formatStrToEnum = {
    {"s16le", SAMPLE_S16LE},
    {"s24le", SAMPLE_S24LE},
    {"s32le", SAMPLE_S32LE},
    {"eac3", INVALID_WIDTH},
};

// for moduleInfo
std::unordered_map<AudioSampleFormat, std::string> AudioDefinitionPolicyUtils::enumToFormatStr = {
    {SAMPLE_S16LE, "s16le"},
    {SAMPLE_S24LE, "s24le"},
    {SAMPLE_S32LE, "s32le"},
};

std::unordered_map<std::string, AudioChannelLayout> AudioDefinitionPolicyUtils::layoutStrToEnum = {
    {"CH_LAYOUT_MONO", CH_LAYOUT_MONO},
    {"CH_LAYOUT_STEREO", CH_LAYOUT_STEREO},
    {"CH_LAYOUT_STEREO_DOWNMIX", CH_LAYOUT_STEREO_DOWNMIX},
    {"CH_LAYOUT_2POINT1", CH_LAYOUT_2POINT1},
    {"CH_LAYOUT_3POINT0", CH_LAYOUT_3POINT0},
    {"CH_LAYOUT_SURROUND", CH_LAYOUT_SURROUND},
    {"CH_LAYOUT_3POINT1", CH_LAYOUT_3POINT1},
    {"CH_LAYOUT_4POINT0", CH_LAYOUT_4POINT0},
    {"CH_LAYOUT_QUAD_SIDE", CH_LAYOUT_QUAD_SIDE},
    {"CH_LAYOUT_QUAD", CH_LAYOUT_QUAD},
    {"CH_LAYOUT_2POINT0POINT2", CH_LAYOUT_2POINT0POINT2},
    {"CH_LAYOUT_4POINT1", CH_LAYOUT_4POINT1},
    {"CH_LAYOUT_5POINT0", CH_LAYOUT_5POINT0},
    {"CH_LAYOUT_5POINT0_BACK", CH_LAYOUT_5POINT0_BACK},
    {"CH_LAYOUT_2POINT1POINT2", CH_LAYOUT_2POINT1POINT2},
    {"CH_LAYOUT_3POINT0POINT2", CH_LAYOUT_3POINT0POINT2},
    {"CH_LAYOUT_5POINT1", CH_LAYOUT_5POINT1},
    {"CH_LAYOUT_5POINT1_BACK", CH_LAYOUT_5POINT1_BACK},
    {"CH_LAYOUT_6POINT0", CH_LAYOUT_6POINT0},
    {"CH_LAYOUT_HEXAGONAL", CH_LAYOUT_HEXAGONAL},
    {"CH_LAYOUT_3POINT1POINT2", CH_LAYOUT_3POINT1POINT2},
    {"CH_LAYOUT_6POINT0_FRONT", CH_LAYOUT_6POINT0_FRONT},
    {"CH_LAYOUT_6POINT1", CH_LAYOUT_6POINT1},
    {"CH_LAYOUT_6POINT1_BACK", CH_LAYOUT_6POINT1_BACK},
    {"CH_LAYOUT_6POINT1_FRONT", CH_LAYOUT_6POINT1_FRONT},
    {"CH_LAYOUT_7POINT0", CH_LAYOUT_7POINT0},
    {"CH_LAYOUT_7POINT0_FRONT", CH_LAYOUT_7POINT0_FRONT},
    {"CH_LAYOUT_7POINT1", CH_LAYOUT_7POINT1},
    {"CH_LAYOUT_OCTAGONAL", CH_LAYOUT_OCTAGONAL},
    {"CH_LAYOUT_5POINT1POINT2", CH_LAYOUT_5POINT1POINT2},
    {"CH_LAYOUT_7POINT1_WIDE", CH_LAYOUT_7POINT1_WIDE},
    {"CH_LAYOUT_7POINT1_WIDE_BACK", CH_LAYOUT_7POINT1_WIDE_BACK},
    {"CH_LAYOUT_5POINT1POINT4", CH_LAYOUT_5POINT1POINT4},
    {"CH_LAYOUT_7POINT1POINT2", CH_LAYOUT_7POINT1POINT2},
    {"CH_LAYOUT_7POINT1POINT4", CH_LAYOUT_7POINT1POINT4},
    {"CH_LAYOUT_10POINT2", CH_LAYOUT_10POINT2},
    {"CH_LAYOUT_9POINT1POINT4", CH_LAYOUT_9POINT1POINT4},
    {"CH_LAYOUT_9POINT1POINT6", CH_LAYOUT_9POINT1POINT6},
    {"CH_LAYOUT_HEXADECAGONAL", CH_LAYOUT_HEXADECAGONAL},
    {"CH_LAYOUT_HOA_ORDER1_ACN_N3D", CH_LAYOUT_HOA_ORDER1_ACN_N3D},
    {"CH_LAYOUT_HOA_ORDER1_ACN_SN3D", CH_LAYOUT_HOA_ORDER1_ACN_SN3D},
    {"CH_LAYOUT_HOA_ORDER1_FUMA", CH_LAYOUT_HOA_ORDER1_FUMA},
    {"CH_LAYOUT_HOA_ORDER2_ACN_N3D", CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {"CH_LAYOUT_HOA_ORDER2_ACN_SN3D", CH_LAYOUT_HOA_ORDER2_ACN_SN3D},
    {"CH_LAYOUT_HOA_ORDER2_FUMA", CH_LAYOUT_HOA_ORDER2_FUMA},
    {"CH_LAYOUT_HOA_ORDER3_ACN_N3D", CH_LAYOUT_HOA_ORDER3_ACN_N3D},
    {"CH_LAYOUT_HOA_ORDER3_ACN_SN3D", CH_LAYOUT_HOA_ORDER3_ACN_SN3D},
    {"CH_LAYOUT_HOA_ORDER3_FUMA", CH_LAYOUT_HOA_ORDER3_FUMA},
};

std::unordered_map<std::string, AudioFlag> AudioDefinitionPolicyUtils::flagStrToEnum = {
    {"AUDIO_OUTPUT_FLAG_NORMAL", AUDIO_OUTPUT_FLAG_NORMAL},
    {"AUDIO_OUTPUT_FLAG_DIRECT", AUDIO_OUTPUT_FLAG_DIRECT},
    {"AUDIO_OUTPUT_FLAG_HD", AUDIO_OUTPUT_FLAG_HD},
    {"AUDIO_OUTPUT_FLAG_MULTICHANNEL", AUDIO_OUTPUT_FLAG_MULTICHANNEL},
    {"AUDIO_OUTPUT_FLAG_LOWPOWER", AUDIO_OUTPUT_FLAG_LOWPOWER},
    {"AUDIO_OUTPUT_FLAG_FAST", AUDIO_OUTPUT_FLAG_FAST},
    {"AUDIO_OUTPUT_FLAG_VOIP", AUDIO_OUTPUT_FLAG_VOIP},
    {"AUDIO_OUTPUT_FLAG_VOIP_FAST", AUDIO_OUTPUT_FLAG_VOIP_FAST},
    {"AUDIO_OUTPUT_FLAG_HWDECODING", AUDIO_OUTPUT_FLAG_HWDECODING},
    {"AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD", AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD},
    {"AUDIO_INPUT_FLAG_NORMAL", AUDIO_INPUT_FLAG_NORMAL},
    {"AUDIO_INPUT_FLAG_FAST", AUDIO_INPUT_FLAG_FAST},
    {"AUDIO_INPUT_FLAG_VOIP", AUDIO_INPUT_FLAG_VOIP},
    {"AUDIO_INPUT_FLAG_VOIP_FAST", AUDIO_INPUT_FLAG_VOIP_FAST},
    {"AUDIO_INPUT_FLAG_WAKEUP", AUDIO_INPUT_FLAG_WAKEUP},
    {"AUDIO_INPUT_FLAG_AI", AUDIO_INPUT_FLAG_AI},
    {"AUDIO_INPUT_FLAG_UNPROCESS", AUDIO_INPUT_FLAG_UNPROCESS},
    {"AUDIO_INPUT_FLAG_LIVE", AUDIO_INPUT_FLAG_LIVE},
    {"AUDIO_INPUT_FLAG_ULTRASONIC", AUDIO_INPUT_FLAG_ULTRASONIC},
    {"AUDIO_INPUT_FLAG_VOICE_RECOGNITION", AUDIO_INPUT_FLAG_VOICE_RECOGNITION},
    {"AUDIO_INPUT_FLAG_RAW_AI", AUDIO_INPUT_FLAG_RAW_AI},
    {"AUDIO_INPUT_FLAG_INTERPHONE", AUDIO_INPUT_FLAG_INTERPHONE},
    {"AUDIO_OUTPUT_FLAG_INTERPHONE", AUDIO_OUTPUT_FLAG_INTERPHONE},
    {"AUDIO_INPUT_FLAG_CAMCORDER", AUDIO_INPUT_FLAG_CAMCORDER},
    {"AUDIO_INPUT_FLAG_VOICE_MESSAGE", AUDIO_INPUT_FLAG_VOICE_MESSAGE},
    {"AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT", AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT},
    {"AUDIO_OUTPUT_FLAG_3DA_DIRECT", AUDIO_OUTPUT_FLAG_3DA_DIRECT},
};

std::unordered_map<std::string, AudioPreloadType> AudioDefinitionPolicyUtils::preloadStrToEnum = {
    {"false", PRELOAD_TYPE_NOTSUPPORT},
    {"true", PRELOAD_TYPE_SUPPORT},
};

std::unordered_map<std::string, uint32_t> AudioDefinitionPolicyUtils::usageStrToEnum = {
    {"AUDIO_USAGE_NORMAL", AUDIO_USAGE_NORMAL},
    {"AUDIO_USAGE_VOIP", AUDIO_USAGE_VOIP},
};

std::unordered_map<std::string, bool> AudioDefinitionPolicyUtils::supModeStrToBool = {
    {"false", false},
    {"true", true},
};

uint32_t AudioDefinitionPolicyUtils::PcmFormatToBytes(AudioSampleFormat format)
{
    // AudioSampleFormat / PCM_8_BIT
    switch (format) {
        case SAMPLE_U8:
            return 1; // 1 byte
        case SAMPLE_S16LE:
            return 2; // 2 byte
        case SAMPLE_S24LE:
            return 3; // 3 byte
        case SAMPLE_S32LE:
            return 4; // 4 byte
        case SAMPLE_F32LE:
            return 4; // 4 byte
        default:
            return 2; // 2 byte
    }
}

DeviceType AudioDefinitionPolicyUtils::GetDeviceTypeFromStr(const std::string &deviceTypeStr)
{
    if (deviceTypeStrToEnum.contains(deviceTypeStr)) {
        return deviceTypeStrToEnum.at(deviceTypeStr);
    }
    return DEVICE_TYPE_NONE;
}

static void SelectBestRate(uint32_t appRate, const std::set<uint32_t> &deviceRates,
    AudioSamplingRate &result)
{
    CHECK_AND_RETURN(!deviceRates.empty());
    uint32_t selected = *deviceRates.begin();
    for (const auto &rate : deviceRates) {
        if (rate <= appRate) {
            selected = rate;
        }
    }
    result = static_cast<AudioSamplingRate>(selected);
}

static void SelectBestChannel(AudioChannel appCh, const std::set<AudioChannelLayout> &deviceLayouts,
    AudioChannelLayout &result)
{
    CHECK_AND_RETURN(!deviceLayouts.empty());
    uint32_t appChVal = static_cast<uint32_t>(appCh);
    uint32_t maxLe = 0;
    // layout -> channel, choose the best one
    for (const auto &layout : deviceLayouts) {
        uint32_t ch = static_cast<uint32_t>(ConvertLayoutToAudioChannel(layout));
        if (ch == appChVal) {
            result = layout;
            return;
        }
        if (ch <= appChVal && ch > maxLe) {
            maxLe = ch;
        }
    }
    if (maxLe > 0) {
        for (const auto &layout : deviceLayouts) {
            if (static_cast<uint32_t>(ConvertLayoutToAudioChannel(layout)) == maxLe) {
                result = layout;
                return;
            }
        }
    }
    result = *deviceLayouts.begin();
}

SelectedDeviceFormat AudioDefinitionPolicyUtils::SelectBestOutputFormat(const AudioStreamInfo &appStreamInfo,
    const std::list<DeviceStreamInfo> &deviceCaps)
{
    SelectedDeviceFormat result;
    std::set<uint32_t> deviceRates;
    std::set<AudioChannelLayout> deviceLayouts;
    bool hasFormat = false;
    for (const auto &cap : deviceCaps) {
        for (const auto &rate : cap.samplingRate) {
            deviceRates.insert(static_cast<uint32_t>(rate));
        }
        if (cap.format != AudioSampleFormat::INVALID_WIDTH) {
            if (!hasFormat || cap.format > result.format) {
                result.format = cap.format;
            }
            hasFormat = true;
        }
        for (const auto &layout : cap.channelLayout) {
            deviceLayouts.insert(layout);
        }
    }
    if (deviceRates.empty() && !hasFormat && deviceLayouts.empty()) {
        return result;
    }
    uint32_t appRate = appStreamInfo.GetEffectiveSampleRate();
    SelectBestRate(appRate, deviceRates, result.rate);
    SelectBestChannel(appStreamInfo.channels, deviceLayouts, result.channelLayout);
    AUDIO_INFO_LOG("SelectBestOutputFormat: appRate=%{public}u, rate=%{public}u, format=%{public}u, layout=%{public}u",
        appRate, static_cast<uint32_t>(result.rate), static_cast<uint32_t>(result.format),
        static_cast<uint32_t>(result.channelLayout));
    return result;
}

bool AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(const std::string &macAddress,
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streams,
    const std::shared_ptr<AudioStreamDescriptor> &extra, AudioStreamInfo &highestInfo)
{
    uint32_t highestRate = 0;
    bool found = false;
    auto checkStream = [&macAddress, &highestRate, &highestInfo, &found](
        const std::shared_ptr<AudioStreamDescriptor> &stream) {
        CHECK_AND_RETURN(stream != nullptr && stream->IsPlayback() && !stream->newDeviceDescs_.empty());
        auto dev = stream->newDeviceDescs_[0];
        CHECK_AND_RETURN(dev != nullptr && dev->macAddress_ == macAddress);
        uint32_t rate = stream->streamInfo_.GetEffectiveSampleRate();
        if (rate > highestRate || (rate == highestRate && stream->streamInfo_.format > highestInfo.format)) {
            highestRate = rate;
            highestInfo = stream->streamInfo_;
            found = true;
        }
    };
    for (const auto &stream : streams) {
        checkStream(stream);
    }
    if (extra != nullptr) {
        checkStream(extra);
    }
    if (found) {
        AUDIO_INFO_LOG("FindHighestFormatStreamOnDevice: mac=%{public}s, rate=%{public}u, format=%{public}u",
            macAddress.c_str(), highestRate, static_cast<uint32_t>(highestInfo.format));
    }
    return found;
}
}
}
