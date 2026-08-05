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
#define LOG_TAG "OHAudioSuiteBuilder"
#endif

#include <cinttypes>
#include <string>
#include <thread>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "audio_stream_info.h"

using OHOS::AudioStandard::OHAudioSuiteNodeBuilder;

static OHOS::AudioStandard::OHAudioSuiteNodeBuilder *ConvertAudioSuitBuilder(OH_AudioNodeBuilder *builder)
{
    return reinterpret_cast<OHAudioSuiteNodeBuilder *>(builder);
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_Create(OH_AudioNodeBuilder **builder)
{
    CHECK_AND_RETURN_RET_LOG(builder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Create audio suite builder failed, builder is nullptr");

    OHAudioSuiteNodeBuilder *nodeBuilder = new OHAudioSuiteNodeBuilder();
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_MEMORY_ALLOC_FAILED, "Create audio suite builder failed, malloc error.");

    *builder = (OH_AudioNodeBuilder *)nodeBuilder;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_Destroy(OH_AudioNodeBuilder *builder)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy AudioNodeBuilder failed, builder is nullptr");

    delete nodeBuilder;
    nodeBuilder = nullptr;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_Reset(OH_AudioNodeBuilder *builder)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Reset AudioNodeBuilder failed, builder is nullptr");

    return nodeBuilder->Reset();
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_SetNodeType(
    OH_AudioNodeBuilder* builder, OH_AudioNode_Type type)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "AudioNodeBuilder SetNodeType failed, builder is nullptr");

    return nodeBuilder->SetNodeType(type);
}

int32_t OH_AudioSuiteNodeBuilderSystem_SetNodeType(
    OH_AudioNodeBuilder* builder, OH_AudioSuite_SystemNodeType type)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "AudioNodeBuilder system SetNodeType failed, builder is nullptr");

    return nodeBuilder->SetNodeType(type);
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_SetFormat(OH_AudioNodeBuilder *builder, OH_AudioFormat audioFormat)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy AudioNodeBuilder failed, builder is nullptr");

    return nodeBuilder->SetFormat(audioFormat);
}

int32_t OH_AudioSuiteNodeBuilderSystem_SetFormat(
    OH_AudioNodeBuilder* builder, OH_AudioSuite_SystemNodeFormat audioFormat)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy AudioNodeBuilder failed, builder is nullptr");

    return nodeBuilder->SetFormat(audioFormat);
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_SetRequestDataCallback(
    OH_AudioNodeBuilder *builder, OH_InputNode_RequestDataCallback callback, void *userData)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy AudioNodeBuilder failed, builder is nullptr");

    return nodeBuilder->SetRequestDataCallback(callback, userData);
}

int32_t OH_AudioSuiteNodeBuilder_SetRequestMetaDataCallback(
    OH_AudioNodeBuilder *builder, OH_InputNode_RequestMetaDataCallback callback, void *userData)
{
    OHAudioSuiteNodeBuilder *nodeBuilder = ConvertAudioSuitBuilder(builder);
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetRequestMetaDataCallback failed, builder is nullptr");

    return nodeBuilder->SetRequestMetaDataCallback(callback, userData);
}

namespace OHOS {
namespace AudioStandard {

static std::unordered_map<OH_AudioChannelLayout, AudioChannel> MAP_LAYOUT_TO_CHANNEL = {
    {OH_AudioChannelLayout::CH_LAYOUT_MONO, AudioChannel::MONO},
    {OH_AudioChannelLayout::CH_LAYOUT_STEREO, AudioChannel::STEREO},
    {OH_AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX, AudioChannel::STEREO},
    {OH_AudioChannelLayout::CH_LAYOUT_2POINT1, AudioChannel::CHANNEL_3},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT0, AudioChannel::CHANNEL_3},
    {OH_AudioChannelLayout::CH_LAYOUT_SURROUND, AudioChannel::CHANNEL_3},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT1, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_4POINT0, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_QUAD_SIDE, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_QUAD, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_2POINT0POINT2, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_4POINT1, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT0, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT0_BACK, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_2POINT1POINT2, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT0POINT2, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT1, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT1_BACK, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT0, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT1POINT2, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT1, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT1_BACK, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT1_FRONT, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT0, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT0_FRONT, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT1, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_OCTAGONAL, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT1POINT2, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK, AudioChannel::CHANNEL_8},
};

static inline const std::vector<OH_Audio_SampleFormat> AUDIO_SUPPORTED_SAMPLE_FORMAT {
    AUDIO_SAMPLE_U8,
    AUDIO_SAMPLE_S16LE,
    AUDIO_SAMPLE_S24LE,
    AUDIO_SAMPLE_S32LE,
    AUDIO_SAMPLE_F32LE,
};

