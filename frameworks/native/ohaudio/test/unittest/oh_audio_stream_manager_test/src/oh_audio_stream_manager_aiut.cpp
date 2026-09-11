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

#include "aiut_log.h"
#include "audio_errors.h"
#include "native_audio_stream_manager.h"
#include "gtest/gtest.h"
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class OHAudioStreamManagerAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

/**
 * @tc.name  : Test OH_AudioManager_GetAudioStreamManager with null param
 * @tc.number: OH_AudioManager_GetAudioStreamManager_NullParam
 * @tc.desc  : Test OH_AudioManager_GetAudioStreamManager with null param returns ERROR_SYSTEM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioManager_GetAudioStreamManager_NullParam, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto result = OH_AudioManager_GetAudioStreamManager(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "expected ERROR_SYSTEM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_GetDirectPlaybackSupport with null manager
 * @tc.number: OH_AudioStreamManager_GetDirectPlaybackSupport_NullManager
 * @tc.desc  : Test GetDirectPlaybackSupport with null manager returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_GetDirectPlaybackSupport_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    OH_AudioStream_DirectPlaybackMode directPlaybackMode;
    auto result = OH_AudioStreamManager_GetDirectPlaybackSupport(nullptr, &streamInfo, usage, &directPlaybackMode);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_GetDirectPlaybackSupport with null streamInfo
 * @tc.number: OH_AudioStreamManager_GetDirectPlaybackSupport_NullStreamInfo
 * @tc.desc  : Test GetDirectPlaybackSupport with null streamInfo returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_GetDirectPlaybackSupport_NullStreamInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto result = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    OH_AudioStream_DirectPlaybackMode directPlaybackMode;
    result = OH_AudioStreamManager_GetDirectPlaybackSupport(audioStreamManager, nullptr, usage, &directPlaybackMode);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_GetDirectPlaybackSupport with invalid usage
 * @tc.number: OH_AudioStreamManager_GetDirectPlaybackSupport_InvalidUsage
 * @tc.desc  : Test GetDirectPlaybackSupport with UNKNOWN usage returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_GetDirectPlaybackSupport_InvalidUsage, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto result = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_UNKNOWN;
    OH_AudioStream_DirectPlaybackMode directPlaybackMode;
    result = OH_AudioStreamManager_GetDirectPlaybackSupport(audioStreamManager, &streamInfo, usage,
        &directPlaybackMode);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_GetDirectPlaybackSupport with null directPlaybackMode
 * @tc.number: OH_AudioStreamManager_GetDirectPlaybackSupport_NullMode
 * @tc.desc  : Test GetDirectPlaybackSupport with null mode returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_GetDirectPlaybackSupport_NullMode, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto result = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    result = OH_AudioStreamManager_GetDirectPlaybackSupport(audioStreamManager, &streamInfo, usage, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsAcousticEchoCancelerSupported with null manager
 * @tc.number: OH_AudioStreamManager_IsAcousticEchoCancelerSupported_NullManager
 * @tc.desc  : Test IsAcousticEchoCancelerSupported with null manager returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsAcousticEchoCancelerSupported_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_MIC;
    bool supported = false;
    auto result = OH_AudioStreamManager_IsAcousticEchoCancelerSupported(nullptr, sourceType, &supported);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsAcousticEchoCancelerSupported with invalid source
 * @tc.number: OH_AudioStreamManager_IsAcousticEchoCancelerSupported_InvalidSource
 * @tc.desc  : Test IsAcousticEchoCancelerSupported with invalid source returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsAcousticEchoCancelerSupported_InvalidSource, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto result = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_SourceType sourceType = static_cast<OH_AudioStream_SourceType>(999);
    bool supported = false;
    result = OH_AudioStreamManager_IsAcousticEchoCancelerSupported(audioStreamManager, sourceType, &supported);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastPlaybackSupported with valid params
 * @tc.number: OH_AudioStreamManager_IsFastPlaybackSupported_Valid
 * @tc.desc  : Test IsFastPlaybackSupported with valid params returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsFastPlaybackSupported_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsFastPlaybackSupported(audioStreamManager, &streamInfo, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastPlaybackSupported with null streamInfo
 * @tc.number: OH_AudioStreamManager_IsFastPlaybackSupported_NullStreamInfo
 * @tc.desc  : Test IsFastPlaybackSupported with null streamInfo returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsFastPlaybackSupported_NullStreamInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsFastPlaybackSupported(audioStreamManager, nullptr, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastRecordingSupported with valid params
 * @tc.number: OH_AudioStreamManager_IsFastRecordingSupported_Valid
 * @tc.desc  : Test IsFastRecordingSupported with valid params returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsFastRecordingSupported_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_MIC;
    auto result = OH_AudioStreamManager_IsFastRecordingSupported(audioStreamManager, &streamInfo, sourceType);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastRecordingSupported with null streamInfo
 * @tc.number: OH_AudioStreamManager_IsFastRecordingSupported_NullStreamInfo
 * @tc.desc  : Test IsFastRecordingSupported with null streamInfo returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsFastRecordingSupported_NullStreamInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_MIC;
    auto result = OH_AudioStreamManager_IsFastRecordingSupported(audioStreamManager, nullptr, sourceType);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastRecordingSupported with invalid source
 * @tc.number: OH_AudioStreamManager_IsFastRecordingSupported_InvalidSource
 * @tc.desc  : Test IsFastRecordingSupported with invalid source returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsFastRecordingSupported_InvalidSource, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_SourceType sourceType = static_cast<OH_AudioStream_SourceType>(999);
    auto result = OH_AudioStreamManager_IsFastRecordingSupported(audioStreamManager, &streamInfo, sourceType);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsIntelligentNoiseReduction with null manager
 * @tc.number: OH_AudioStreamManager_IsIntelligentNoiseReduction_NullManager
 * @tc.desc  : Test IsIntelligentNoiseReductionEnabledForCurrentDevice with null manager returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsIntelligentNoiseReduction_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_MIC;
    auto result = OH_AudioStreamManager_IsIntelligentNoiseReductionEnabledForCurrentDevice(nullptr, sourceType);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsIntelligentNoiseReduction with invalid source
 * @tc.number: OH_AudioStreamManager_IsIntelligentNoiseReduction_InvalidSource
 * @tc.desc  : Test IsIntelligentNoiseReductionEnabledForCurrentDevice with invalid source returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsIntelligentNoiseReduction_InvalidSource, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_SourceType sourceType = static_cast<OH_AudioStream_SourceType>(999);
    auto result = OH_AudioStreamManager_IsIntelligentNoiseReductionEnabledForCurrentDevice(audioStreamManager,
        sourceType);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsMultichannelPlaybackSupported with valid params
 * @tc.number: OH_AudioStreamManager_IsMultichannelPlaybackSupported_Valid
 * @tc.desc  : Test IsMultichannelPlaybackSupported with valid params returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsMultichannelPlaybackSupported_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_2POINT1;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsMultichannelPlaybackSupported(audioStreamManager, &streamInfo, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsMultichannelPlaybackSupported with null streamInfo
 * @tc.number: OH_AudioStreamManager_IsMultichannelPlaybackSupported_NullStreamInfo
 * @tc.desc  : Test IsMultichannelPlaybackSupported with null streamInfo returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsMultichannelPlaybackSupported_NullStreamInfo,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsMultichannelPlaybackSupported(audioStreamManager, nullptr, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsDirectPlaybackSupported with valid params
 * @tc.number: OH_AudioStreamManager_IsDirectPlaybackSupported_Valid
 * @tc.desc  : Test IsDirectPlaybackSupported with valid params returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsDirectPlaybackSupported_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsDirectPlaybackSupported(audioStreamManager, &streamInfo, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsDirectPlaybackSupported with null streamInfo
 * @tc.number: OH_AudioStreamManager_IsDirectPlaybackSupported_NullStreamInfo
 * @tc.desc  : Test IsDirectPlaybackSupported with null streamInfo returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsDirectPlaybackSupported_NullStreamInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsDirectPlaybackSupported(audioStreamManager, nullptr, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsOffloadPlaybackSupported with valid params
 * @tc.number: OH_AudioStreamManager_IsOffloadPlaybackSupported_Valid
 * @tc.desc  : Test IsOffloadPlaybackSupported with valid params returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsOffloadPlaybackSupported_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsOffloadPlaybackSupported(audioStreamManager, &streamInfo, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsOffloadPlaybackSupported with null streamInfo
 * @tc.number: OH_AudioStreamManager_IsOffloadPlaybackSupported_NullStreamInfo
 * @tc.desc  : Test IsOffloadPlaybackSupported with null streamInfo returns false
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsOffloadPlaybackSupported_NullStreamInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    auto result = OH_AudioStreamManager_IsOffloadPlaybackSupported(audioStreamManager, nullptr, usage);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsAcousticEchoCancelerSupported with all valid source types
 * @tc.number: OH_AudioStreamManager_IsAcousticEchoCancelerSupported_AllSources
 * @tc.desc  : Test IsAcousticEchoCancelerSupported with all valid source types returns SUCCESS
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsAcousticEchoCancelerSupported_AllSources,
    TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");

    std::vector<OH_AudioStream_SourceType> validSources = {
        AUDIOSTREAM_SOURCE_TYPE_MIC,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION,
        AUDIOSTREAM_SOURCE_TYPE_PLAYBACK_CAPTURE,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_CALL,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_DOWNLINK,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_MESSAGE,
        AUDIOSTREAM_SOURCE_TYPE_UNPROCESSED,
        AUDIOSTREAM_SOURCE_TYPE_CAMCORDER,
        AUDIOSTREAM_SOURCE_TYPE_LIVE,
    };
    int callCount = 0;
    for (auto sourceType : validSources) {
        bool supported = false;
        auto result = OH_AudioStreamManager_IsAcousticEchoCancelerSupported(audioStreamManager, sourceType,
            &supported);
        AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS,
            "sourceType=%d failed, ret=%d", sourceType, result);
        callCount++;
    }
    AIUT_EXPECT_EQ_INFO_LOG(callCount, static_cast<int>(validSources.size()),
        "not all sources tested, count=%d", callCount);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastPlaybackSupported with all valid usages
 * @tc.number: OH_AudioStreamManager_IsFastPlaybackSupported_AllUsages
 * @tc.desc  : Test IsFastPlaybackSupported with all valid usages completes without crash
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsFastPlaybackSupported_AllUsages, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");
    OH_AudioStreamInfo streamInfo;
    streamInfo.samplingRate = 48000;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    streamInfo.sampleFormat = AUDIOSTREAM_SAMPLE_S24LE;

    std::vector<OH_AudioStream_Usage> validUsages = {
        AUDIOSTREAM_USAGE_MUSIC,
        AUDIOSTREAM_USAGE_VOICE_COMMUNICATION,
        AUDIOSTREAM_USAGE_VOICE_ASSISTANT,
        AUDIOSTREAM_USAGE_ALARM,
        AUDIOSTREAM_USAGE_VOICE_MESSAGE,
        AUDIOSTREAM_USAGE_RINGTONE,
        AUDIOSTREAM_USAGE_NOTIFICATION,
        AUDIOSTREAM_USAGE_MOVIE,
        AUDIOSTREAM_USAGE_GAME,
        AUDIOSTREAM_USAGE_AUDIOBOOK,
        AUDIOSTREAM_USAGE_NAVIGATION,
        AUDIOSTREAM_USAGE_VIDEO_COMMUNICATION,
    };
    int callCount = 0;
    for (auto usage : validUsages) {
        OH_AudioStreamManager_IsFastPlaybackSupported(audioStreamManager, &streamInfo, usage);
        callCount++;
    }
    AIUT_EXPECT_EQ_INFO_LOG(callCount, static_cast<int>(validUsages.size()),
        "not all usages tested, count=%d", callCount);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsIntelligentNoiseReduction with all valid sources
 * @tc.number: OH_AudioStreamManager_IsIntelligentNoiseReduction_AllSources
 * @tc.desc  : Test IsIntelligentNoiseReductionEnabledForCurrentDevice with all valid sources completes
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioStreamManagerAiut, OH_AudioStreamManager_IsIntelligentNoiseReduction_AllSources, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioStreamManager *audioStreamManager = nullptr;
    auto ret = OH_AudioManager_GetAudioStreamManager(&audioStreamManager);
    AIUT_EXPECT_EQ_INFO_LOG(ret, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioStreamManager failed, ret=%d", ret);
    AIUT_ASSERT_NE_INFO_LOG(audioStreamManager, nullptr, "audioStreamManager is null");

    std::vector<OH_AudioStream_SourceType> validSources = {
        AUDIOSTREAM_SOURCE_TYPE_MIC,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION,
        AUDIOSTREAM_SOURCE_TYPE_PLAYBACK_CAPTURE,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_CALL,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_DOWNLINK,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION,
        AUDIOSTREAM_SOURCE_TYPE_VOICE_MESSAGE,
        AUDIOSTREAM_SOURCE_TYPE_UNPROCESSED,
        AUDIOSTREAM_SOURCE_TYPE_CAMCORDER,
        AUDIOSTREAM_SOURCE_TYPE_LIVE,
    };
    int callCount = 0;
    for (auto sourceType : validSources) {
        OH_AudioStreamManager_IsIntelligentNoiseReductionEnabledForCurrentDevice(audioStreamManager, sourceType);
        callCount++;
    }
    AIUT_EXPECT_EQ_INFO_LOG(callCount, static_cast<int>(validSources.size()),
        "not all sources tested, count=%d", callCount);
}

} // namespace AudioStandard
} // namespace OHOS