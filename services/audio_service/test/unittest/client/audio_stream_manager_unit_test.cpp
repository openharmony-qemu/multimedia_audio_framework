/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_stream_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioSteamManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test IsStreamActive API
 * @tc.type  : FUNC
 * @tc.number: IsStreamActive_001
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioStreamManagerUnitTest, IsStreamActive_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_001 start");
    bool result = AudioStreamManager::GetInstance()->IsStreamActive(STREAM_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_001 result1:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioStreamManager::GetInstance()->IsStreamActive(STREAM_VOICE_ASSISTANT);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_001 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioStreamManager::GetInstance()->IsStreamActive(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_001 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioStreamManager::GetInstance()->IsStreamActive(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_001 result3:%{public}d", result);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test IsStreamActiveByStreamUsage API
 * @tc.number: IsStreamActiveByStreamUsage_001
 * @tc.desc  : Test IsStreamActiveByStreamUsage interface.
 */
HWTEST(AudioStreamManagerUnitTest, IsStreamActiveByStreamUsage_001, TestSize.Level1)
{
    StreamUsage ILLEGAL_STREAM_USAGE = static_cast<StreamUsage>(static_cast<int32_t>(STREAM_USAGE_MAX)+999);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActiveByStreamUsage_001 start");
    bool result = AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActiveByStreamUsage_001 result1:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActiveByStreamUsage_001 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(ILLEGAL_STREAM_USAGE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActiveByStreamUsage_001 result2:%{public}d", result);
    EXPECT_EQ(result, false);
}
/**
 * @tc.name  : Test GetHardeareOutputSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetHardeareOutputSamplingRate_001
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetHardwareOutputSamplingRate_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamManager::GetInstance()->GetHardwareOutputSamplingRate(desc);

    EXPECT_EQ(desc->deviceRole_, -1);
}

/**
 * @tc.name  : Test IsStreamActiveByStreamUsage API
 * @tc.type  : FUNC
 * @tc.number: IsStreamActiveByStreamUsage_001
 * @tc.desc  : Test IsStreamActiveByStreamUsage interface.
 */
HWTEST(AudioStreamManagerUnitTest, IsStreamActiveByStreamUsage_002, TestSize.Level1)
{
    EXPECT_FALSE(AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_MEDIA));
    EXPECT_FALSE(AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_VOICE_COMMUNICATION));
    EXPECT_FALSE(AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_SYSTEM));
    EXPECT_FALSE(AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_DTMF));
    EXPECT_FALSE(AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_ENFORCED_TONE));
    EXPECT_FALSE(AudioStreamManager::GetInstance()->IsStreamActiveByStreamUsage(STREAM_USAGE_VOICE_CALL_ASSISTANT));
}
/**
 * @tc.name  : Test GetHardeareOutputSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetHardeareOutputSamplingRate_001
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetSupportedAudioEffectProperty_001, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;

    int32_t result = AudioStreamManager::GetInstance()->GetSupportedAudioEffectProperty(propertyArray);

    EXPECT_NE(result, 1);
}
/**
 * @tc.name  : Test GetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEffectPropertye_001
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetAudioEffectProperty_001, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;

    int32_t result = AudioStreamManager::GetInstance()->GetAudioEffectProperty(propertyArray);
    EXPECT_NE(result, 1);
}
/**
 * @tc.name  : Test GetAudioEnhanceProperty API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEnhanceProperty_001
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetAudioEnhanceProperty_001, TestSize.Level1)
{
    AudioEnhancePropertyArray propertyArray;

    int32_t result = AudioStreamManager::GetInstance()->GetAudioEnhanceProperty(propertyArray);

    EXPECT_NE(result, 1);
}
/**
 * @tc.name  : Test ForceStopAudioStream API
 * @tc.type  : FUNC
 * @tc.number: ForceStopAudioStream
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioStreamManagerUnitTest, ForceStopAudioStream_001, TestSize.Level1)
{
    StopAudioType audioType = STOP_ALL;

    int32_t result = AudioStreamManager::GetInstance()->ForceStopAudioStream(audioType);
    EXPECT_NE(result, 1);
}
/**
 * @tc.name  : Test IsAudioLoopbackSupported API
 * @tc.type  : FUNC
 * @tc.number: IsAudioLoopbackSupported_001
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioStreamManagerUnitTest, IsAudioLoopbackSupported_001, TestSize.Level1)
{
    AudioLoopbackMode mode = LOOPBACK_HARDWARE;

    int32_t result = AudioStreamManager::GetInstance()->IsAudioLoopbackSupported(mode);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test RegisterAudioRendererEventListener API
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioRendererEventListener_001
 * @tc.desc  : Test RegisterAudioRendererEventListener interface.
 */