static inline const std::vector<OH_Audio_SampleRate> AUDIO_SUITE_SUPPORTED_SAMPLING_RATES {
    OH_Audio_SampleRate::SAMPLE_RATE_8000,
    OH_Audio_SampleRate::SAMPLE_RATE_11025,
    OH_Audio_SampleRate::SAMPLE_RATE_12000,
    OH_Audio_SampleRate::SAMPLE_RATE_16000,
    OH_Audio_SampleRate::SAMPLE_RATE_22050,
    OH_Audio_SampleRate::SAMPLE_RATE_24000,
    OH_Audio_SampleRate::SAMPLE_RATE_32000,
    OH_Audio_SampleRate::SAMPLE_RATE_44100,
    OH_Audio_SampleRate::SAMPLE_RATE_48000,
    OH_Audio_SampleRate::SAMPLE_RATE_64000,
    OH_Audio_SampleRate::SAMPLE_RATE_88200,
    OH_Audio_SampleRate::SAMPLE_RATE_96000,
    OH_Audio_SampleRate::SAMPLE_RATE_176400,
    OH_Audio_SampleRate::SAMPLE_RATE_192000,
};

static inline const std::vector<OH_Audio_EncodingType> AUDIO_SUPPORTED_ENCODEING_TYPE {
    AUDIO_ENCODING_TYPE_RAW,
};

using namespace OHOS::AudioStandard::AudioSuite;

static bool CheckChannelInfoValid(int32_t channelCount, OH_AudioChannelLayout channelLayout)
{
    AUDIO_DEBUG_LOG("Check channelCount:%{public}d, channelLayout:%{public}" PRIu64, channelCount, channelLayout);
    CHECK_AND_RETURN_RET_LOG(MAP_LAYOUT_TO_CHANNEL.find(channelLayout) != MAP_LAYOUT_TO_CHANNEL.end(),
        false,
        "Channel layout parameters do not exist");
    CHECK_AND_RETURN_RET_LOG(
        MAP_LAYOUT_TO_CHANNEL[channelLayout] == channelCount, false, "Channel parameter settings incorrect");
    return true;
}

bool IsHoaFormat(OH_AudioChannelLayout channelLayout)
{
    switch (channelLayout) {
        case CH_LAYOUT_AMB_ORDER1_ACN_N3D:
        case CH_LAYOUT_AMB_ORDER1_ACN_SN3D:
        case CH_LAYOUT_AMB_ORDER1_FUMA:
        case CH_LAYOUT_AMB_ORDER2_ACN_N3D:
        case CH_LAYOUT_AMB_ORDER2_ACN_SN3D:
        case CH_LAYOUT_AMB_ORDER2_FUMA:
        case CH_LAYOUT_AMB_ORDER3_ACN_N3D:
        case CH_LAYOUT_AMB_ORDER3_ACN_SN3D:
        case CH_LAYOUT_AMB_ORDER3_FUMA:
            return true;
        default:
            return false;
    }
}

bool CheckHoaInputFormat(OH_AudioFormat audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(audioFormat.samplingRate == OH_Audio_SampleRate::SAMPLE_RATE_16000 ||
        audioFormat.samplingRate == OH_Audio_SampleRate::SAMPLE_RATE_48000,
        false, "HOA input format requires 16k or 48k samplingRate, got %{public}d", audioFormat.samplingRate);
    CHECK_AND_RETURN_RET_LOG(!NotContain(AUDIO_SUPPORTED_SAMPLE_FORMAT, audioFormat.sampleFormat),
        false, "input format sampleFormat%{public}d not supported", audioFormat.sampleFormat);
    CHECK_AND_RETURN_RET_LOG(!NotContain(AUDIO_SUPPORTED_ENCODEING_TYPE, audioFormat.encodingType),
        false, "input format encodingType%{public}d not supported", audioFormat.encodingType);
    switch (audioFormat.channelLayout) {
        case CH_LAYOUT_AMB_ORDER1_ACN_N3D:
        case CH_LAYOUT_AMB_ORDER1_ACN_SN3D:
        case CH_LAYOUT_AMB_ORDER1_FUMA:
            CHECK_AND_RETURN_RET_LOG(audioFormat.channelCount == AudioChannel::CHANNEL_4,
                false, "HOA ORDER1 requires 4 channels, got %{public}d", audioFormat.channelCount);
            break;
        case CH_LAYOUT_AMB_ORDER2_ACN_N3D:
        case CH_LAYOUT_AMB_ORDER2_ACN_SN3D:
        case CH_LAYOUT_AMB_ORDER2_FUMA:
            CHECK_AND_RETURN_RET_LOG(audioFormat.channelCount == AudioChannel::CHANNEL_9,
                false, "HOA ORDER2 requires 9 channels, got %{public}d", audioFormat.channelCount);
            break;
        case CH_LAYOUT_AMB_ORDER3_ACN_N3D:
        case CH_LAYOUT_AMB_ORDER3_ACN_SN3D:
        case CH_LAYOUT_AMB_ORDER3_FUMA:
            CHECK_AND_RETURN_RET_LOG(audioFormat.channelCount == AudioChannel::CHANNEL_16,
                false, "HOA ORDER3 requires 16 channels, got %{public}d", audioFormat.channelCount);
            break;
        default:
            return false;
    }
    return true;
}

