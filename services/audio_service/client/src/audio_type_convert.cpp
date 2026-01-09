/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_type_convert.h"
#include "audio_log.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {

void OtherDeviceTypeCases(DeviceType deviceType)
{
    switch (deviceType) {
        case OHOS::AudioStandard::DEVICE_TYPE_FILE_SINK:
        case OHOS::AudioStandard::DEVICE_TYPE_FILE_SOURCE:
        case OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_SCO:
        case OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP:
        case OHOS::AudioStandard::DEVICE_TYPE_MAX:
            AUDIO_INFO_LOG("don't supported the device type");
            break;
        default:
            AUDIO_INFO_LOG("invalid input parameter");
            break;
    }
}

AudioPin GetPinValueForPeripherals(DeviceType deviceType, DeviceRole deviceRole, uint16_t dmDeviceType)
{
    AudioPin pin = AUDIO_PIN_NONE;
    switch (deviceType) {
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADSET:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_HS_MIC;
            } else {
                pin = AUDIO_PIN_OUT_HEADSET;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADPHONES:
            pin = AUDIO_PIN_OUT_HEADPHONE;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_DP:
            pin = AUDIO_PIN_OUT_DP;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_USB_HEADSET:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_USB_HEADSET;
            } else {
                pin = AUDIO_PIN_OUT_USB_HEADSET;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_HDMI:
            pin = AUDIO_PIN_OUT_HDMI;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_ACCESSORY:
            dmDeviceType = AudioPolicyManager::GetInstance().GetDmDeviceType();
            if (dmDeviceType == DM_DEVICE_TYPE_PENCIL) {
                pin = AUDIO_PIN_IN_PENCIL;
            } else if (dmDeviceType == DM_DEVICE_TYPE_UWB) {
                pin = AUDIO_PIN_IN_UWB;
            }
            break;
        default:
            AUDIO_INFO_LOG("other case");
    }
    return pin;
}


AudioPin AudioTypeConvert::GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole)
{
    AudioPin pin = AUDIO_PIN_NONE;
    uint16_t dmDeviceType = 0;
    switch (deviceType) {
        case OHOS::AudioStandard::DEVICE_TYPE_NONE:
        case OHOS::AudioStandard::DEVICE_TYPE_INVALID:
            pin = AUDIO_PIN_NONE;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_DEFAULT:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_DAUDIO_DEFAULT;
            } else {
                pin = AUDIO_PIN_OUT_DAUDIO_DEFAULT;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_SPEAKER:
            pin = AUDIO_PIN_OUT_SPEAKER;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_MIC:
        case OHOS::AudioStandard::DEVICE_TYPE_WAKEUP:
            pin = AUDIO_PIN_IN_MIC;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADSET:
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADPHONES:
        case OHOS::AudioStandard::DEVICE_TYPE_DP:
        case OHOS::AudioStandard::DEVICE_TYPE_USB_HEADSET:
        case OHOS::AudioStandard::DEVICE_TYPE_HDMI:
        case OHOS::AudioStandard::DEVICE_TYPE_ACCESSORY:
            pin = GetPinValueForPeripherals(deviceType, deviceRole, dmDeviceType);
            break;
        default:
            OtherDeviceTypeCases(deviceType);
            break;
    }
    return pin;
}

DeviceType AudioTypeConvert::GetTypeValueFromPin(AudioPin pin)
{
    DeviceType type = DEVICE_TYPE_NONE;
    switch (pin) {
        case OHOS::AudioStandard::AUDIO_PIN_NONE:
            type = DEVICE_TYPE_NONE;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_SPEAKER:
            type = DEVICE_TYPE_SPEAKER;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HEADSET:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_LINEOUT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HDMI:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB_EXT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_DAUDIO_DEFAULT:
            type = DEVICE_TYPE_DEFAULT;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_MIC:
            type = DEVICE_TYPE_MIC;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_HS_MIC:
            type = DEVICE_TYPE_WIRED_HEADSET;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_LINEIN:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_USB_EXT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_DAUDIO_DEFAULT:
            type = DEVICE_TYPE_DEFAULT;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_PENCIL:
        case OHOS::AudioStandard::AUDIO_PIN_IN_UWB:
            type = DEVICE_TYPE_ACCESSORY;
            break;
        default:
            AUDIO_INFO_LOG("invalid input parameter");
            break;
    }
    return type;
}

void CreateStreamMap(std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> &streamMap)
{
    // Only use stream usage to choose stream type
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC)] = STREAM_MUSIC;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_CALL_ASSISTANT)] = STREAM_VOICE_CALL_ASSISTANT;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MESSAGE)] = STREAM_VOICE_MESSAGE;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_RINGTONE)] = STREAM_RING;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MOVIE)] = STREAM_MOVIE;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_GAME)] = STREAM_GAME;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_AUDIOBOOK)] = STREAM_SPEECH;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NAVIGATION)] = STREAM_NAVIGATION;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_DTMF)] = STREAM_DTMF;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ULTRASONIC)] = STREAM_ULTRASONIC;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_RINGTONE)] = STREAM_VOICE_RING;
}

std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> CreateStreamMap()
{
    std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    // Mapping relationships from content and usage to stream type in design
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN)] = STREAM_MUSIC;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_CALL_ASSISTANT)] = STREAM_VOICE_CALL_ASSISTANT;
    streamMap[std::make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[std::make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[std::make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[std::make_pair(CONTENT_TYPE_MOVIE, STREAM_USAGE_MEDIA)] = STREAM_MOVIE;
    streamMap[std::make_pair(CONTENT_TYPE_GAME, STREAM_USAGE_MEDIA)] = STREAM_GAME;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_MEDIA)] = STREAM_SPEECH;
    streamMap[std::make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[std::make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[std::make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[std::make_pair(CONTENT_TYPE_DTMF, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_DTMF;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[std::make_pair(CONTENT_TYPE_ULTRASONIC, STREAM_USAGE_SYSTEM)] = STREAM_ULTRASONIC;

    // Old mapping relationships from content and usage to stream type
    streamMap[std::make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[std::make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_UNKNOWN)] = STREAM_NOTIFICATION;
    streamMap[std::make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_MEDIA)] = STREAM_NOTIFICATION;
    streamMap[std::make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[std::make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_UNKNOWN)] = STREAM_RING;
    streamMap[std::make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_MEDIA)] = STREAM_RING;
    streamMap[std::make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;

    CreateStreamMap(streamMap);
    return streamMap;
}

const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_ = CreateStreamMap();

AudioStreamType AudioTypeConvert::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    auto pos = streamTypeMap_.find(std::make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    } else {
        AUDIO_ERR_LOG("The pair of contentType and streamUsage is not in design. Use the default stream type");
    }

    if (streamType == AudioStreamType::STREAM_MEDIA) {
        streamType = AudioStreamType::STREAM_MUSIC;
    }

    return streamType;
}
} // namespace AudioStandard
} // namespace OHOS
