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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_static_buffer_provider.h"
#include "oh_audio_buffer.h"
#include "audio_service_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioStaticBufferProviderUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioStaticBufferProviderUnitTest::SetUpTestCase(void) {}
void AudioStaticBufferProviderUnitTest::TearDownTestCase(void) {}
void AudioStaticBufferProviderUnitTest::SetUp(void) {}
void AudioStaticBufferProviderUnitTest::TearDown(void) {}

HWTEST(AudioStaticBufferProviderUnitTest, GetStaticBufferInfo_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.encoding = ENCODING_PCM;
    
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    auto sharedBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(sharedBuffer, nullptr);
    
    sharedBuffer->SetStaticMode(true);
    
    auto provider = AudioStaticBufferProvider::CreateInstance(streamInfo, sharedBuffer);
    ASSERT_NE(provider, nullptr);
    
    StaticBufferInfo staticBufferInfo;
    staticBufferInfo.totalLoopTimes_ = 5;
    staticBufferInfo.currentLoopTimes_ = 2;
    staticBufferInfo.curStaticDataPos_ = 100;
    
    provider->SetStaticBufferInfo(staticBufferInfo);
    
    StaticBufferInfo retrievedInfo;
    provider->GetStaticBufferInfo(retrievedInfo);
    
    EXPECT_EQ(retrievedInfo.totalLoopTimes_, 5);
    EXPECT_EQ(retrievedInfo.currentLoopTimes_, 2);
    EXPECT_EQ(retrievedInfo.curStaticDataPos_, 100);
}

HWTEST(AudioStaticBufferProviderUnitTest, GetStaticBufferInfo_002, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_44100;;
    streamInfo.channels = MONO;
    streamInfo.format = SAMPLE_S24LE;
    streamInfo.encoding = ENCODING_PCM;
    
    uint32_t totalSizeInFrame = 512;
    uint32_t byteSizePerFrame = 3;
    auto sharedBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(sharedBuffer, nullptr);
    
    sharedBuffer->SetStaticMode(true);
    
    auto provider = AudioStaticBufferProvider::CreateInstance(streamInfo, sharedBuffer);
    ASSERT_NE(provider, nullptr);
    
    StaticBufferInfo staticBufferInfo;
    staticBufferInfo.totalLoopTimes_ = -1;
    staticBufferInfo.currentLoopTimes_ = 0;
    staticBufferInfo.curStaticDataPos_ = 0;
    
    provider->SetStaticBufferInfo(staticBufferInfo);
    
    StaticBufferInfo retrievedInfo;
    provider->GetStaticBufferInfo(retrievedInfo);
    
    EXPECT_EQ(retrievedInfo.totalLoopTimes_, -1);
    EXPECT_EQ(retrievedInfo.currentLoopTimes_, 0);
    EXPECT_EQ(retrievedInfo.curStaticDataPos_, 0);
}

HWTEST(AudioStaticBufferProviderUnitTest, GetStaticBufferInfo_003, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.encoding = ENCODING_PCM;
    
    uint32_t totalSizeInFrame = 2048;
    uint32_t byteSizePerFrame = 4;
    auto sharedBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(sharedBuffer, nullptr);
    
    sharedBuffer->SetStaticMode(true);
    
    auto provider = AudioStaticBufferProvider::CreateInstance(streamInfo, sharedBuffer);
    ASSERT_NE(provider, nullptr);
    
    uint8_t buffer[1024];
    uint8_t *bufferPtr = buffer;
    size_t bufferSize = 1024;
    provider->SetProcessedBuffer(&bufferPtr, bufferSize);
    
    StaticBufferInfo retrievedInfo;
    provider->GetStaticBufferInfo(retrievedInfo);
    
    EXPECT_EQ(retrievedInfo.totalLoopTimes_, 0);
    EXPECT_EQ(retrievedInfo.currentLoopTimes_, 0);
    EXPECT_EQ(retrievedInfo.curStaticDataPos_, 0);
}

HWTEST(AudioStaticBufferProviderUnitTest, CreateInstance_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.encoding = ENCODING_PCM;
    
    auto provider = AudioStaticBufferProvider::CreateInstance(streamInfo, nullptr);
    EXPECT_EQ(provider, nullptr);
}

HWTEST(AudioStaticBufferProviderUnitTest, CreateInstance_002, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.encoding = ENCODING_PCM;
    
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    auto sharedBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(sharedBuffer, nullptr);
    
    auto provider = AudioStaticBufferProvider::CreateInstance(streamInfo, sharedBuffer);
    ASSERT_NE(provider, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS