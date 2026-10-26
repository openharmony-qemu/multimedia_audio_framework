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
#define LOG_TAG "IAudioStream"
#endif

#include "i_audio_stream.h"
#include <unistd.h>
#include <map>

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "stream_dfx_manager.h"
#include "audio_policy_manager.h"
#include "capturer_in_client.h"
#include "renderer_in_client.h"

namespace OHOS {
namespace AudioStandard {
const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_ = IAudioStream::CreateStreamMap();

const std::map<AudioChannel, AudioChannelLayout> AUDIO_DEFAULT_CHANNEL_LAYOUT_MAP = {
    {MONO, CH_LAYOUT_MONO},
    {STEREO, CH_LAYOUT_STEREO},
    {CHANNEL_3, CH_LAYOUT_SURROUND},
    {CHANNEL_4, CH_LAYOUT_2POINT0POINT2},
    {CHANNEL_5, CH_LAYOUT_5POINT0_BACK},
    {CHANNEL_6, CH_LAYOUT_5POINT1},
    {CHANNEL_7, CH_LAYOUT_6POINT1_BACK},
    {CHANNEL_8, CH_LAYOUT_5POINT1POINT2},
    {CHANNEL_9, CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {CHANNEL_10, CH_LAYOUT_7POINT1POINT2},
    {CHANNEL_12, CH_LAYOUT_7POINT1POINT4},
    {CHANNEL_14, CH_LAYOUT_9POINT1POINT4},
    {CHANNEL_16, CH_LAYOUT_9POINT1POINT6}
};

std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> IAudioStream::CreateStreamMap()
{
    std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    // Mapping relationships from content and usage to stream type in design
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN)] = STREAM_MUSIC;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_INTERPHONE)] = STREAM_VOICE_COMMUNICATION;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[std::make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[std::make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_TRANSFER_RINGTONE)] = STREAM_VOICE_RING;
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
    streamMap[std::make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_ANNOUNCEMENT)] = STREAM_ANNOUNCEMENT;
    streamMap[std::make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_EMERGENCY)] = STREAM_EMERGENCY;

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
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_TRANSFER_RINGTONE)] = STREAM_VOICE_RING;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN,
        STREAM_USAGE_VOICE_ASSISTANT_SYSTEM)] = STREAM_VOICE_ASSISTANT_SYSTEM;
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
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_INTERPHONE)] = STREAM_VOICE_COMMUNICATION;
#ifdef MULTI_ALARM_LEVEL
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ANNOUNCEMENT)] = STREAM_ANNOUNCEMENT;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_EMERGENCY)] = STREAM_EMERGENCY;
#else
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ANNOUNCEMENT)] = STREAM_ALARM;
    streamMap[std::make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_EMERGENCY)] = STREAM_ALARM;
#endif
}

int32_t IAudioStream::CheckRendererAudioStreamInfo(const AudioStreamParams info)
{
    CHECK_AND_RETURN_RET_LOG(Util::IsFormatValid(info.format) && Util::IsEncodingTypeValid(info.encoding) &&
        ((info.customSampleRate == 0 && Util::IsSamplingRateValid(info.samplingRate)) ||
        (info.customSampleRate != 0 && Util::IsCustomSampleRateValid(info.customSampleRate))),
        ERR_NOT_SUPPORTED, "Unsupported audio renderer parameter");
    CHECK_AND_RETURN_RET(Util::IsPlaybackChannelRelatedInfoValid(info.encoding, info.channels, info.channelLayout),
        ERR_NOT_SUPPORTED);
    return SUCCESS;
}

int32_t IAudioStream::CheckCapturerAudioStreamInfo(const AudioStreamParams info)
{
    if (!Util::IsFormatValid(info.format) || !Util::IsEncodingTypeValid(info.encoding) ||
        !Util::IsSamplingRateValid(info.samplingRate)) {
        AUDIO_ERR_LOG("Unsupported audio capturer parameter");
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_PARAM, "Unsupported audio capturer parameter", true);
        return ERR_NOT_SUPPORTED;
    }
    CHECK_AND_CALL_FUNC_RETURN_RET(Util::IsRecordChannelRelatedInfoValid(info.channels, info.channelLayout),
        ERR_NOT_SUPPORTED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_PARAM, "Invalid record channel info", true));
    return SUCCESS;
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

int32_t IAudioStream::GetByteSizePerFrameWithEc(const AudioStreamParams &params, size_t &result)
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
    result = bitWidthSize * static_cast<size_t>(params.channels + params.ecChannels);
    return SUCCESS;
}

int32_t IAudioStream::GetByteSizePerFrameWithMicInEc(const AudioStreamParams &params, size_t &result)
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

    size_t totalChannels = static_cast<size_t>(params.channels + params.micInChannels + params.ecChannels);
    if (totalChannels < 1 || totalChannels > 16) { // 1 is min channel size, 16 is max channel size
        return ERR_INVALID_PARAM;
    }
    result = bitWidthSize * totalChannels;
    return SUCCESS;
}

int32_t IAudioStream::GetByteSizePerFrameWithMicIn(const AudioStreamParams &params, size_t &result)
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

    size_t totalChannels = static_cast<size_t>(params.channels + params.micInChannels);
    if (totalChannels < 1 || totalChannels > 16) { // 1 is min channel size, 16 is max channel size
        return ERR_INVALID_PARAM;
    }
    result = bitWidthSize * totalChannels;
    return SUCCESS;
}

std::shared_ptr<IAudioStream> IAudioStream::GetPlaybackStream(StreamClass streamClass, AudioStreamParams params,
    AudioStreamType eStreamType, int32_t appUid)
{
    Trace trace("IAudioStream::GetPlaybackStream");
    if (streamClass == FAST_STREAM || streamClass == VOIP_STREAM || streamClass == PA_STREAM) {
        HILOG_COMM_INFO("[GetPlaybackStream]Create renderer in client for playback stream");
        return RendererInClient::GetInstance(eStreamType, appUid);
    }
    return nullptr;
}

std::shared_ptr<IAudioStream> IAudioStream::GetRecordStream(StreamClass streamClass, AudioStreamParams params,
    AudioStreamType eStreamType, int32_t appUid)
{
    Trace trace("IAudioStream::GetRecordStream");
    if (streamClass == FAST_STREAM || streamClass == VOIP_STREAM || streamClass == PA_STREAM) {
        HILOG_COMM_INFO("[GetRecordStream]Create capturer in client for record stream");
        return CapturerInClient::GetInstance(eStreamType, appUid);
    }
    return nullptr;
}

AudioChannelLayout IAudioStream::ConvertChannelsToDefaultChannelLayout(AudioChannel channels,
    AudioChannelLayout channelLayout)
{
    CHECK_AND_RETURN_RET_LOG(channelLayout == CH_LAYOUT_UNKNOWN, channelLayout,
        "channelLayout is not CH_LAYOUT_UNKNOWN");

    auto iter = AUDIO_DEFAULT_CHANNEL_LAYOUT_MAP.find(channels);
    if (iter != AUDIO_DEFAULT_CHANNEL_LAYOUT_MAP.end()) {
        return iter->second;
    }
    return channelLayout;
}
} // namespace AudioStandard
} // namespace OHOS
