/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "IAudioStream"
#endif

#include "i_audio_stream.h"
#include <map>

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "fast_audio_stream.h"

#include "capturer_in_client.h"
#include "renderer_in_client.h"

namespace OHOS {
namespace AudioStandard {
const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_ = IAudioStream::CreateStreamMap();
std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> IAudioStream::CreateStreamMap()
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

    IAudioStream::CreateStreamMap(streamMap);
    return streamMap;
}

void IAudioStream::CreateStreamMap(std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> &streamMap)
{
    // Only use stream usage to choose stream type
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC)] = STREAM_MUSIC;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
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
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_CALL_ASSISTANT)] = STREAM_VOICE_CALL_ASSISTANT;
}

AudioStreamType IAudioStream::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = STREAM_MUSIC;
    auto pos = streamTypeMap_.find(std::make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    }

    if (streamType == STREAM_MEDIA) {
        streamType = STREAM_MUSIC;
    }

    return streamType;
}

const std::string IAudioStream::GetEffectSceneName(const StreamUsage &streamUsage)
{
    // return AudioPolicyManager::GetInstance().GetEffectSceneName(streamUsage);
    SupportedEffectConfig supportedEffectConfig;
    AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    std::string streamUsageString = "";
    for (const auto& pair : STREAM_USAGE_MAP) {
        if (pair.second == streamUsage) {
            streamUsageString = pair.first;
            break;
        }
    }
    if (supportedEffectConfig.postProcessNew.stream.empty()) {
        AUDIO_WARNING_LOG("empty scene type set!");
        return "";
    }
    if (streamUsageString == "") {
        AUDIO_WARNING_LOG("Find streamUsage string failed, not in the parser's string-enum map.");
        return supportedEffectConfig.postProcessNew.stream.back().scene;
    }
    for (const SceneMappingItem &item: supportedEffectConfig.postProcessSceneMap) {
        if (item.name == streamUsageString) {
            return item.sceneType;
        }
    }
    return supportedEffectConfig.postProcessNew.stream.back().scene;
}

int32_t IAudioStream::GetByteSizePerFrame(const AudioStreamParams &params, size_t &result)
{
    result = 0;
    size_t bitWidthSize = 0;
    switch (params.format) {
        case SAMPLE_U8:
            bitWidthSize = 1; // size is 1
            break;
        case SAMPLE_S16LE:
            bitWidthSize = 2; // size is 2
            break;
        case SAMPLE_S24LE:
            bitWidthSize = 3; // size is 3
            break;
        case SAMPLE_S32LE:
            bitWidthSize = 4; // size is 4
            break;
        case SAMPLE_F32LE:
            bitWidthSize = 4; // size is 4
            break;
        default:
            return ERR_INVALID_PARAM;
            break;
    }

    if (params.channels < 1 || params.channels > 16) { // 1 is min channel size, 16 is max channel size
        return ERR_INVALID_PARAM;
    }
    result = bitWidthSize * static_cast<size_t>(params.channels);
    return SUCCESS;
}

bool IAudioStream::IsStreamSupported(int32_t streamFlags, const AudioStreamParams &params)
{
    // 0 for normal stream
    if (streamFlags == 0) {
        return true;
    }
    // 1 for fast stream
    if (streamFlags == STREAM_FLAG_FAST) {
        // check audio sample rate
        AudioSamplingRate samplingRate = static_cast<AudioSamplingRate>(params.samplingRate);
        auto rateItem = std::find(AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES.begin(),
            AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES.end(), samplingRate);
        if (rateItem == AUDIO_FAST_STREAM_SUPPORTED_SAMPLING_RATES.end()) {
            AUDIO_WARNING_LOG("Sampling rate %{public}d does not meet the requirements", samplingRate);
            return false;
        }

        // check audio channel
        AudioChannel channels = static_cast<AudioChannel>(params.channels);
        auto channelItem = std::find(AUDIO_FAST_STREAM_SUPPORTED_CHANNELS.begin(),
            AUDIO_FAST_STREAM_SUPPORTED_CHANNELS.end(), channels);
        if (channelItem == AUDIO_FAST_STREAM_SUPPORTED_CHANNELS.end()) {
            AUDIO_WARNING_LOG("Audio channel %{public}d does not meet the requirements", channels);
            return false;
        }

        // check audio sample format
        AudioSampleFormat format = static_cast<AudioSampleFormat>(params.format);
        auto formatItem = std::find(AUDIO_FAST_STREAM_SUPPORTED_FORMATS.begin(),
            AUDIO_FAST_STREAM_SUPPORTED_FORMATS.end(), format);
        if (formatItem == AUDIO_FAST_STREAM_SUPPORTED_FORMATS.end()) {
            AUDIO_WARNING_LOG("Audio sample format %{public}d does not meet the requirements", format);
            return false;
        }
    }
    return true;
}

std::shared_ptr<IAudioStream> IAudioStream::GetPlaybackStream(StreamClass streamClass, AudioStreamParams params,
    AudioStreamType eStreamType, int32_t appUid)
{
    Trace trace("IAudioStream::GetPlaybackStream");
    if (streamClass == FAST_STREAM || streamClass == VOIP_STREAM) {
        (void)params;
        AUDIO_INFO_LOG("Create fast playback stream");
        return std::make_shared<FastAudioStream>(eStreamType, AUDIO_MODE_PLAYBACK, appUid);
    }

    if (streamClass == PA_STREAM) {
        AUDIO_INFO_LOG("Create ipc playback stream");
        return RendererInClient::GetInstance(eStreamType, appUid);
    }
    return nullptr;
}

