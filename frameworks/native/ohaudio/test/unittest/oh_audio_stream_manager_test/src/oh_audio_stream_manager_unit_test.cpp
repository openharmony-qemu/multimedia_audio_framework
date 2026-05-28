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

#include "oh_audio_stream_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioStreamManagerUnitTest::SetUpTestCase(void) { }

void OHAudioStreamManagerUnitTest::TearDownTestCase(void) { }

void OHAudioStreamManagerUnitTest::SetUp(void) { }

void OHAudioStreamManagerUnitTest::TearDown(void) { }

/**
 * @tc.name  : Test OH_AudioStreamManager_IsAcousticEchoCancelerSupported.
 * @tc.number: OH_AudioStreamManager_IsAcousticEchoCancelerSupported_001
 * @tc.desc  : Test OH_AudioStreamManager_IsAcousticEchoCancelerSupported.
 */
HWTEST(OHAudioStreamManagerUnitTest, OH_AudioStreamManager_IsAcousticEchoCancelerSupported_001, TestSize.Level0)
{
    OH_AudioStreamManager *audioStreamManager = nullptr;
    bool supported = false;
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_MIC;
    auto result = OH_AudioStreamManager_IsAcousticEchoCancelerSupported(audioStreamManager, sourceType, &supported);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastPlaybackSupported.
 * @tc.number: OH_AudioStreamManager_IsFastPlaybackSupported_001
 * @tc.desc  : Test OH_AudioStreamManager_IsFastPlaybackSupported.
 */
HWTEST(OHAudioStreamManagerUnitTest, OH_AudioStreamManager_IsFastPlaybackSupported_001, TestSize.Level0)
{
    OH_AudioStreamManager *audioStreamManager = nullptr;
    OH_AudioStreamInfo *streamInfo = nullptr;
    OH_AudioStream_Usage usage = static_cast<OH_AudioStream_Usage>(100);
    auto result = OH_AudioStreamManager_IsFastPlaybackSupported(audioStreamManager, streamInfo, usage);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsFastRecordingSupported.
 * @tc.number: OH_AudioStreamManager_IsFastRecordingSupported_001
 * @tc.desc  : Test OH_AudioStreamManager_IsFastRecordingSupported.
 */
HWTEST(OHAudioStreamManagerUnitTest, OH_AudioStreamManager_IsFastRecordingSupported_001, TestSize.Level0)
{
    OH_AudioStreamManager *audioStreamManager = nullptr;
    OH_AudioStreamInfo *streamInfo = nullptr;
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_MIC;
    auto result = OH_AudioStreamManager_IsFastRecordingSupported(audioStreamManager, streamInfo, sourceType);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsMultichannelPlaybackSupported.
 * @tc.number: OH_AudioStreamManager_IsMultichannelPlaybackSupported_001
 * @tc.desc  : Test OH_AudioStreamManager_IsMultichannelPlaybackSupported.
 */
HWTEST(OHAudioStreamManagerUnitTest, OH_AudioStreamManager_IsMultichannelPlaybackSupported_001, TestSize.Level0)
{
    OH_AudioStreamManager *audioStreamManager = nullptr;
    OH_AudioStreamInfo *streamInfo = nullptr;
    OH_AudioStream_Usage usage = static_cast<OH_AudioStream_Usage>(100);
    auto result = OH_AudioStreamManager_IsMultichannelPlaybackSupported(audioStreamManager, streamInfo, usage);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsDirectPlaybackSupported.
 * @tc.number: OH_AudioStreamManager_IsDirectPlaybackSupported_001
 * @tc.desc  : Test OH_AudioStreamManager_IsDirectPlaybackSupported.
 */
HWTEST(OHAudioStreamManagerUnitTest, OH_AudioStreamManager_IsDirectPlaybackSupported_001, TestSize.Level0)
{
    OH_AudioStreamManager *audioStreamManager = nullptr;
    OH_AudioStreamInfo *streamInfo = nullptr;
    OH_AudioStream_Usage usage = static_cast<OH_AudioStream_Usage>(100);
    auto result = OH_AudioStreamManager_IsDirectPlaybackSupported(audioStreamManager, streamInfo, usage);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test OH_AudioStreamManager_IsOffloadPlaybackSupported.
 * @tc.number: OH_AudioStreamManager_IsOffloadPlaybackSupported_001
 * @tc.desc  : Test OH_AudioStreamManager_IsOffloadPlaybackSupported.
 */
HWTEST(OHAudioStreamManagerUnitTest, OH_AudioStreamManager_IsOffloadPlaybackSupported_001, TestSize.Level0)
{
    OH_AudioStreamManager *audioStreamManager = nullptr;
    OH_AudioStreamInfo *streamInfo = nullptr;
    OH_AudioStream_Usage usage = static_cast<OH_AudioStream_Usage>(100);
    auto result = OH_AudioStreamManager_IsOffloadPlaybackSupported(audioStreamManager, streamInfo, usage);
    EXPECT_EQ(result, false);
}
} // namespace AudioStandard
} // namespace OHOS
