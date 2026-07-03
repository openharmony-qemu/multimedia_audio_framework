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

#include "audio_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/audio_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_stream_enum.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_CAPTURE_ID = 1;
constexpr uint32_t TEST_CAPTURE_ID_2 = 10004;
constexpr uint32_t TEST_STREAM_ID = 100000;
constexpr uint32_t TEST_SAMPLE_RATE_48000 = 48000;
constexpr uint32_t TEST_CHANNEL_STEREO = 2;
constexpr uint32_t TEST_CHANNEL_LAYOUT_4POINT0 = CH_LAYOUT_4POINT0;
constexpr uint32_t TEST_UNIQUE_ID_INTERVAL = 8;
constexpr uint32_t TEST_AUDIO_INPUT_VOICE_MESSAGE_TYPE = 1u << 18;
constexpr size_t TEST_DEFAULT_SIZE = 8;
constexpr float TEST_VOLUME_FULL = 1.0f;
} // namespace

// ==================== AudioCaptureSourceAiut ====================

void AudioCaptureSourceAiut::SetUpTestCase(void) {}
void AudioCaptureSourceAiut::TearDownTestCase(void) {}
void AudioCaptureSourceAiut::SetUp(void) {}
void AudioCaptureSourceAiut::TearDown(void) {}

IAudioSourceAttr AudioCaptureSourceAiut::CreatePrimaryAttr()
{
    IAudioSourceAttr attr = {};
    attr.adapterName = "primary";
    attr.sampleRate = TEST_SAMPLE_RATE_48000;
    attr.channel = TEST_CHANNEL_STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = TEST_CHANNEL_LAYOUT_4POINT0;
    attr.deviceType = DEVICE_TYPE_MIC;
    attr.openMicSpeaker = 1;
    return attr;
}

IAudioSourceAttr AudioCaptureSourceAiut::CreateUsbAttr()
{
    IAudioSourceAttr attr = {};
    attr.adapterName = "usb";
    attr.channel = TEST_CHANNEL_STEREO;
    return attr;
}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test AudioCaptureSource construction
 * @tc.number: AudioCaptureSource_Constructor_001
 * @tc.desc  : Test AudioCaptureSource constructor creates uninitialized object.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test AudioCaptureSource destruction without init
 * @tc.number: AudioCaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID_2, "test_device");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== GetChannelCountByChannelLayout Tests ====================

