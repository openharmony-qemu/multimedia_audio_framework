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

#include "audio_effect.h"
#include "audio_session_device_info.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "audio_stream_descriptor.h"
#include "audio_zone_info.h"
#include "hdi_adapter_type.h"
#include "audio_device_info.h"
#include "audio_device_stream_info.h"
#include "oh_audio_buffer.h"
#include "audio_shared_memory.h"
#include "audio_ipc_serialization_unit_test.h"

using namespace testing::ext;
namespace {
constexpr int32_t TEST_VALUE_1 = 10;
constexpr int32_t TEST_VALUE_2 = 20;
constexpr int32_t TEST_VALUE_3 = 30;
constexpr int32_t TEST_VALUE_4 = 40;
constexpr uint32_t TOTAL_SIZE_IN_FRAME = 882;
constexpr uint32_t BYTE_SIZE_PER_FRAME = 4;
const std::string TEST_STRING_VALUE_1 = "abc";
const std::string TEST_STRING_VALUE_2 = "cba";
const std::string TEST_STRING_VALUE_3 = "abcd";
const std::string TEST_STRING_VALUE_4 = "dbca";
}

namespace OHOS {
namespace AudioStandard {

void AudioIpcSerializationUnitTest::SetUpTestCase(void) {}
void AudioIpcSerializationUnitTest::TearDownTestCase(void) {}
void AudioIpcSerializationUnitTest::SetUp(void) {}
void AudioIpcSerializationUnitTest::TearDown(void) {}

static bool IsAppInfoEqual(const AppInfo &a, const AppInfo &b)
{
    return a.appFullTokenId == b.appFullTokenId &&
        a.appTokenId == b.appTokenId &&
        a.appPid == b.appPid &&
        a.appUid == b.appUid;
}

static bool IsStreamEffectModeEqual(const StreamEffectMode &a, const StreamEffectMode &b)
{
    if (a.mode != b.mode || a.devicePort.size() != b.devicePort.size()) {
        return false;
    }
    for (int32_t i = 0; i < a.devicePort.size(); i++) {
        if (a.devicePort[i].type != b.devicePort[i].type ||
            a.devicePort[i].chain != b.devicePort[i].chain) {
            return false;
        }
    }
    return true;
}

static bool IsProcessNewEqual(const ProcessNew &a, const ProcessNew &b)
{
    if (a.stream.size() != b.stream.size()) {
        return false;
    }
    for (int32_t i = 0; i < a.stream.size(); i++) {
        if (a.stream[i].priority != b.stream[i].priority ||
            a.stream[i].scene != b.stream[i].scene ||
            a.stream[i].streamEffectMode.size() != b.stream[i].streamEffectMode.size()) {
            return false;
        }
        for (int32_t j = 0; j < a.stream[i].streamEffectMode.size(); j++) {
            if (!IsStreamEffectModeEqual(a.stream[i].streamEffectMode[j], b.stream[i].streamEffectMode[j])) {
                return false;
            }
        }
    }
    return true;
}

static bool IsSceneMappingItemArrayEqual(const std::vector<SceneMappingItem> &a,
    const std::vector<SceneMappingItem> &b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (int32_t i = 0; i < a.size(); i++) {
        if (a[i].name != b[i].name || a[i].sceneType != b[i].sceneType) {
            return false;
        }
    }
    return true;
}

static bool IsAudioDeviceDescriptorEqual(const AudioDeviceDescriptor &a, const AudioDeviceDescriptor &b)
{
    return a.deviceType_ == b.deviceType_ &&
        a.deviceRole_ == b.deviceRole_ &&
        a.deviceId_ == b.deviceId_ &&
        a.channelMasks_ == b.channelMasks_ &&
        a.channelIndexMasks_ == b.channelIndexMasks_ &&
        a.deviceName_ == b.deviceName_ &&
        a.interruptGroupId_ == b.interruptGroupId_ &&
        a.volumeGroupId_ == b.volumeGroupId_ &&
        a.networkId_ == b.networkId_ &&
        a.dmDeviceType_ == b.dmDeviceType_ &&
        a.displayName_ == b.displayName_ &&
        a.audioStreamInfo_.size() == b.audioStreamInfo_.size() &&
        a.deviceCategory_ == b.deviceCategory_ &&
        a.connectState_ == b.connectState_ &&
        a.exceptionFlag_ == b.exceptionFlag_ &&
        a.connectTimeStamp_ == b.connectTimeStamp_ &&
        a.isScoRealConnected_ == b.isScoRealConnected_ &&
        a.isEnable_ == b.isEnable_ &&
        a.mediaVolume_ == b.mediaVolume_ &&
        a.isLowLatencyDevice_ == b.isLowLatencyDevice_ &&
        a.a2dpOffloadFlag_ == b.a2dpOffloadFlag_ &&
        a.descriptorType_ == b.descriptorType_ &&
        a.spatializationSupported_ == b.spatializationSupported_ &&
        a.hasPair_ == b.hasPair_ &&
        a.isVrSupported_ == b.isVrSupported_;
}

static bool IsAudioProcessConfigEqual(const AudioProcessConfig &a, const AudioProcessConfig &b)
{
    return a.appInfo.appUid == b.appInfo.appUid &&
        a.appInfo.appTokenId == b.appInfo.appTokenId &&
        a.appInfo.appPid == b.appInfo.appPid &&
        a.appInfo.appFullTokenId == b.appInfo.appFullTokenId &&
        a.streamInfo.samplingRate == b.streamInfo.samplingRate &&
        a.streamInfo.encoding == b.streamInfo.encoding &&
        a.streamInfo.format == b.streamInfo.format &&
        a.streamInfo.channels == b.streamInfo.channels &&
        a.streamInfo.channelLayout == b.streamInfo.channelLayout &&
        a.audioMode == b.audioMode &&
        a.rendererInfo.contentType == b.rendererInfo.contentType &&
        a.rendererInfo.streamUsage == b.rendererInfo.streamUsage &&
        a.rendererInfo.rendererFlags == b.rendererInfo.rendererFlags &&
        a.rendererInfo.originalFlag == b.rendererInfo.originalFlag &&
        a.rendererInfo.sceneType == b.rendererInfo.sceneType &&
        a.rendererInfo.spatializationEnabled == b.rendererInfo.spatializationEnabled &&
        a.rendererInfo.headTrackingEnabled == b.rendererInfo.headTrackingEnabled &&
        a.rendererInfo.isSatellite == b.rendererInfo.isSatellite &&
        a.rendererInfo.pipeType == b.rendererInfo.pipeType &&
        a.rendererInfo.playerType == b.rendererInfo.playerType &&
        a.rendererInfo.expectedPlaybackDurationBytes == b.rendererInfo.expectedPlaybackDurationBytes &&
        a.rendererInfo.effectMode == b.rendererInfo.effectMode &&
        a.rendererInfo.isLoopback == b.rendererInfo.isLoopback &&
        a.rendererInfo.loopbackMode == b.rendererInfo.loopbackMode &&
        a.rendererInfo.isVirtualKeyboard == b.rendererInfo.isVirtualKeyboard &&
        a.privacyType == b.privacyType &&
        a.capturerInfo.sourceType == b.capturerInfo.sourceType &&
        a.capturerInfo.capturerFlags == b.capturerInfo.capturerFlags &&
        a.capturerInfo.originalFlag == b.capturerInfo.originalFlag &&
        a.capturerInfo.pipeType == b.capturerInfo.pipeType &&
        a.capturerInfo.recorderType == b.capturerInfo.recorderType &&
        a.capturerInfo.isLoopback == b.capturerInfo.isLoopback &&
        a.capturerInfo.loopbackMode == b.capturerInfo.loopbackMode &&
        a.streamType == b.streamType &&
        a.deviceType == b.deviceType &&
        a.isInnerCapturer == b.isInnerCapturer &&
        a.isWakeupCapturer == b.isWakeupCapturer &&
        a.originalSessionId == b.originalSessionId &&
        a.innerCapId == b.innerCapId;
}

void InitAudioStreamInfo(AudioStreamInfo &info)
{
    info.samplingRate = SAMPLE_RATE_44100;
    info.encoding = ENCODING_EAC3;
    info.format = SAMPLE_F32LE;
    info.channels = STEREO;
    info.channelLayout = CH_LAYOUT_MONO;
}

void InitAudioRendererInfo(AudioRendererInfo &info)
{
    info.contentType = CONTENT_TYPE_SPEECH;
    info.streamUsage = STREAM_USAGE_SYSTEM;
    info.rendererFlags = TEST_VALUE_1;
    info.originalFlag = TEST_VALUE_2;
    info.sceneType = TEST_STRING_VALUE_1;
    info.spatializationEnabled = true;
    info.headTrackingEnabled = false;
    info.pipeType = PIPE_TYPE_DIRECT_OUT;
    info.samplingRate = SAMPLE_RATE_44100;
    info.encodingType = TEST_VALUE_1;
    info.channelLayout = TEST_VALUE_3;
    info.format = SAMPLE_S32LE;
    info.isOffloadAllowed = true;
    info.playerType = PLAYER_TYPE_OPENSL_ES;
    info.expectedPlaybackDurationBytes = TEST_VALUE_4;
    info.effectMode = TEST_VALUE_1;
    info.volumeMode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    info.isLoopback = true;
    info.loopbackMode = LOOPBACK_HARDWARE;
    info.isVirtualKeyboard = true;
    info.audioFlag = TEST_VALUE_3;
}

void InitAudioCapturerInfo(AudioCapturerInfo &info)
{
    info.sceneType = SOURCE_TYPE_WAKEUP;
    info.capturerFlags = TEST_VALUE_1;
    info.originalFlag = TEST_VALUE_2;
    info.pipeType = PIPE_TYPE_DIRECT_OUT;
    info.samplingRate = SAMPLE_RATE_44100;
    info.encodingType = TEST_VALUE_1;
    info.channelLayout = TEST_VALUE_2;
    info.sceneType = TEST_STRING_VALUE_1;
    info.recorderType = RECORDER_TYPE_OPENSL_ES;
    info.isLoopback = true;
    info.loopbackMode = LOOPBACK_HARDWARE;
}

/**
 * @tc.name  : Test AudioStreamDescriptor.
 * @tc.number: AudioStreamDescriptor_001
 * @tc.desc  : Test AudioStreamDescriptor Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioStreamDescriptor_001, TestSize.Level1)
{
    Parcel parcel;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(device, nullptr);
    AudioStreamDescriptor descriptor = {};
    descriptor.streamInfo_.samplingRate = SAMPLE_RATE_11025;
    descriptor.audioMode_ = AUDIO_MODE_RECORD;
    descriptor.audioFlag_ = AUDIO_OUTPUT_FLAG_HWDECODING;
    descriptor.routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    descriptor.startTimeStamp_ = TEST_VALUE_1;
    descriptor.rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    descriptor.capturerInfo_.sourceType = SOURCE_TYPE_REMOTE_CAST;
    descriptor.appInfo_ = { TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4 };
    descriptor.sessionId_ = TEST_VALUE_1;
    descriptor.callerUid_ = TEST_VALUE_2;
    descriptor.callerPid_ = TEST_VALUE_3;
    descriptor.streamAction_ = AUDIO_STREAM_ACTION_MOVE;
    descriptor.oldDeviceDescs_.push_back(device);
    descriptor.newDeviceDescs_.push_back(device);
    descriptor.newDeviceDescs_.push_back(device);

    EXPECT_TRUE(descriptor.Marshalling(parcel));
    auto newDescriptor = std::shared_ptr<AudioStreamDescriptor>(AudioStreamDescriptor::Unmarshalling(parcel));
    ASSERT_NE(newDescriptor, nullptr);

    EXPECT_EQ(newDescriptor->streamInfo_.samplingRate, descriptor.streamInfo_.samplingRate);
    EXPECT_EQ(newDescriptor->audioMode_, descriptor.audioMode_);
    EXPECT_EQ(newDescriptor->audioFlag_, descriptor.audioFlag_);
    EXPECT_EQ(newDescriptor->routeFlag_, descriptor.routeFlag_);
    EXPECT_EQ(newDescriptor->startTimeStamp_, descriptor.startTimeStamp_);
    EXPECT_EQ(newDescriptor->rendererInfo_.streamUsage, descriptor.rendererInfo_.streamUsage);
    EXPECT_EQ(newDescriptor->capturerInfo_.sourceType, descriptor.capturerInfo_.sourceType);
    EXPECT_TRUE(IsAppInfoEqual(newDescriptor->appInfo_, descriptor.appInfo_));
    EXPECT_EQ(newDescriptor->sessionId_, descriptor.sessionId_);
    EXPECT_EQ(newDescriptor->callerUid_, descriptor.callerUid_);
    EXPECT_EQ(newDescriptor->callerPid_, descriptor.callerPid_);
    EXPECT_EQ(newDescriptor->streamAction_, descriptor.streamAction_);
    EXPECT_EQ(newDescriptor->oldDeviceDescs_.size(), descriptor.oldDeviceDescs_.size());
    EXPECT_EQ(newDescriptor->newDeviceDescs_.size(), descriptor.newDeviceDescs_.size());
}

/**
 * @tc.name  : Test AudioZoneContext.
 * @tc.number: AudioZoneContext_001
 * @tc.desc  : Test AudioZoneContext Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioZoneContext_001, TestSize.Level1)
{
    Parcel parcel;
    AudioZoneContext context = {};
    context.focusStrategy_ = AudioZoneFocusStrategy::DISTRIBUTED_FOCUS_STRATEGY;

    EXPECT_TRUE(context.Marshalling(parcel));
    auto newContext = std::shared_ptr<AudioZoneContext>(AudioZoneContext::Unmarshalling(parcel));
    ASSERT_NE(newContext, nullptr);
    EXPECT_TRUE(newContext->focusStrategy_ == context.focusStrategy_);
}

/**
 * @tc.name  : Test AudioZoneDescriptor.
 * @tc.number: AudioZoneDescriptor_001
 * @tc.desc  : Test AudioZoneDescriptor Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioZoneDescriptor_001, TestSize.Level1)
{
    Parcel parcel;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(device, nullptr);
    AudioZoneDescriptor descriptor = {};
    descriptor.zoneId_ = TEST_VALUE_1;
    descriptor.name_ = TEST_STRING_VALUE_1;
    descriptor.uids_ = {TEST_VALUE_1, TEST_VALUE_2};
    descriptor.devices_.push_back(device);

    EXPECT_TRUE(descriptor.Marshalling(parcel));
    auto newDescriptor = std::shared_ptr<AudioZoneDescriptor>(AudioZoneDescriptor::Unmarshalling(parcel));
    ASSERT_NE(newDescriptor, nullptr);
    EXPECT_TRUE(newDescriptor->zoneId_ == descriptor.zoneId_);
    EXPECT_TRUE(newDescriptor->name_ == descriptor.name_);
    EXPECT_TRUE(newDescriptor->uids_ == descriptor.uids_);
    EXPECT_TRUE(newDescriptor->devices_.size() == descriptor.devices_.size());
}

/**
 * @tc.name  : Test AudioZoneStream.
 * @tc.number: AudioZoneStream_001
 * @tc.desc  : Test AudioZoneStream Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioZoneStream_001, TestSize.Level1)
{
    Parcel parcel;
    AudioZoneStream stream = {};
    stream.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    stream.sourceType = SOURCE_TYPE_CAMCORDER;
    stream.isPlay = true;

    EXPECT_TRUE(stream.Marshalling(parcel));
    auto newStream = std::shared_ptr<AudioZoneStream>(AudioZoneStream::Unmarshalling(parcel));
    ASSERT_NE(newStream, nullptr);
    EXPECT_TRUE((*newStream) == stream);
}

/**
 * @tc.name  : Test IAudioSinkAttr.
 * @tc.number: IAudioSinkAttr_001
 * @tc.desc  : Test IAudioSinkAttr Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, IAudioSinkAttr_001, TestSize.Level1)
{
    Parcel parcel;
    IAudioSinkAttr attr = {};
    attr.adapterName = TEST_STRING_VALUE_1;
    attr.openMicSpeaker = TEST_VALUE_1;
    attr.format = static_cast<AudioSampleFormat>(TEST_VALUE_2);
    attr.sampleRate = TEST_VALUE_3;
    attr.channel = TEST_VALUE_4;
    attr.volume = TEST_VALUE_1;
    attr.filePath = TEST_STRING_VALUE_1;
    attr.deviceNetworkId = TEST_STRING_VALUE_2;
    attr.deviceType = TEST_VALUE_2;
    attr.channelLayout = TEST_VALUE_3;
    attr.audioStreamFlag = TEST_VALUE_4;
    attr.address = TEST_STRING_VALUE_1;
    attr.aux = TEST_STRING_VALUE_2;


    EXPECT_TRUE(attr.Marshalling(parcel));
    auto newAttr = std::shared_ptr<IAudioSinkAttr>(IAudioSinkAttr::Unmarshalling(parcel));
    ASSERT_NE(newAttr, nullptr);
    EXPECT_TRUE(newAttr->adapterName == attr.adapterName);
    EXPECT_TRUE(newAttr->openMicSpeaker == attr.openMicSpeaker);
    EXPECT_TRUE(newAttr->format == attr.format);
    EXPECT_TRUE(newAttr->sampleRate == attr.sampleRate);
    EXPECT_TRUE(newAttr->channel == attr.channel);
    EXPECT_TRUE(newAttr->volume == attr.volume);
    EXPECT_TRUE(newAttr->filePath == attr.filePath);
    EXPECT_TRUE(newAttr->deviceNetworkId == attr.deviceNetworkId);
    EXPECT_TRUE(newAttr->deviceType == attr.deviceType);
    EXPECT_TRUE(newAttr->channelLayout == attr.channelLayout);
    EXPECT_TRUE(newAttr->audioStreamFlag == attr.audioStreamFlag);
    EXPECT_TRUE(newAttr->address == attr.address);
    EXPECT_TRUE(newAttr->aux == attr.aux);
}

/**
 * @tc.name  : Test IAudioSourceAttr.
 * @tc.number: IAudioSourceAttr_001
 * @tc.desc  : Test IAudioSourceAttr Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, IAudioSourceAttr_001, TestSize.Level1)
{
    Parcel parcel;
    IAudioSourceAttr attr = {};
    attr.adapterName = TEST_STRING_VALUE_1;
    attr.openMicSpeaker = TEST_VALUE_1;
    attr.format = static_cast<AudioSampleFormat>(TEST_VALUE_2);
    attr.sampleRate = TEST_VALUE_3;
    attr.channel = TEST_VALUE_4;
    attr.volume = TEST_VALUE_1;
    attr.bufferSize = TEST_VALUE_2;
    attr.isBigEndian = true;
    attr.filePath = TEST_STRING_VALUE_1;
    attr.deviceNetworkId = TEST_STRING_VALUE_2;
    attr.deviceType = TEST_VALUE_2;
    attr.sourceType = TEST_VALUE_1;
    attr.channelLayout = TEST_VALUE_3;
    attr.audioStreamFlag = TEST_VALUE_4;
    attr.hasEcConfig = false;
    attr.formatEc = static_cast<AudioSampleFormat>(TEST_VALUE_2);;
    attr.sampleRateEc = TEST_VALUE_4;
    attr.channelEc = TEST_VALUE_1;

    EXPECT_TRUE(attr.Marshalling(parcel));
    auto newAttr = std::shared_ptr<IAudioSourceAttr>(IAudioSourceAttr::Unmarshalling(parcel));
    ASSERT_NE(newAttr, nullptr);
    EXPECT_TRUE(newAttr->adapterName == attr.adapterName);
    EXPECT_TRUE(newAttr->openMicSpeaker == attr.openMicSpeaker);
    EXPECT_TRUE(newAttr->format == attr.format);
    EXPECT_TRUE(newAttr->sampleRate == attr.sampleRate);
    EXPECT_TRUE(newAttr->channel == attr.channel);
    EXPECT_TRUE(newAttr->volume == attr.volume);
    EXPECT_TRUE(newAttr->bufferSize == attr.bufferSize);
    EXPECT_TRUE(newAttr->isBigEndian == attr.isBigEndian);
    EXPECT_TRUE(newAttr->filePath == attr.filePath);
    EXPECT_TRUE(newAttr->deviceNetworkId == attr.deviceNetworkId);
    EXPECT_TRUE(newAttr->deviceType == attr.deviceType);
    EXPECT_TRUE(newAttr->sourceType == attr.sourceType);
    EXPECT_TRUE(newAttr->channelLayout == attr.channelLayout);
    EXPECT_TRUE(newAttr->audioStreamFlag == attr.audioStreamFlag);
    EXPECT_TRUE(newAttr->hasEcConfig == attr.hasEcConfig);
    EXPECT_TRUE(newAttr->formatEc == attr.formatEc);
    EXPECT_TRUE(newAttr->sampleRateEc == attr.sampleRateEc);
    EXPECT_TRUE(newAttr->channelEc == attr.channelEc);
}

/**
 * @tc.name  : Test DeviceStreamInfo.
 * @tc.number: DeviceStreamInfo_001
 * @tc.desc  : Test DeviceStreamInfo Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, DeviceStreamInfo_001, TestSize.Level1)
{
    Parcel parcel;
    DeviceStreamInfo info = {};
    info.encoding = AudioEncodingType::ENCODING_EAC3;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.channelLayout = { CH_LAYOUT_MONO, CH_LAYOUT_STEREO };
    info.samplingRate = { SAMPLE_RATE_44100, SAMPLE_RATE_64000 };

    EXPECT_TRUE(info.Marshalling(parcel));
    DeviceStreamInfo newInfo = {};
    newInfo.Unmarshalling(parcel);
    EXPECT_TRUE(info == newInfo);
}

/**
 * @tc.name  : Test AudioDeviceDescriptor.
 * @tc.number: AudioDeviceDescriptor_001
 * @tc.desc  : Test AudioDeviceDescriptor Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioDeviceDescriptor_001, TestSize.Level1)
{
    Parcel parcel;
    AudioDeviceDescriptor descriptor = {};
    descriptor.deviceType_ = DEVICE_TYPE_EARPIECE;
    descriptor.deviceRole_ = DEVICE_ROLE_MAX;
    descriptor.deviceId_ = TEST_VALUE_1;
    descriptor.channelMasks_ = TEST_VALUE_2;
    descriptor.channelIndexMasks_ = TEST_VALUE_3;
    descriptor.deviceName_ = TEST_STRING_VALUE_1;
    descriptor.macAddress_ = TEST_STRING_VALUE_2;
    descriptor.interruptGroupId_ = TEST_VALUE_1;
    descriptor.volumeGroupId_ = TEST_VALUE_2;
    descriptor.networkId_ = TEST_STRING_VALUE_1;
    descriptor.dmDeviceType_ = TEST_VALUE_1;
    descriptor.displayName_ = TEST_STRING_VALUE_2;
    DeviceStreamInfo info = {};
    descriptor.audioStreamInfo_.assign(TEST_VALUE_1, info);
    descriptor.deviceCategory_ = BT_HEARAID;
    descriptor.connectState_ = VIRTUAL_CONNECTED;
    // AudioDeviceDescriptor
    descriptor.exceptionFlag_ = false;
    descriptor.connectTimeStamp_ = TEST_VALUE_1;
    descriptor.isScoRealConnected_ = true;
    descriptor.isEnable_ = false;
    descriptor.mediaVolume_ = TEST_VALUE_2;
    descriptor.callVolume_ = TEST_VALUE_3;
    // DeviceInfo
    descriptor.isLowLatencyDevice_ = true;
    descriptor.a2dpOffloadFlag_ = TEST_VALUE_4;
    // Other
    descriptor.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
    descriptor.spatializationSupported_ = true;
    descriptor.hasPair_ = false;
    descriptor.routerType_ = ROUTER_TYPE_COCKPIT_PHONE;
    descriptor.isVrSupported_ = true;

    EXPECT_TRUE(descriptor.Marshalling(parcel));
    auto newDescriptor = std::shared_ptr<AudioDeviceDescriptor>(AudioDeviceDescriptor::Unmarshalling(parcel));
    ASSERT_NE(newDescriptor, nullptr);
    EXPECT_TRUE(IsAudioDeviceDescriptorEqual(*newDescriptor, descriptor));
}

/**
 * @tc.name  : Test AudioStreamDeviceChangeReasonExt.
 * @tc.number: AudioStreamDeviceChangeReasonExt_001
 * @tc.desc  : Test AudioStreamDeviceChangeReasonExt Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioStreamDeviceChangeReasonExt_001, TestSize.Level1)
{
    Parcel parcel;
    AudioStreamDeviceChangeReasonExt reason = {};
    reason.reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE_EXT;

    EXPECT_TRUE(reason.Marshalling(parcel));
    auto newReason = std::shared_ptr<AudioStreamDeviceChangeReasonExt>(
        AudioStreamDeviceChangeReasonExt::Unmarshalling(parcel));
    ASSERT_NE(newReason, nullptr);
    EXPECT_TRUE(newReason->reason_ == reason.reason_);
}

/**
 * @tc.name  : Test AudioSpatialDeviceState.
 * @tc.number: AudioSpatialDeviceState_001
 * @tc.desc  : Test AudioSpatialDeviceState Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioSpatialDeviceState_001, TestSize.Level1)
{
    Parcel parcel;
    AudioSpatialDeviceState deviceState = {};
    deviceState.address = TEST_STRING_VALUE_1;
    deviceState.isSpatializationSupported = true;
    deviceState.isHeadTrackingSupported = false;
    deviceState.spatialDeviceType = EARPHONE_TYPE_GLASSES;

    EXPECT_TRUE(deviceState.Marshalling(parcel));
    auto newDeviceState = std::shared_ptr<AudioSpatialDeviceState>(AudioSpatialDeviceState::Unmarshalling(parcel));
    ASSERT_NE(newDeviceState, nullptr);
    EXPECT_TRUE(newDeviceState->address == deviceState.address);
    EXPECT_TRUE(newDeviceState->isSpatializationSupported == deviceState.isSpatializationSupported);
    EXPECT_TRUE(newDeviceState->isHeadTrackingSupported == deviceState.isHeadTrackingSupported);
    EXPECT_TRUE(newDeviceState->spatialDeviceType == deviceState.spatialDeviceType);
}

/**
 * @tc.name  : Test Library.
 * @tc.number: Library_001
 * @tc.desc  : Test Library Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, Library_001, TestSize.Level1)
{
    Parcel parcel;
    Library library = {};
    library.name = TEST_STRING_VALUE_1;
    library.path = TEST_STRING_VALUE_2;

    EXPECT_TRUE(library.Marshalling(parcel));
    auto newLibrary = std::shared_ptr<Library>(Library::Unmarshalling(parcel));
    ASSERT_NE(newLibrary, nullptr);
    EXPECT_TRUE(newLibrary->name == library.name);
    EXPECT_TRUE(newLibrary->path == library.path);
}

/**
 * @tc.name  : Test Effect.
 * @tc.number: Effect_001
 * @tc.desc  : Test Effect Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, Effect_001, TestSize.Level1)
{
    Parcel parcel;
    Effect effect = {};
    effect.name = TEST_STRING_VALUE_1;
    effect.libraryName = TEST_STRING_VALUE_2;
    effect.effectProperty = { TEST_STRING_VALUE_3, TEST_STRING_VALUE_4 };

    EXPECT_TRUE(effect.Marshalling(parcel));
    auto newEffect = std::shared_ptr<Effect>(Effect::Unmarshalling(parcel));
    ASSERT_NE(newEffect, nullptr);
    EXPECT_TRUE(newEffect->name == effect.name);
    EXPECT_TRUE(newEffect->libraryName == effect.libraryName);
    EXPECT_TRUE(newEffect->effectProperty == effect.effectProperty);
}

/**
 * @tc.name  : Test Effect.
 * @tc.number: Effect_002
 * @tc.desc  : Test Effect Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, Effect_002, TestSize.Level1)
{
    Parcel parcel;
    Effect effect = {};
    effect.effectProperty.assign(Effect::MAX_EFFECT_PROPERTY_SIZE + 1, TEST_STRING_VALUE_1);
    EXPECT_TRUE(effect.Marshalling(parcel));
    auto newEffect = std::shared_ptr<Effect>(Effect::Unmarshalling(parcel));
    EXPECT_EQ(newEffect, nullptr);
}

/**
 * @tc.name  : Test EffectChain.
 * @tc.number: EffectChain_001
 * @tc.desc  : Test EffectChain Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, EffectChain_001, TestSize.Level1)
{
    Parcel parcel;
    EffectChain effectChain = {};
    effectChain.name = TEST_STRING_VALUE_1;
    effectChain.apply = { TEST_STRING_VALUE_2, TEST_STRING_VALUE_3 };
    effectChain.label = TEST_STRING_VALUE_4;

    EXPECT_TRUE(effectChain.Marshalling(parcel));
    auto newEffectChain = std::shared_ptr<EffectChain>(EffectChain::Unmarshalling(parcel));
    ASSERT_NE(newEffectChain, nullptr);
    EXPECT_TRUE(newEffectChain->name == effectChain.name);
    EXPECT_TRUE(newEffectChain->apply == effectChain.apply);
    EXPECT_TRUE(newEffectChain->label == effectChain.label);
}

/**
 * @tc.name  : Test EffectChainManagerParam.
 * @tc.number: EffectChainManagerParam_001
 * @tc.desc  : Test EffectChainManagerParam Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, EffectChainManagerParam_001, TestSize.Level1)
{
    Parcel parcel;
    EffectChainManagerParam param = {};
    param.maxExtraNum = TEST_VALUE_1;
    param.defaultSceneName = TEST_STRING_VALUE_1;
    param.priorSceneList = { TEST_STRING_VALUE_1, TEST_STRING_VALUE_2 };
    param.sceneTypeToChainNameMap = {
        {TEST_STRING_VALUE_3, TEST_STRING_VALUE_3},
        {TEST_STRING_VALUE_4, TEST_STRING_VALUE_4}};
    param.effectDefaultProperty = {
        {TEST_STRING_VALUE_1, TEST_STRING_VALUE_1},
        {TEST_STRING_VALUE_2, TEST_STRING_VALUE_2}};

    EXPECT_TRUE(param.Marshalling(parcel));
    auto newParam = std::shared_ptr<EffectChainManagerParam>(EffectChainManagerParam::Unmarshalling(parcel));
    ASSERT_NE(newParam, nullptr);
    EXPECT_TRUE(newParam->maxExtraNum == param.maxExtraNum);
    EXPECT_TRUE(newParam->defaultSceneName == param.defaultSceneName);
    EXPECT_TRUE(newParam->priorSceneList == param.priorSceneList);
    EXPECT_TRUE(newParam->sceneTypeToChainNameMap == param.sceneTypeToChainNameMap);
    EXPECT_TRUE(newParam->effectDefaultProperty == param.effectDefaultProperty);
}

/**
 * @tc.name  : Test EffectChainManagerParam.
 * @tc.number: EffectChainManagerParam_002
 * @tc.desc  : Test EffectChainManagerParam Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, EffectChainManagerParam_002, TestSize.Level1)
{
    Parcel parcel;
    EffectChainManagerParam param = {};
    param.priorSceneList.assign(AUDIO_EFFECT_PRIOR_SCENE_UPPER_LIMIT + 1, TEST_STRING_VALUE_1);
    EXPECT_TRUE(param.Marshalling(parcel));
    auto newParam = std::shared_ptr<EffectChainManagerParam>(EffectChainManagerParam::Unmarshalling(parcel));
    EXPECT_EQ(newParam, nullptr);
}

/**
 * @tc.name  : Test EffectChainManagerParam.
 * @tc.number: EffectChainManagerParam_003
 * @tc.desc  : Test EffectChainManagerParam Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, EffectChainManagerParam_003, TestSize.Level1)
{
    Parcel parcel;
    EffectChainManagerParam param = {};
    for (int i = 0; i <= AUDIO_EFFECT_CHAIN_CONFIG_UPPER_LIMIT; i++) {
        param.sceneTypeToChainNameMap[std::to_string(i)] = TEST_STRING_VALUE_1;
    }
    EXPECT_TRUE(param.Marshalling(parcel));
    auto newParam = std::shared_ptr<EffectChainManagerParam>(EffectChainManagerParam::Unmarshalling(parcel));
    EXPECT_EQ(newParam, nullptr);
}

/**
 * @tc.name  : Test EffectChainManagerParam.
 * @tc.number: EffectChainManagerParam_004
 * @tc.desc  : Test EffectChainManagerParam Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, EffectChainManagerParam_004, TestSize.Level1)
{
    Parcel parcel;
    EffectChainManagerParam param = {};
    for (int i = 0; i <= AUDIO_EFFECT_COUNT_PROPERTY_UPPER_LIMIT; i++) {
        param.effectDefaultProperty[std::to_string(i)] = TEST_STRING_VALUE_1;
    }
    EXPECT_TRUE(param.Marshalling(parcel));
    auto newParam = std::shared_ptr<EffectChainManagerParam>(EffectChainManagerParam::Unmarshalling(parcel));
    EXPECT_EQ(newParam, nullptr);
}

/**
 * @tc.name  : Test SupportedEffectConfig.
 * @tc.number: SupportedEffectConfig_001
 * @tc.desc  : Test SupportedEffectConfig Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, SupportedEffectConfig_001, TestSize.Level1)
{
    Parcel parcel;
    SupportedEffectConfig config = {};
    Device device1 = { TEST_STRING_VALUE_1, TEST_STRING_VALUE_2 };
    Device device2 = { TEST_STRING_VALUE_2, TEST_STRING_VALUE_3 };
    StreamEffectMode mode1 = { TEST_STRING_VALUE_3, { device1 } };
    StreamEffectMode mode2 = { TEST_STRING_VALUE_4, { device2 } };
    Stream stream1 = { PRIOR_SCENE, TEST_STRING_VALUE_1, { mode1 } };
    Stream stream2 = { NORMAL_SCENE, TEST_STRING_VALUE_2, { mode2 } };
    config.preProcessNew.stream = { stream1 };
    config.postProcessNew.stream = { stream2, stream2 };
    config.postProcessSceneMap = { { TEST_STRING_VALUE_1, TEST_STRING_VALUE_2 },
        { TEST_STRING_VALUE_2, TEST_STRING_VALUE_3 } };

    EXPECT_TRUE(config.Marshalling(parcel));
    auto newConfig = std::shared_ptr<SupportedEffectConfig>(SupportedEffectConfig::Unmarshalling(parcel));
    ASSERT_NE(newConfig, nullptr);
    EXPECT_TRUE(IsProcessNewEqual(newConfig->preProcessNew, config.preProcessNew));
    EXPECT_TRUE(IsProcessNewEqual(newConfig->postProcessNew, config.postProcessNew));
    EXPECT_TRUE(IsSceneMappingItemArrayEqual(newConfig->postProcessSceneMap, config.postProcessSceneMap));
}

/**
 * @tc.name  : Test SupportedEffectConfig.
 * @tc.number: SupportedEffectConfig_002
 * @tc.desc  : Test SupportedEffectConfig Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, SupportedEffectConfig_002, TestSize.Level1)
{
    Parcel parcel;
    SupportedEffectConfig config = {};
    Stream stream = {};
    config.preProcessNew.stream.assign(AUDIO_EFFECT_COUNT_UPPER_LIMIT + 1, stream);
    EXPECT_TRUE(config.Marshalling(parcel));
    auto newConfig = std::shared_ptr<SupportedEffectConfig>(SupportedEffectConfig::Unmarshalling(parcel));
    EXPECT_EQ(newConfig, nullptr);
}

/**
 * @tc.name  : Test SupportedEffectConfig.
 * @tc.number: SupportedEffectConfig_003
 * @tc.desc  : Test SupportedEffectConfig Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, SupportedEffectConfig_003, TestSize.Level1)
{
    Parcel parcel;
    SupportedEffectConfig config = {};
    Stream stream = {};
    config.postProcessNew.stream.assign(AUDIO_EFFECT_COUNT_UPPER_LIMIT + 1, stream);
    EXPECT_TRUE(config.Marshalling(parcel));
    auto newConfig = std::shared_ptr<SupportedEffectConfig>(SupportedEffectConfig::Unmarshalling(parcel));
    EXPECT_EQ(newConfig, nullptr);
}

/**
 * @tc.name  : Test SupportedEffectConfig.
 * @tc.number: SupportedEffectConfig_004
 * @tc.desc  : Test SupportedEffectConfig Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, SupportedEffectConfig_004, TestSize.Level1)
{
    Parcel parcel;
    SupportedEffectConfig config = {};
    SceneMappingItem item = {};
    config.postProcessSceneMap.assign(SupportedEffectConfig::POST_PROCESS_SCENE_MAP_MAX_SIZE + 1, item);
    EXPECT_TRUE(config.Marshalling(parcel));
    auto newConfig = std::shared_ptr<SupportedEffectConfig>(SupportedEffectConfig::Unmarshalling(parcel));
    EXPECT_EQ(newConfig, nullptr);
}

/**
 * @tc.name  : Test AudioEffectPropertyArrayV3.
 * @tc.number: AudioEffectPropertyArrayV3_001
 * @tc.desc  : Test AudioEffectPropertyArrayV3 Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioEffectPropertyArrayV3_001, TestSize.Level1)
{
    Parcel parcel;
    AudioEffectPropertyArrayV3 propertyArray = {};
    AudioEffectPropertyV3 property = { TEST_STRING_VALUE_1, TEST_STRING_VALUE_2, CAPTURE_EFFECT_FLAG };
    propertyArray.property.push_back(property);

    EXPECT_TRUE(propertyArray.Marshalling(parcel));
    auto newPropertyArray = std::shared_ptr<AudioEffectPropertyArrayV3>(
        AudioEffectPropertyArrayV3::Unmarshalling(parcel));
    ASSERT_NE(newPropertyArray, nullptr);
    EXPECT_EQ(newPropertyArray->property.size(), propertyArray.property.size());
    if (newPropertyArray->property.size() == propertyArray.property.size()) {
        for (int32_t i = 0; i < propertyArray.property.size(); i++) {
            EXPECT_EQ(newPropertyArray->property[i], propertyArray.property[i]);
        }
    }
}

/**
 * @tc.name  : Test AudioEffectPropertyArrayV3.
 * @tc.number: AudioEffectPropertyArrayV3_002
 * @tc.desc  : Test AudioEffectPropertyArrayV3 Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioEffectPropertyArrayV3_002, TestSize.Level1)
{
    Parcel parcel;
    AudioEffectPropertyArrayV3 propertyArray = {};
    AudioEffectPropertyV3 property = {};

    propertyArray.property.assign(AUDIO_EFFECT_COUNT_UPPER_LIMIT + 1, property);
    EXPECT_TRUE(propertyArray.Marshalling(parcel));
    auto newPropertyArray = std::shared_ptr<AudioEffectPropertyArrayV3>(
        AudioEffectPropertyArrayV3::Unmarshalling(parcel));
    EXPECT_EQ(newPropertyArray, nullptr);
}

/**
 * @tc.name  : Test AudioEnhancePropertyArray.
 * @tc.number: AudioEnhancePropertyArray_001
 * @tc.desc  : Test AudioEnhancePropertyArray Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioEnhancePropertyArray_001, TestSize.Level1)
{
    Parcel parcel;
    AudioEnhancePropertyArray propertyArray = {};
    AudioEnhanceProperty property = { TEST_STRING_VALUE_1, TEST_STRING_VALUE_2 };
    propertyArray.property.push_back(property);

    EXPECT_TRUE(propertyArray.Marshalling(parcel));
    auto newPropertyArray = std::shared_ptr<AudioEnhancePropertyArray>(
        AudioEnhancePropertyArray::Unmarshalling(parcel));
    ASSERT_NE(newPropertyArray, nullptr);
    EXPECT_EQ(newPropertyArray->property.size(), propertyArray.property.size());
    if (newPropertyArray->property.size() == propertyArray.property.size()) {
        for (int32_t i = 0; i < propertyArray.property.size(); i++) {
            EXPECT_EQ(newPropertyArray->property[i], propertyArray.property[i]);
        }
    }
}

/**
 * @tc.name  : Test AudioEnhancePropertyArray.
 * @tc.number: AudioEnhancePropertyArray_002
 * @tc.desc  : Test AudioEnhancePropertyArray Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioEnhancePropertyArray_002, TestSize.Level1)
{
    Parcel parcel;
    AudioEnhancePropertyArray propertyArray = {};
    AudioEnhanceProperty property = {};

    propertyArray.property.assign(AUDIO_EFFECT_COUNT_UPPER_LIMIT + 1, property);
    EXPECT_TRUE(propertyArray.Marshalling(parcel));
    auto newPropertyArray = std::shared_ptr<AudioEnhancePropertyArray>(
        AudioEnhancePropertyArray::Unmarshalling(parcel));
    EXPECT_EQ(newPropertyArray, nullptr);
}

/**
 * @tc.name  : Test AudioEffectPropertyArray.
 * @tc.number: AudioEffectPropertyArray_001
 * @tc.desc  : Test AudioEffectPropertyArray Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioEffectPropertyArray_001, TestSize.Level1)
{
    Parcel parcel;
    AudioEffectPropertyArray propertyArray = {};
    AudioEffectProperty property = { TEST_STRING_VALUE_1, TEST_STRING_VALUE_2 };
    propertyArray.property.push_back(property);

    EXPECT_TRUE(propertyArray.Marshalling(parcel));
    auto newPropertyArray = std::shared_ptr<AudioEffectPropertyArray>(
        AudioEffectPropertyArray::Unmarshalling(parcel));
    ASSERT_NE(newPropertyArray, nullptr);
    EXPECT_EQ(newPropertyArray->property.size(), propertyArray.property.size());
    if (newPropertyArray->property.size() == propertyArray.property.size()) {
        for (int32_t i = 0; i < propertyArray.property.size(); i++) {
            EXPECT_EQ(newPropertyArray->property[i], propertyArray.property[i]);
        }
    }
}

/**
 * @tc.name  : Test AudioEffectPropertyArray.
 * @tc.number: AudioEffectPropertyArray_002
 * @tc.desc  : Test AudioEffectPropertyArray Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioEffectPropertyArray_002, TestSize.Level1)
{
    Parcel parcel;
    AudioEffectPropertyArray propertyArray = {};
    AudioEffectProperty property = {};

    propertyArray.property.assign(AUDIO_EFFECT_COUNT_UPPER_LIMIT + 1, property);
    EXPECT_TRUE(propertyArray.Marshalling(parcel));
    auto newPropertyArray = std::shared_ptr<AudioEffectPropertyArray>(
        AudioEffectPropertyArray::Unmarshalling(parcel));
    EXPECT_EQ(newPropertyArray, nullptr);
}

/**
 * @tc.name  : Test AudioSpatializationState.
 * @tc.number: AudioSpatializationState_001
 * @tc.desc  : Test AudioSpatializationState Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioSpatializationState_001, TestSize.Level1)
{
    Parcel parcel;
    AudioSpatializationState state = { true, false };

    EXPECT_TRUE(state.Marshalling(parcel));
    auto newState = std::shared_ptr<AudioSpatializationState>(AudioSpatializationState::Unmarshalling(parcel));
    ASSERT_NE(newState, nullptr);
    EXPECT_EQ(newState->spatializationEnabled, state.spatializationEnabled);
    EXPECT_EQ(newState->headTrackingEnabled, state.headTrackingEnabled);
}

/**
 * @tc.name  : Test ConverterConfig.
 * @tc.number: ConverterConfig_001
 * @tc.desc  : Test ConverterConfig Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, ConverterConfig_001, TestSize.Level1)
{
    Parcel parcel;
    ConverterConfig config = {};
    config.version = TEST_STRING_VALUE_1;
    config.library.name = TEST_STRING_VALUE_2;
    config.library.path = TEST_STRING_VALUE_3;
    config.outChannelLayout = TEST_VALUE_1;

    EXPECT_TRUE(config.Marshalling(parcel));
    auto newConfig = std::shared_ptr<ConverterConfig>(ConverterConfig::Unmarshalling(parcel));
    ASSERT_NE(newConfig, nullptr);
    EXPECT_EQ(newConfig->version, config.version);
    EXPECT_EQ(newConfig->library.name, config.library.name);
    EXPECT_EQ(newConfig->library.path, config.library.path);
    EXPECT_EQ(newConfig->outChannelLayout, config.outChannelLayout);
}

/**
 * @tc.name  : Test ToneSegment.
 * @tc.number: ToneSegment_001
 * @tc.desc  : Test ToneSegment Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, ToneSegment_001, TestSize.Level1)
{
    Parcel parcel;
    ToneSegment toneSegment = {};
    toneSegment.duration = TEST_VALUE_1;
    for (int32_t i = 0; i < TONEINFO_MAX_WAVES + 1; i++) {
        toneSegment.waveFreq[i] = TEST_VALUE_2 + i;
    }
    toneSegment.loopCnt = TEST_VALUE_2;
    toneSegment.loopIndx = TEST_VALUE_3;

    EXPECT_TRUE(toneSegment.Marshalling(parcel));
    auto newToneSegment = std::shared_ptr<ToneSegment>(ToneSegment::Unmarshalling(parcel));
    ASSERT_NE(newToneSegment, nullptr);
    EXPECT_EQ(newToneSegment->duration, toneSegment.duration);
    EXPECT_EQ(newToneSegment->loopCnt, toneSegment.loopCnt);
    EXPECT_EQ(newToneSegment->loopIndx, toneSegment.loopIndx);
    for (int32_t i = 0; i < TONEINFO_MAX_WAVES + 1; i++) {
        EXPECT_EQ(newToneSegment->waveFreq[i], toneSegment.waveFreq[i]);
    }
}

/**
 * @tc.name  : Test ToneInfo.
 * @tc.number: ToneInfo_001
 * @tc.desc  : Test ToneInfo Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, ToneInfo_001, TestSize.Level1)
{
    Parcel parcel;
    ToneInfo toneInfo = {};
    toneInfo.segmentCnt = TONEINFO_MAX_SEGMENTS;
    toneInfo.repeatCnt = TEST_VALUE_1;
    toneInfo.repeatSegment = TEST_VALUE_2;
    toneInfo.segments[0].duration = TEST_VALUE_3;
    toneInfo.segments[0].loopCnt = TEST_VALUE_4;

    EXPECT_TRUE(toneInfo.Marshalling(parcel));
    auto newToneInfo = std::shared_ptr<ToneInfo>(ToneInfo::Unmarshalling(parcel));
    ASSERT_NE(newToneInfo, nullptr);
    EXPECT_EQ(newToneInfo->segmentCnt, toneInfo.segmentCnt);
    EXPECT_EQ(newToneInfo->repeatCnt, toneInfo.repeatCnt);
    EXPECT_EQ(newToneInfo->repeatSegment, toneInfo.repeatSegment);
    EXPECT_EQ(newToneInfo->segments[0].duration, toneInfo.segments[0].duration);
    EXPECT_EQ(newToneInfo->segments[0].loopCnt, toneInfo.segments[0].loopCnt);
}

/**
 * @tc.name  : Test VolumeEvent.
 * @tc.number: VolumeEvent_001
 * @tc.desc  : Test VolumeEvent Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, VolumeEvent_001, TestSize.Level1)
{
    Parcel parcel;
    VolumeEvent event = {};
    event.volumeType = STREAM_RING;
    event.volume = TEST_VALUE_1;
    event.updateUi = true;
    event.volumeGroupId = TEST_VALUE_2;
    event.networkId = TEST_STRING_VALUE_1;
    event.volumeMode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    event.notifyRssWhenAccountsChange = true;

    EXPECT_TRUE(event.Marshalling(parcel));
    auto newEvent = std::shared_ptr<VolumeEvent>(VolumeEvent::Unmarshalling(parcel));
    ASSERT_NE(newEvent, nullptr);
    EXPECT_EQ(newEvent->volumeType, event.volumeType);
    EXPECT_EQ(newEvent->volume, event.volume);
    EXPECT_EQ(newEvent->updateUi, event.updateUi);
    EXPECT_EQ(newEvent->volumeGroupId, event.volumeGroupId);
    EXPECT_EQ(newEvent->networkId, event.networkId);
    EXPECT_EQ(newEvent->volumeMode, event.volumeMode);
    EXPECT_EQ(newEvent->notifyRssWhenAccountsChange, event.notifyRssWhenAccountsChange);
}

/**
 * @tc.name  : Test StreamVolumeEvent.
 * @tc.number: StreamVolumeEvent_001
 * @tc.desc  : Test StreamVolumeEvent Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, StreamVolumeEvent_001, TestSize.Level1)
{
    Parcel parcel;
    StreamVolumeEvent event = {};
    event.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    event.volume = TEST_VALUE_1;
    event.updateUi = true;
    event.volumeGroupId = TEST_VALUE_2;
    event.networkId = TEST_STRING_VALUE_1;
    event.volumeMode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;

    EXPECT_TRUE(event.Marshalling(parcel));
    auto newEvent = std::shared_ptr<StreamVolumeEvent>(StreamVolumeEvent::Unmarshalling(parcel));
    ASSERT_NE(newEvent, nullptr);
    EXPECT_EQ(newEvent->streamUsage, event.streamUsage);
    EXPECT_EQ(newEvent->volume, event.volume);
    EXPECT_EQ(newEvent->updateUi, event.updateUi);
    EXPECT_EQ(newEvent->volumeGroupId, event.volumeGroupId);
    EXPECT_EQ(newEvent->networkId, event.networkId);
    EXPECT_EQ(newEvent->volumeMode, event.volumeMode);
}

/**
 * @tc.name  : Test AudioWorkgroupChangeInfoIpc.
 * @tc.number: AudioWorkgroupChangeInfoIpc_001
 * @tc.desc  : Test AudioWorkgroupChangeInfoIpc Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioWorkgroupChangeInfoIpc_001, TestSize.Level1)
{
    Parcel parcel;
    AudioWorkgroupChangeInfoIpc info = {};
    info.changeInfo = { TEST_VALUE_1, TEST_VALUE_2, true };

    EXPECT_TRUE(info.Marshalling(parcel));
    auto newInfo = std::shared_ptr<AudioWorkgroupChangeInfoIpc>(AudioWorkgroupChangeInfoIpc::Unmarshalling(parcel));
    ASSERT_NE(newInfo, nullptr);
    EXPECT_EQ(newInfo->changeInfo.pid, info.changeInfo.pid);
    EXPECT_EQ(newInfo->changeInfo.groupId, info.changeInfo.groupId);
    EXPECT_EQ(newInfo->changeInfo.startAllowed, info.changeInfo.startAllowed);
}

/**
 * @tc.name  : Test CurrentOutputDeviceChangedEvent.
 * @tc.number: CurrentOutputDeviceChangedEvent_001
 * @tc.desc  : Test CurrentOutputDeviceChangedEvent Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, CurrentOutputDeviceChangedEvent_001, TestSize.Level1)
{
    Parcel parcel;
    CurrentOutputDeviceChangedEvent event = {};
    auto device = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(device, nullptr);

    event.devices.assign(CurrentOutputDeviceChangedEvent::DEVICE_CHANGE_VALID_SIZE + 1, device);
    EXPECT_TRUE(event.Marshalling(parcel));
    auto newEvent = std::shared_ptr<CurrentOutputDeviceChangedEvent>(
        CurrentOutputDeviceChangedEvent::Unmarshalling(parcel));
    EXPECT_EQ(newEvent, nullptr);
}

/**
 * @tc.name  : Test MicrophoneBlockedInfo.
 * @tc.number: MicrophoneBlockedInfo_001
 * @tc.desc  : Test MicrophoneBlockedInfo Deserialization.
 */
HWTEST(AudioIpcSerializationUnitTest, MicrophoneBlockedInfo_001, TestSize.Level1)
{
    Parcel parcel;
    MicrophoneBlockedInfo info = {};
    auto device = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(device, nullptr);

    info.devices.assign(MicrophoneBlockedInfo::DEVICE_CHANGE_VALID_SIZE + 1, device);
    EXPECT_TRUE(info.Marshalling(parcel));
    auto newInfo = std::shared_ptr<MicrophoneBlockedInfo>(MicrophoneBlockedInfo::Unmarshalling(parcel));
    EXPECT_EQ(newInfo, nullptr);
}

/**
 * @tc.name  : Test OHAudioBuffer.
 * @tc.number: OHAudioBuffer_001
 * @tc.desc  : Test OHAudioBuffer Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, OHAudioBuffer_001, TestSize.Level1)
{
    Parcel parcel;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t spanSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t byteSizePerFrame = BYTE_SIZE_PER_FRAME;
    auto audioBuffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(audioBuffer, nullptr);

    EXPECT_TRUE(audioBuffer->Marshalling(parcel));
    auto newAudioBuffer = std::shared_ptr<OHAudioBuffer>(OHAudioBuffer::Unmarshalling(parcel));
    ASSERT_NE(newAudioBuffer, nullptr);
    auto initInfo = newAudioBuffer->ohAudioBufferBase_.GetInitializationInfo();
    EXPECT_EQ(initInfo.totalSizeInFrame, totalSizeInFrame);
    EXPECT_EQ(initInfo.byteSizePerFrame, byteSizePerFrame);
    EXPECT_EQ(newAudioBuffer->spanBasicInfo_.spanSizeInFrame_, spanSizeInFrame);
}

/**
 * @tc.name  : Test OHAudioBufferBase.
 * @tc.number: OHAudioBufferBase_001
 * @tc.desc  : Test OHAudioBufferBase Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, OHAudioBufferBase_001, TestSize.Level1)
{
    Parcel parcel;
    uint32_t totalSizeInFrame = TOTAL_SIZE_IN_FRAME;
    uint32_t byteSizePerFrame = BYTE_SIZE_PER_FRAME;
    auto audioBuffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(audioBuffer, nullptr);

    EXPECT_TRUE(audioBuffer->Marshalling(parcel));
    auto newAudioBuffer = std::shared_ptr<OHAudioBufferBase>(OHAudioBufferBase::Unmarshalling(parcel));
    ASSERT_NE(newAudioBuffer, nullptr);
    EXPECT_EQ(newAudioBuffer->totalSizeInFrame_, totalSizeInFrame);
    EXPECT_EQ(newAudioBuffer->byteSizePerFrame_, byteSizePerFrame);
}

/**
 * @tc.name  : Test AudioSharedMemory.
 * @tc.number: AudioSharedMemory_001
 * @tc.desc  : Test AudioSharedMemory Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioSharedMemory_001, TestSize.Level1)
{
    Parcel parcel;
    size_t mapSize = 100;
    std::string name = "test";
    auto sharedMemory = AudioSharedMemory::CreateFormLocal(mapSize, name);
    ASSERT_NE(sharedMemory, nullptr);

    EXPECT_TRUE(sharedMemory->Marshalling(parcel));
    auto newSharedMemory = std::shared_ptr<AudioSharedMemory>(AudioSharedMemory::Unmarshalling(parcel));
    ASSERT_NE(newSharedMemory, nullptr);
    EXPECT_EQ(newSharedMemory->GetSize(), mapSize);
    EXPECT_EQ(newSharedMemory->GetName(), name);
}

/**
 * @tc.name  : Test AudioProcessConfig.
 * @tc.number: AudioProcessConfig_001
 * @tc.desc  : Test AudioProcessConfig Serialization.
 */
HWTEST(AudioIpcSerializationUnitTest, AudioProcessConfig_001, TestSize.Level1)
{
    Parcel parcel;
    AudioProcessConfig config = {};
    config.appInfo = { TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4 };
    InitAudioStreamInfo(config.streamInfo);
    InitAudioRendererInfo(config.rendererInfo);
    InitAudioCapturerInfo(config.capturerInfo);
    config.audioMode = AUDIO_MODE_RECORD;
    config.privacyType = PRIVACY_TYPE_PRIVATE;
    config.streamType = STREAM_ALARM;
    config.deviceType = DEVICE_TYPE_SPEAKER;
    config.isInnerCapturer = true;
    config.isWakeupCapturer = false;
    config.originalSessionId = TEST_VALUE_3;
    config.innerCapId = TEST_VALUE_4;

    EXPECT_TRUE(config.Marshalling(parcel));
    auto newConfig = std::shared_ptr<AudioProcessConfig>(AudioProcessConfig::Unmarshalling(parcel));
    ASSERT_NE(newConfig, nullptr);
    EXPECT_TRUE(IsAudioProcessConfigEqual(*newConfig, config));
}
} // namespace AudioStandard
} // namespace OHOS
 