std::shared_ptr<IAudioStream> IAudioStream::GetRecordStream(StreamClass streamClass, AudioStreamParams params,
    AudioStreamType eStreamType, int32_t appUid)
{
    Trace trace("IAudioStream::GetRecordStream");
    if (streamClass == FAST_STREAM || streamClass == VOIP_STREAM) {
        (void)params;
        AUDIO_INFO_LOG("Create fast record stream");
        return std::make_shared<FastAudioStream>(eStreamType, AUDIO_MODE_RECORD, appUid);
    }
    if (streamClass == PA_STREAM) {
        AUDIO_INFO_LOG("Create ipc record stream");
        return CapturerInClient::GetInstance(eStreamType, appUid);
    }
    return nullptr;
}

bool IAudioStream::IsFormatValid(uint8_t format)
{
    bool isValidFormat = (find(AUDIO_SUPPORTED_FORMATS.begin(), AUDIO_SUPPORTED_FORMATS.end(), format)
                          != AUDIO_SUPPORTED_FORMATS.end());
    AUDIO_DEBUG_LOG("AudioStream: IsFormatValid: %{public}s", isValidFormat ? "true" : "false");
    return isValidFormat;
}

bool IAudioStream::IsRendererChannelValid(uint8_t channel)
{
    bool isValidChannel = (find(RENDERER_SUPPORTED_CHANNELS.begin(), RENDERER_SUPPORTED_CHANNELS.end(), channel)
                           != RENDERER_SUPPORTED_CHANNELS.end());
    AUDIO_DEBUG_LOG("AudioStream: IsChannelValid: %{public}s", isValidChannel ? "true" : "false");
    return isValidChannel;
}

bool IAudioStream::IsCapturerChannelValid(uint8_t channel)
{
    bool isValidChannel = (find(CAPTURER_SUPPORTED_CHANNELS.begin(), CAPTURER_SUPPORTED_CHANNELS.end(), channel)
                           != CAPTURER_SUPPORTED_CHANNELS.end());
    AUDIO_DEBUG_LOG("AudioStream: IsChannelValid: %{public}s", isValidChannel ? "true" : "false");
    return isValidChannel;
}

bool IAudioStream::IsEncodingTypeValid(uint8_t encodingType)
{
    bool isValidEncodingType
            = (find(AUDIO_SUPPORTED_ENCODING_TYPES.begin(), AUDIO_SUPPORTED_ENCODING_TYPES.end(), encodingType)
               != AUDIO_SUPPORTED_ENCODING_TYPES.end());
    AUDIO_DEBUG_LOG("AudioStream: IsEncodingTypeValid: %{public}s", isValidEncodingType ? "true" : "false");
    return isValidEncodingType;
}

bool IAudioStream::IsSamplingRateValid(uint32_t samplingRate)
{
    bool isValidSamplingRate
            = (find(AUDIO_SUPPORTED_SAMPLING_RATES.begin(), AUDIO_SUPPORTED_SAMPLING_RATES.end(), samplingRate)
               != AUDIO_SUPPORTED_SAMPLING_RATES.end());
    AUDIO_DEBUG_LOG("AudioStream: IsSamplingRateValid: %{public}s", isValidSamplingRate ? "true" : "false");
    return isValidSamplingRate;
}

bool IAudioStream::IsRendererChannelLayoutValid(uint64_t channelLayout)
{
    bool isValidRendererChannelLayout = (find(RENDERER_SUPPORTED_CHANNELLAYOUTS.begin(),
        RENDERER_SUPPORTED_CHANNELLAYOUTS.end(), channelLayout) != RENDERER_SUPPORTED_CHANNELLAYOUTS.end());
    AUDIO_DEBUG_LOG("AudioStream: isValidRendererChannelLayout: %{public}s",
        isValidRendererChannelLayout ? "true" : "false");
    return isValidRendererChannelLayout;
}

bool IAudioStream::IsCapturerChannelLayoutValid(uint64_t channelLayout)
{
    bool isValidCapturerChannelLayout = IsRendererChannelLayoutValid(channelLayout);
    AUDIO_DEBUG_LOG("AudioStream: isValidCapturerChannelLayout: %{public}s",
        isValidCapturerChannelLayout ? "true" : "false");
    return isValidCapturerChannelLayout;
}

bool IAudioStream::IsPlaybackChannelRelatedInfoValid(uint8_t channels, uint64_t channelLayout)
{
    if (!IsRendererChannelValid(channels)) {
        AUDIO_ERR_LOG("AudioStream: Invalid sink channel %{public}d", channels);
        return false;
    }
    if (!IsRendererChannelLayoutValid(channelLayout)) {
        AUDIO_ERR_LOG("AudioStream: Invalid sink channel layout");
        return false;
    }
    return true;
}

bool IAudioStream::IsRecordChannelRelatedInfoValid(uint8_t channels, uint64_t channelLayout)
{
    if (!IsCapturerChannelValid(channels)) {
        return false;
    }
    if (!IsCapturerChannelLayoutValid(channelLayout)) {
        return false;
    }
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