HWTEST(AudioStreamManagerUnitTest, RegisterAudioRendererEventListener_001, TestSize.Level1)
{
    int32_t clientPid = 1;
    std::shared_ptr<AudioRendererStateChangeCallback> callback = nullptr;

    EXPECT_EQ(AudioStreamManager::GetInstance()
              ->RegisterAudioRendererEventListener(clientPid, callback), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetHardwareOutputSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetHardwareOutputSamplingRate_002
 * @tc.desc  : Test GetHardwareOutputSamplingRate interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetHardwareOutputSamplingRate_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    int32_t result = AudioStreamManager::GetInstance()->GetHardwareOutputSamplingRate(desc);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetHardwareOutputSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetHardwareOutputSamplingRate_003
 * @tc.desc  : Test GetHardwareOutputSamplingRate interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetHardwareOutputSamplingRate_003, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->deviceRole_ = OUTPUT_DEVICE;
    int32_t result = AudioStreamManager::GetInstance()->GetHardwareOutputSamplingRate(desc);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test GetSupportedAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: GetSupportedAudioEffectProperty_002
 * @tc.desc  : Test GetSupportedAudioEffectProperty interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetSupportedAudioEffectProperty_002, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    int32_t result = AudioStreamManager::GetInstance()->GetSupportedAudioEffectProperty(propertyArray);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test GetSupportedAudioEnhanceProperty API
 * @tc.type  : FUNC
 * @tc.number: GetSupportedAudioEnhanceProperty_001
 * @tc.desc  : Test GetSupportedAudioEnhanceProperty interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetSupportedAudioEnhanceProperty_001, TestSize.Level1)
{
    AudioEnhancePropertyArray propertyArray;
    int32_t result = AudioStreamManager::GetInstance()->GetSupportedAudioEnhanceProperty(propertyArray);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test SetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectProperty_001
 * @tc.desc  : Test SetAudioEffectProperty interface.
 */
HWTEST(AudioStreamManagerUnitTest, SetAudioEffectProperty_001, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    int32_t result = AudioStreamManager::GetInstance()->SetAudioEffectProperty(propertyArray);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test GetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEffectProperty_002
 * @tc.desc  : Test GetAudioEffectProperty interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetAudioEffectProperty_002, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    int32_t result = AudioStreamManager::GetInstance()->GetAudioEffectProperty(propertyArray);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test SetAudioEnhanceProperty API
 * @tc.type  : FUNC
 * @tc.number: SetAudioEnhanceProperty_001
 * @tc.desc  : Test SetAudioEnhanceProperty interface.
 */
HWTEST(AudioStreamManagerUnitTest, SetAudioEnhanceProperty_001, TestSize.Level1)
{
    AudioEnhancePropertyArray propertyArray;
    int32_t result = AudioStreamManager::GetInstance()->SetAudioEnhanceProperty(propertyArray);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test GetAudioEnhanceProperty API
 * @tc.type  : FUNC
 * @tc.number: GetAudioEnhanceProperty_002
 * @tc.desc  : Test GetAudioEnhanceProperty interface.
 */
HWTEST(AudioStreamManagerUnitTest, GetAudioEnhanceProperty_002, TestSize.Level1)
{
    AudioEnhancePropertyArray propertyArray;
    int32_t result = AudioStreamManager::GetInstance()->GetAudioEnhanceProperty(propertyArray);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test IsCapturerFocusAvailable API
 * @tc.type  : FUNC
 * @tc.number: IsCapturerFocusAvailable_001
 * @tc.desc  : Test IsCapturerFocusAvailable interface.
 */
HWTEST(AudioStreamManagerUnitTest, IsCapturerFocusAvailable_001, TestSize.Level1)
{
    AudioCapturerInfo capturerInfo;
    int32_t result = AudioStreamManager::GetInstance()->IsCapturerFocusAvailable(capturerInfo);
    EXPECT_EQ(result, 0);
}
} // namespace AudioStandard
} // namespace OHOS