static bool CheckAudioFormat(
    OH_Audio_SampleRate samplingRate, OH_Audio_SampleFormat sampleFormat, OH_Audio_EncodingType encodingType)
{
    CHECK_AND_RETURN_RET_LOG(!NotContain(AUDIO_SUITE_SUPPORTED_SAMPLING_RATES, samplingRate),
        false, "input format samplingRate:%{public}d, not supported", samplingRate);
    CHECK_AND_RETURN_RET_LOG(!NotContain(AUDIO_SUPPORTED_SAMPLE_FORMAT, sampleFormat),
        false, "input format sampleFormat%{public}d not supported", sampleFormat);
    CHECK_AND_RETURN_RET_LOG(!NotContain(AUDIO_SUPPORTED_ENCODEING_TYPE, encodingType),
        false, "input format encodingType%{public}d not supported", encodingType);
    return true;
}

bool CheckAudioFormat(OH_AudioFormat audioFormat)
{
    return CheckChannelInfoValid(audioFormat.channelCount, audioFormat.channelLayout) &&
        CheckAudioFormat(audioFormat.samplingRate, audioFormat.sampleFormat, audioFormat.encodingType);
}

bool CheckAudioFormat(OH_AudioSuite_SystemNodeFormat audioFormat)
{
    AUDIO_INFO_LOG("Check samplingRate:%{public}d, sampleFormat:%{public}d, encoding:%{public}d, "
        "channelCount:%{public}d, channelLayout:%{public}" PRIu64, audioFormat.samplingRate,
        audioFormat.sampleFormat, audioFormat.encoding, audioFormat.channelCount, audioFormat.channelLayout);
    CHECK_AND_RETURN_RET_LOG(
        MAP_LAYOUT_TO_CHANNEL.find(audioFormat.channelLayout) != MAP_LAYOUT_TO_CHANNEL.end(), false,
        "SetFormat failed AudioFormat channelLayout invalid.");
    CHECK_AND_RETURN_RET_LOG(MAP_LAYOUT_TO_CHANNEL[audioFormat.channelLayout] == audioFormat.channelCount, false,
        "SetFormat failed AudioFormat channelCount invalid.");
    CHECK_AND_RETURN_RET_LOG(audioFormat.encoding == 0 ||
        audioFormat.encoding == static_cast<int32_t>(AudioStreamEncodingType::ENCODING_METADATA), false,
        "SetFormat failed AudioFormat encoding invalid.");
    CHECK_AND_RETURN_RET_LOG(!NotContain(AUDIO_SUITE_SUPPORTED_SAMPLING_RATES, audioFormat.samplingRate),
        false, "input format samplingRate:%{public}d, not supported", audioFormat.samplingRate);
    CHECK_AND_RETURN_RET_LOG(!NotContain(AUDIO_SUPPORTED_SAMPLE_FORMAT, audioFormat.sampleFormat),
        false, "input format sampleFormat%{public}d not supported", audioFormat.sampleFormat);
    return true;
}

OHAudioSuiteNodeBuilder::~OHAudioSuiteNodeBuilder()
{
    AUDIO_INFO_LOG("OHAudioSuiteNodeBuilder destroyed, type is %{public}d", static_cast<int32_t>(nodeType_));
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetFormat(OH_AudioFormat audioFormat)
{
    if (nodeType_ == NODE_TYPE_INPUT && IsHoaFormat(audioFormat.channelLayout)) {
        CHECK_AND_RETURN_RET_LOG(CheckHoaInputFormat(audioFormat), AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT,
            "Set Hoa Format failed AudioFormat invalid, nodeType = %{public}d.", static_cast<int32_t>(nodeType_));
    } else {
        CHECK_AND_RETURN_RET_LOG(CheckAudioFormat(audioFormat), AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT,
            "SetFormat failed AudioFormat invalid, nodeType = %{public}d.", static_cast<int32_t>(nodeType_));
    }

    nodeFormat_.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(audioFormat.channelLayout);
    nodeFormat_.audioChannelInfo.numChannels = audioFormat.channelCount;
    nodeFormat_.encodingType = static_cast<AudioStreamEncodingType>(audioFormat.encodingType);
    nodeFormat_.format = static_cast<AudioSampleFormat>(audioFormat.sampleFormat);
    nodeFormat_.rate =  static_cast<AudioSamplingRate>(audioFormat.samplingRate);
    setNodeFormat_ = true;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetFormat(OH_AudioSuite_SystemNodeFormat audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(CheckAudioFormat(audioFormat), AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT,
        "SetFormat failed AudioFormat invalid, nodeType = %{public}d.", static_cast<int32_t>(nodeType_));

    nodeFormat_.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(audioFormat.channelLayout);
    nodeFormat_.audioChannelInfo.numChannels = audioFormat.channelCount;
    nodeFormat_.encodingType = static_cast<AudioStreamEncodingType>(audioFormat.encoding);
    nodeFormat_.format = static_cast<AudioSampleFormat>(audioFormat.sampleFormat);
    nodeFormat_.rate =  static_cast<AudioSamplingRate>(audioFormat.samplingRate);
    setNodeFormat_ = true;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetRequestDataCallback(
    OH_InputNode_RequestDataCallback callback, void *userData)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetRequestDataCallback failed, callback is nullptr");

    requestDataCallBack_ = callback;
    callBackUserData_ = userData;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetRequestMetaDataCallback(
    OH_InputNode_RequestMetaDataCallback callback, void *userData)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "SetRequestMetaDataCallback failed, callback is nullptr");

    requestMetaDataCallBack_ = callback;
    callBackUserData_ = userData;
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetNodeType(OH_AudioNode_Type type)
{
    CHECK_AND_RETURN_RET_LOG(
        NODETYPE_TOSTRING_MAP.find(static_cast<AudioNodeType>(type)) != NODETYPE_TOSTRING_MAP.end(),
        AUDIOSUITE_ERROR_INVALID_PARAM, "AudioNodeBuilder SetNodeType failed, type is invalid");
    nodeType_ = static_cast<AudioNodeType>(type);
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetNodeType(OH_AudioSuite_SystemNodeType type)
{
    CHECK_AND_RETURN_RET_LOG(
        NODETYPE_TOSTRING_MAP.find(static_cast<AudioNodeType>(type)) != NODETYPE_TOSTRING_MAP.end(),
        AUDIOSUITE_ERROR_INVALID_PARAM, "AudioNodeBuilder SetNodeType failed, type is invalid");
    nodeType_ = static_cast<AudioNodeType>(type);
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::Reset()
{
    nodeType_ = NODE_TYPE_EMPTY;
    nodeFormat_ = {};
    setNodeFormat_ = false;
    requestDataCallBack_ = nullptr;
    requestMetaDataCallBack_ = nullptr;
    callBackUserData_ = nullptr;
    return AUDIOSUITE_SUCCESS;
}

AudioNodeType OHAudioSuiteNodeBuilder::GetNodeType() const
{
    return nodeType_;
}

bool OHAudioSuiteNodeBuilder::IsSetFormat() const
{
    return setNodeFormat_;
}

bool OHAudioSuiteNodeBuilder::IsSetRequestDataCallback() const
{
    return requestDataCallBack_ != nullptr || requestMetaDataCallBack_ != nullptr;
}

bool OHAudioSuiteNodeBuilder::IsSetRequestMetaDataCallback() const
{
    return requestMetaDataCallBack_ != nullptr;
}

AudioFormat OHAudioSuiteNodeBuilder::GetNodeFormat() const
{
    return nodeFormat_;
}

OH_InputNode_RequestDataCallback OHAudioSuiteNodeBuilder::GetRequestDataCallback() const
{
    return requestDataCallBack_;
}

OH_InputNode_RequestMetaDataCallback OHAudioSuiteNodeBuilder::GetRequestMetaDataCallback() const
{
    return requestMetaDataCallBack_;
}

void *OHAudioSuiteNodeBuilder::GetCallBackUserData() const
{
    return callBackUserData_;
}

}  // namespace AudioStandard
}  // namespace OHOS
