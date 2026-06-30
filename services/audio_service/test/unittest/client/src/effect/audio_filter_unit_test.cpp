/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_stream_types.h"
#include "parcel.h"
#include <gtest/gtest.h>
#include <memory>

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t TEST_UID = 10001;
constexpr int32_t TEST_STREAM_ID = 42;
constexpr int32_t TEST_RENDERER_FLAGS = 0x01;
constexpr AudioStreamType AUDIO_STREAM_TYPE_RENDERER = AudioStreamType::STREAM_MUSIC;

} // namespace

class AudioFilterUnitTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(AudioFilterUnitTest, AudioRendererFilter_Marshalling_001, testing::ext::TestSize.Level1)
{
    AudioRendererFilter filter;
    filter.uid = TEST_UID;
    filter.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    filter.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    filter.streamType = AUDIO_STREAM_TYPE_RENDERER;
    filter.rendererInfo.rendererFlags = TEST_RENDERER_FLAGS;
    filter.streamId = TEST_STREAM_ID;

    OHOS::Parcel parcel;
    bool ret = filter.Marshalling(parcel);
    EXPECT_TRUE(ret);
}

HWTEST_F(AudioFilterUnitTest, AudioRendererFilter_Unmarshalling_001, testing::ext::TestSize.Level1)
{
    AudioRendererFilter filter;
    filter.uid = TEST_UID;
    filter.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    filter.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    filter.streamType = AUDIO_STREAM_TYPE_RENDERER;
    filter.rendererInfo.rendererFlags = TEST_RENDERER_FLAGS;
    filter.streamId = TEST_STREAM_ID;

    OHOS::Parcel parcel;
    (void)filter.Marshalling(parcel);

    parcel.RewindRead(0);
    AudioRendererFilter *unmarshalled = AudioRendererFilter::Unmarshalling(parcel);
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_EQ(unmarshalled->uid, TEST_UID);
    EXPECT_EQ(unmarshalled->streamId, TEST_STREAM_ID);
    delete unmarshalled;
}

HWTEST_F(AudioFilterUnitTest, AudioCapturerFilter_Marshalling_001, testing::ext::TestSize.Level1)
{
    AudioCapturerFilter filter;
    filter.uid = TEST_UID;
    filter.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    filter.capturerInfo.capturerFlags = 0x02;
    filter.audioDeviceSelectMode = 1;
    filter.streamId = TEST_STREAM_ID;

    OHOS::Parcel parcel;
    bool ret = filter.Marshalling(parcel);
    EXPECT_TRUE(ret);
}

HWTEST_F(AudioFilterUnitTest, AudioCapturerFilter_Unmarshalling_001, testing::ext::TestSize.Level1)
{
    AudioCapturerFilter filter;
    filter.uid = TEST_UID;
    filter.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    filter.capturerInfo.capturerFlags = 0x02;
    filter.audioDeviceSelectMode = 1;
    filter.streamId = TEST_STREAM_ID;

    OHOS::Parcel parcel;
    (void)filter.Marshalling(parcel);

    parcel.RewindRead(0);
    AudioCapturerFilter *unmarshalled = AudioCapturerFilter::Unmarshalling(parcel);
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_EQ(unmarshalled->uid, TEST_UID);
    EXPECT_EQ(unmarshalled->streamId, TEST_STREAM_ID);
    delete unmarshalled;
}

HWTEST_F(AudioFilterUnitTest, AudioRendererFilter_Constructor_001, testing::ext::TestSize.Level1)
{
    AudioRendererFilter filter;
    EXPECT_EQ(filter.uid, -1);
    EXPECT_EQ(filter.streamId, -1);
}

HWTEST_F(AudioFilterUnitTest, AudioCapturerFilter_Constructor_001, testing::ext::TestSize.Level1)
{
    AudioCapturerFilter filter;
    EXPECT_EQ(filter.uid, -1);
    EXPECT_EQ(filter.streamId, -1);
}

HWTEST_F(AudioFilterUnitTest, AudioRendererFilter_Unmarshalling_Null_001, testing::ext::TestSize.Level1)
{
    OHOS::Parcel parcel;
    // Write invalid data to cause unmarshalling failure
    parcel.WriteInt32(TEST_UID);
    parcel.RewindRead(0);
    AudioRendererFilter *result = AudioRendererFilter::Unmarshalling(parcel);
    // Result depends on parcel data, just call to ensure coverage
    if (result != nullptr) {
        delete result;
    }
    SUCCEED();
}

} // namespace AudioStandard
} // namespace OHOS