/**
 * @tc.name  : Test GetChannelCountByChannelLayout
 * @tc.number: AudioCaptureSource_GetChannelCountByChannelLayout_001
 * @tc.desc  : Test GetChannelCountByChannelLayout returns correct channel count for various layouts.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetChannelCountByChannelLayout_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");

    uint64_t channelCount = 0;
    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    AIUT_EXPECT_EQ_INFO_LOG(channelCount, static_cast<uint64_t>(AudioChannel::STEREO),
        "UNKNOWN layout should return STEREO, got %lu", channelCount);

    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(
        AudioChannelLayout::CH_LAYOUT_MONO);
    AIUT_EXPECT_EQ_INFO_LOG(channelCount, static_cast<uint64_t>(AudioChannel::MONO),
        "MONO layout should return MONO, got %lu", channelCount);

    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(
        AudioChannelLayout::CH_LAYOUT_STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(channelCount, static_cast<uint64_t>(AudioChannel::STEREO),
        "STEREO layout should return STEREO, got %lu", channelCount);

    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(
        AudioChannelLayout::CH_LAYOUT_4POINT0);
    AIUT_EXPECT_EQ_INFO_LOG(channelCount, static_cast<uint64_t>(AudioChannel::CHANNEL_4),
        "4POINT0 layout should return CHANNEL_4, got %lu", channelCount);
}

// ==================== MappingAudioInputType Tests ====================

/**
 * @tc.name  : Test MappingAudioInputType
 * @tc.number: AudioCaptureSource_MappingAudioInputType_001
 * @tc.desc  : Test MappingAudioInputType returns correct type for known and unknown strings.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_MappingAudioInputType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");

    AudioInputType result = source->MappingAudioInputType("AUDIO_INPUT_VOICE_TRANSCRIPTION");
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_INPUT_VOICE_TRANSCRIPTION,
        "expected AUDIO_INPUT_VOICE_TRANSCRIPTION, got %d", static_cast<int32_t>(result));

    result = source->MappingAudioInputType("AUDIO_INPUT_VOICE_MESSAGE_TYPE");
    AIUT_EXPECT_EQ_INFO_LOG(result, static_cast<AudioInputType>(TEST_AUDIO_INPUT_VOICE_MESSAGE_TYPE),
        "expected AUDIO_INPUT_VOICE_MESSAGE_TYPE, got %d", static_cast<int32_t>(result));

    result = source->MappingAudioInputType("AUDIO_INPUT_INVALID");
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_INPUT_MIC_TYPE,
        "expected AUDIO_INPUT_MIC_TYPE for invalid, got %d", static_cast<int32_t>(result));
}

// ==================== ConvertToHDIAudioInputType Tests ====================

/**
 * @tc.name  : Test ConvertToHDIAudioInputType
 * @tc.number: AudioCaptureSource_ConvertToHDIAudioInputType_001
 * @tc.desc  : Test ConvertToHDIAudioInputType returns correct type.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_ConvertToHDIAudioInputType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");

    std::string hdiSourceType = "AUDIO_INPUT_VOICE_TRANSCRIPTION";
    AudioInputType result = source->ConvertToHDIAudioInputType(1, hdiSourceType);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_INPUT_VOICE_TRANSCRIPTION,
        "expected AUDIO_INPUT_VOICE_TRANSCRIPTION, got %d", static_cast<int32_t>(result));

    result = source->ConvertToHDIAudioInputType(SOURCE_TYPE_VOICE_MESSAGE, "AUDIO_INPUT_DEFAULT_TYPE");
    AIUT_EXPECT_EQ_INFO_LOG(result, static_cast<AudioInputType>(TEST_AUDIO_INPUT_VOICE_MESSAGE_TYPE),
        "expected VOICE_MESSAGE input type, got %d", static_cast<int32_t>(result));
}

// ==================== GetUniqueIdBySourceType Tests ====================

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_EC
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_001
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_EC.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_EC;
    source->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_EC * TEST_UNIQUE_ID_INTERVAL,
        "expected EC offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_MIC_REF
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_002
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_MIC_REF.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_MIC_REF;
    source->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_MIC_REF * TEST_UNIQUE_ID_INTERVAL,
        "expected MIC_REF offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_WAKEUP
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_003
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_WAKEUP.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_WAKEUP;
    source->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_WAKEUP * TEST_UNIQUE_ID_INTERVAL,
        "expected WAKEUP offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_VOICE_TRANSCRIPTION
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_004
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_VOICE_TRANSCRIPTION.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_004, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    source->attr_.hdiSourceType = "AUDIO_INPUT_VOICE_TRANSCRIPTION";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_VOICE_TRANSCRIPTION * TEST_UNIQUE_ID_INTERVAL,
        "expected VOICE_TRANSCRIPTION offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_UNPROCESSED
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_005
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_UNPROCESSED.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_005, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_UNPROCESSED;
    source->attr_.hdiSourceType = "AUDIO_INPUT_RAW_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_UNPROCESS * TEST_UNIQUE_ID_INTERVAL,
        "expected UNPROCESS offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_OFFLOAD_CAPTURE
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_006
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_OFFLOAD_CAPTURE.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_006, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_OFFLOAD_CAPTURE;
    source->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_OFFLOAD_CAPTURE * TEST_UNIQUE_ID_INTERVAL,
        "expected OFFLOAD_CAPTURE offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_INVALID
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_007
 * @tc.desc  : Test GetUniqueIdBySourceType falls back to PRIMARY for invalid source type.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_007, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_INVALID;
    source->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY * TEST_UNIQUE_ID_INTERVAL,
        "expected PRIMARY offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_INVALID with empty hdiSourceType
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_008
 * @tc.desc  : Test GetUniqueIdBySourceType falls back to PRIMARY with empty hdiSourceType.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_008, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_INVALID;
    source->attr_.hdiSourceType = "";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY * TEST_UNIQUE_ID_INTERVAL,
        "expected PRIMARY offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_ULTRASONIC
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_009
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_ULTRASONIC.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_009, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_ULTRASONIC;
    source->attr_.hdiSourceType = "AUDIO_INPUT_ULTRASONIC_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_ULTRASONIC * TEST_UNIQUE_ID_INTERVAL,
        "expected ULTRASONIC offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_VOICE_RECOGNITION
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_010
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_VOICE_RECOGNITION.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_010, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    source->attr_.hdiSourceType = "AUDIO_INPUT_VOICE_RECOGNITION_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_VOICE_RECOGNITION * TEST_UNIQUE_ID_INTERVAL,
        "expected VOICE_RECOGNITION offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_011
 * @tc.desc  : Test GetUniqueIdBySourceType for SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_011, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT;
    source->attr_.hdiSourceType = "AUDIO_INPUT_RAW_AI_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_UNPROCESSED_VOICE_ASSISTANT * TEST_UNIQUE_ID_INTERVAL,
        "expected UNPROCESSED_VOICE_ASSISTANT offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_INTERPHONE
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_012
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_INTERPHONE.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_012, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_INTERPHONE;
    source->attr_.hdiSourceType = "AUDIO_INPUT_INTERPHONE_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_INTERPHONE * TEST_UNIQUE_ID_INTERVAL,
        "expected INTERPHONE offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_LIVE
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_013
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_LIVE.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_013, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_LIVE;
    source->attr_.hdiSourceType = "AUDIO_INPUT_LIVE_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_LIVE * TEST_UNIQUE_ID_INTERVAL,
        "expected LIVE offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_CAMCORDER
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_014
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_CAMCORDER.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_014, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_CAMCORDER;
    source->attr_.hdiSourceType = "AUDIO_INPUT_CAMCORDER_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_CAMCORDER * TEST_UNIQUE_ID_INTERVAL,
        "expected CAMCORDER offset id, got %u", id);
}

/**
 * @tc.name  : Test GetUniqueIdBySourceType for SOURCE_TYPE_VOICE_MESSAGE
 * @tc.number: AudioCaptureSource_GetUniqueIdBySourceType_015
 * @tc.desc  : Test GetUniqueIdBySourceType returns correct ID for SOURCE_TYPE_VOICE_MESSAGE.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetUniqueIdBySourceType_015, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_VOICE_MESSAGE;
    source->attr_.hdiSourceType = "AUDIO_INPUT_VOICE_MESSAGE_TYPE";
    uint32_t id = source->GetUniqueIdBySourceType();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_VOICE_MESSAGE * TEST_UNIQUE_ID_INTERVAL,
        "expected VOICE_MESSAGE offset id, got %u", id);
}

// ==================== GetMultiDeviceUniqueId Tests ====================

/**
 * @tc.name  : Test GetMultiDeviceUniqueId for primary extra typec capturer
 * @tc.number: AudioCaptureSource_GetMultiDeviceUniqueId_001
 * @tc.desc  : Test GetMultiDeviceUniqueId returns correct ID for primary extra typec.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetMultiDeviceUniqueId_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID,
        HDI_ID_INFO_PRIMARY_EXTRA_CAPTURER_TYPEC);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    uint32_t id = source->GetMultiDeviceUniqueId();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY_EXTRA_TYPEC * TEST_UNIQUE_ID_INTERVAL,
        "expected PRIMARY_EXTRA_TYPEC id, got %u", id);
}

/**
 * @tc.name  : Test GetMultiDeviceUniqueId for primary extra wired capturer
 * @tc.number: AudioCaptureSource_GetMultiDeviceUniqueId_002
 * @tc.desc  : Test GetMultiDeviceUniqueId returns correct ID for primary extra wired.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetMultiDeviceUniqueId_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID,
        HDI_ID_INFO_PRIMARY_EXTRA_CAPTURER_WIRED);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    uint32_t id = source->GetMultiDeviceUniqueId();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY_EXTRA_WIRED * TEST_UNIQUE_ID_INTERVAL,
        "expected PRIMARY_EXTRA_WIRED id, got %u", id);
}

/**
 * @tc.name  : Test GetMultiDeviceUniqueId for primary extra bt capturer
 * @tc.number: AudioCaptureSource_GetMultiDeviceUniqueId_003
 * @tc.desc  : Test GetMultiDeviceUniqueId returns correct ID for primary extra bt nearlink.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetMultiDeviceUniqueId_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID,
        HDI_ID_INFO_PRIMARY_EXTRA_CAPTURER_BT_NEARLINK);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    uint32_t id = source->GetMultiDeviceUniqueId();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY_EXTRA_BT_NER * TEST_UNIQUE_ID_INTERVAL,
        "expected PRIMARY_EXTRA_BT_NER id, got %u", id);
}

/**
 * @tc.name  : Test GetMultiDeviceUniqueId fallback to GetUniqueIdBySourceType
 * @tc.number: AudioCaptureSource_GetMultiDeviceUniqueId_004
 * @tc.desc  : Test GetMultiDeviceUniqueId falls back when halName does not match extra types.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_GetMultiDeviceUniqueId_004, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->attr_.sourceType = SOURCE_TYPE_WAKEUP;
    uint32_t id = source->GetMultiDeviceUniqueId();
    AIUT_EXPECT_EQ_INFO_LOG(id,
        AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_WAKEUP * TEST_UNIQUE_ID_INTERVAL,
        "expected WAKEUP fallback id, got %u", id);
}

// ==================== ConvertToAudioInputRouteFlag Tests ====================

/**
 * @tc.name  : Test ConvertToAudioInputRouteFlag
 * @tc.number: AudioCaptureSource_ConvertToAudioInputRouteFlag_001
 * @tc.desc  : Test ConvertToAudioInputRouteFlag returns correct flag for various source types.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_ConvertToAudioInputRouteFlag_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");

    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_VOICE_TRANSCRIPTION),
        AUDIO_INPUT_FLAG_AI, "VOICE_TRANSCRIPTION should be AI flag");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_UNPROCESSED),
        AUDIO_INPUT_FLAG_UNPROCESS, "UNPROCESSED should be UNPROCESS flag");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_LIVE),
        AUDIO_INPUT_FLAG_LIVE, "LIVE should be LIVE flag");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_VOICE_MESSAGE),
        AUDIO_INPUT_FLAG_VOICE_MESSAGE, "VOICE_MESSAGE should be VOICE_MESSAGE flag");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_ULTRASONIC),
        AUDIO_INPUT_FLAG_ULTRASONIC, "ULTRASONIC should be ULTRASONIC flag");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_VOICE_RECOGNITION),
        AUDIO_INPUT_FLAG_VOICE_RECOGNITION, "VOICE_RECOGNITION flag mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT),
        AUDIO_INPUT_FLAG_RAW_AI, "UNPROCESSED_VOICE_ASSISTANT should be RAW_AI flag");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_WAKEUP),
        AUDIO_INPUT_FLAG_WAKEUP, "WAKEUP should be WAKEUP flag");
    AIUT_EXPECT_EQ_INFO_LOG(source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_OFFLOAD_CAPTURE),
        AUDIO_INPUT_FLAG_OFFLOAD, "OFFLOAD_CAPTURE should be OFFLOAD flag");
}

/**
 * @tc.name  : Test ConvertToAudioInputRouteFlag for SOURCE_TYPE_INTERPHONE
 * @tc.number: AudioCaptureSource_ConvertToAudioInputRouteFlag_002
 * @tc.desc  : Test ConvertToAudioInputRouteFlag returns INTERPHONE flag.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_ConvertToAudioInputRouteFlag_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AudioFlag result = source->ConvertToAudioInputRouteFlag(SOURCE_TYPE_INTERPHONE);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_INPUT_FLAG_INTERPHONE,
        "expected INTERPHONE flag, got %d", static_cast<int32_t>(result));
}

// ==================== UpdateRawSampleAttr Tests ====================

/**
 * @tc.name  : Test UpdateRawSampleAttr for SOURCE_TYPE_VOICE_RECOGNITION
 * @tc.number: AudioCaptureSource_UpdateRawSampleAttr_001
 * @tc.desc  : Test UpdateRawSampleAttr sets correct raw attributes for VOICE_RECOGNITION.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_UpdateRawSampleAttr_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");

    source->attr_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    source->attr_.formatMicIn = SAMPLE_S16LE;
    source->attr_.sampleRateMicIn = SAMPLE_RATE_16000;
    source->attr_.channelMicIn = CHANNEL_4;

    struct AudioSampleAttributes param = {};
    source->UpdateRawSampleAttr(param);
    AIUT_EXPECT_EQ_INFO_LOG(param.rawSampleAttributes.rawFormat, AUDIO_FORMAT_TYPE_PCM_16_BIT,
        "rawFormat mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(param.rawSampleAttributes.rawSampleRate, SAMPLE_RATE_16000,
        "rawSampleRate mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(param.rawSampleAttributes.rawChannelLayout, CH_LAYOUT_QUAD,
        "rawChannelLayout mismatch");
}

/**
 * @tc.name  : Test UpdateRawSampleAttr for SOURCE_TYPE_MIC (no raw attr)
 * @tc.number: AudioCaptureSource_UpdateRawSampleAttr_002
 * @tc.desc  : Test UpdateRawSampleAttr does not modify param when sourceType is MIC.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_UpdateRawSampleAttr_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");

    source->attr_.sourceType = SOURCE_TYPE_MIC;
    struct AudioSampleAttributes param = {};
    AudioFormatType expectFormat = param.rawSampleAttributes.rawFormat;
    source->UpdateRawSampleAttr(param);
    AIUT_EXPECT_EQ_INFO_LOG(param.rawSampleAttributes.rawFormat, expectFormat,
        "rawFormat should not change for SOURCE_TYPE_MIC");
}

/**
 * @tc.name  : Test UpdateRawSampleAttr for SOURCE_TYPE_CAMCORDER
 * @tc.number: AudioCaptureSource_UpdateRawSampleAttr_003
 * @tc.desc  : Test UpdateRawSampleAttr sets correct raw attributes for CAMCORDER.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_UpdateRawSampleAttr_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");

    source->attr_.sourceType = SOURCE_TYPE_CAMCORDER;
    source->attr_.formatMicIn = SAMPLE_S24LE;
    source->attr_.sampleRateMicIn = SAMPLE_RATE_48000;
    source->attr_.channelMicIn = CHANNEL_4;

    struct AudioSampleAttributes param = {};
    source->UpdateRawSampleAttr(param);
    AIUT_EXPECT_EQ_INFO_LOG(param.rawSampleAttributes.rawFormat, AUDIO_FORMAT_TYPE_PCM_24_BIT,
        "rawFormat mismatch for CAMCORDER");
    AIUT_EXPECT_EQ_INFO_LOG(param.rawSampleAttributes.rawSampleRate, SAMPLE_RATE_48000,
        "rawSampleRate mismatch for CAMCORDER");
    AIUT_EXPECT_EQ_INFO_LOG(param.rawSampleAttributes.rawChannelLayout, CH_LAYOUT_QUAD,
        "rawChannelLayout mismatch for CAMCORDER");
}

// ==================== Stop Without Init Tests ====================

/**
 * @tc.name  : Test Stop without init
 * @tc.number: AudioCaptureSource_StopWithoutInit_001
 * @tc.desc  : Test Stop() called before Init() returns SUCCESS.
 */
HWTEST_F(AudioCaptureSourceAiut, AudioCaptureSource_StopWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<AudioCaptureSource>(TEST_CAPTURE_ID_2, "test_device");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Stop without init, got %d", ret);
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

} // namespace AudioStandard
} // namespace OHOS
