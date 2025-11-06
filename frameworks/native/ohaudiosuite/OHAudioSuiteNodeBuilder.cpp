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
    return (OHAudioSuiteNodeBuilder *)builder;
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_Create(OH_AudioNodeBuilder **builder)
{
    CHECK_AND_RETURN_RET_LOG(builder != nullptr,
        AUDIOSUITE_ERROR_INVALID_PARAM, "Create audio suite builder failed, builder is nullptr");

    OHAudioSuiteNodeBuilder *nodeBuilder = new OHAudioSuiteNodeBuilder();
    CHECK_AND_RETURN_RET_LOG(nodeBuilder != nullptr,
        AUDIOSUITE_ERROR_SYSTEM, "Create audio suite builder failed, malloc error.");

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
        AUDIOSUITE_ERROR_INVALID_PARAM, "Destroy AudioNodeBuilder failed, builder is nullptr");

    return nodeBuilder->SetNodeType(type);
}

OH_AudioSuite_Result OH_AudioSuiteNodeBuilder_SetFormat(OH_AudioNodeBuilder *builder, OH_AudioFormat audioFormat)
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

namespace OHOS {
namespace AudioStandard {

using namespace OHOS::AudioStandard::AudioSuite;

static bool CheckChannelInfoValid(int32_t channelCount, OH_AudioChannelLayout channelLayout)
{
    AUDIO_DEBUG_LOG("Check channelCount:%{public}d, channelLayout:%{public}" PRIu64, channelCount, channelLayout);
    switch (channelCount) {
        case AudioChannel::MONO:
        case AudioChannel::STEREO:
            break;
        default:
            return false;
    }

    switch (channelLayout) {
        case OH_AudioChannelLayout::CH_LAYOUT_MONO:
            return channelCount == AudioChannel::MONO;
        case OH_AudioChannelLayout::CH_LAYOUT_STEREO:
        case OH_AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX:
            return channelCount == AudioChannel::STEREO;
        default:
            return false;
    }

    return false;
}

bool CheckAudioFormat(OH_AudioFormat audioFormat)
{
    return CheckChannelInfoValid(audioFormat.channelCount, audioFormat.channelLayout);
}

OHAudioSuiteNodeBuilder::~OHAudioSuiteNodeBuilder()
{
    AUDIO_INFO_LOG("OHAudioSuiteNodeBuilder destroyed, type is %{public}d", static_cast<int32_t>(nodeType_));
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetFormat(OH_AudioFormat audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(CheckAudioFormat(audioFormat), AUDIOSUITE_ERROR_UNSUPPORTED_FORMAT,
        "SetFormat failed AudioFormat invailed, nodeType = %{public}d.", static_cast<int32_t>(nodeType_));

    nodeFormat_.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(audioFormat.channelLayout);
    nodeFormat_.audioChannelInfo.numChannels = audioFormat.channelCount;
    nodeFormat_.encodingType = static_cast<AudioStreamEncodingType>(audioFormat.encodingType);
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

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::SetNodeType(OH_AudioNode_Type type)
{
    nodeType_ = static_cast<AudioNodeType>(type);
    return AUDIOSUITE_SUCCESS;
}

OH_AudioSuite_Result OHAudioSuiteNodeBuilder::Reset()
{
    nodeType_ = NODE_TYPE_EMPTY;
    nodeFormat_ = {};
    setNodeFormat_ = false;
    requestDataCallBack_ = nullptr;
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
    return requestDataCallBack_ != nullptr;
}

AudioFormat OHAudioSuiteNodeBuilder::GetNodeFormat() const
{
    return nodeFormat_;
}

OH_InputNode_RequestDataCallback OHAudioSuiteNodeBuilder::GetRequestDataCallback() const
{
    return requestDataCallBack_;
}

void *OHAudioSuiteNodeBuilder::GetCallBackUserData() const
{
    return callBackUserData_;
}

}  // namespace AudioStandard
}  // namespace OHOS
