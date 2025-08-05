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

#include <iostream>
#include "gtest/gtest.h"
#include "audio_errors.h"
#include "audio_offload_stream_unit_test.h"
#include "audio_spatialization_service.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void AudioOffloadStreamTest::SetUpTestCase(void) {}
void AudioOffloadStreamTest::TearDownTestCase(void) {}
void AudioOffloadStreamTest::SetUp(void)
{
    audioOffloadStream_ = &AudioOffloadStream::GetInstance();
}
void AudioOffloadStreamTest::TearDown(void)
{
    if (audioOffloadStream_ != nullptr) {
        audioOffloadStream_ = nullptr;
    }
}

/**
 * @tc.name  : HandlePowerStateChanged_NoChange_Test
 * @tc.number: AudioOffloadStreamTest_001
 * @tc.desc  : Test scenario where the power state does not change.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_NoChange_Test, TestSize.Level2)
{
    audioOffloadStream_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::FREEZE;
    audioOffloadStream_->HandlePowerStateChanged(state);
    EXPECT_EQ(audioOffloadStream_->currentPowerState_, PowerMgr::PowerState::FREEZE);
}

/**
 * @tc.name  : HandlePowerStateChanged_ActiveToInactive_Test
 * @tc.number: AudioOffloadStreamTest_002
 * @tc.desc  : Test scenario where the power state changes from ACTIVE to INACTIVE.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_ActiveToInactive_Test, TestSize.Level2)
{
    audioOffloadStream_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::INACTIVE;
    audioOffloadStream_->HandlePowerStateChanged(state);
    EXPECT_EQ(audioOffloadStream_->currentPowerState_, PowerMgr::PowerState::INACTIVE);
}

/**
 * @tc.name  : HandlePowerStateChanged_NoOffloadSupport_Test
 * @tc.number: AudioOffloadStreamTest_003
 * @tc.desc  : Test scenario where the device does not support offload.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_NoOffloadSupport_Test, TestSize.Level3)
{
    audioOffloadStream_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::FREEZE;
    audioOffloadStream_->HandlePowerStateChanged(state);
    EXPECT_EQ(audioOffloadStream_->currentPowerState_, PowerMgr::PowerState::FREEZE);
}

/**
 * @tc.name  : HandlePowerStateChanged_OffloadSessionIDNotSet_Test
 * @tc.number: audioOffloadStreamTest_004
 * @tc.desc  : Test scenario where the offload session ID is not set.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_OffloadSessionIDNotSet_Test, TestSize.Level3)
{
    audioOffloadStream_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::FREEZE;
    audioOffloadStream_->offloadSessionID_ = std::nullopt;
    audioOffloadStream_->HandlePowerStateChanged(state);
    EXPECT_EQ(audioOffloadStream_->currentPowerState_, PowerMgr::PowerState::FREEZE);
}

/**
 * @tc.name  : HandlePowerStateChanged_OffloadSessionIDSet_Test
 * @tc.number: audioOffloadStreamTest_005
 * @tc.desc  : Test scenario where the offload session ID is set.
 */
HWTEST_F(AudioOffloadStreamTest, HandlePowerStateChanged_OffloadSessionIDSet_Test, TestSize.Level3)
{
    audioOffloadStream_->currentPowerState_ = PowerMgr::PowerState::FREEZE;
    PowerMgr::PowerState state = PowerMgr::PowerState::FREEZE;
    audioOffloadStream_->offloadSessionID_ = 1;
    audioOffloadStream_->HandlePowerStateChanged(state);
    EXPECT_EQ(audioOffloadStream_->currentPowerState_, PowerMgr::PowerState::FREEZE);

    bool oldState = audioOffloadStream_->isOffloadAvailable_;
    audioOffloadStream_->isOffloadAvailable_ = false;
    audioOffloadStream_->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioOffloadStream_->HandlePowerStateChanged(PowerMgr::PowerState::INACTIVE);
    audioOffloadStream_->isOffloadAvailable_ = oldState;
    EXPECT_FALSE(audioOffloadStream_->GetOffloadAvailableFromXml());
}

/**
 * @tc.name  : CheckStreamOffloadMode_001
 * @tc.number: Audio_OffloadStream_006
 * @tc.desc  : Test CheckStreamOffloadMode when offload not available from xml.
 */
HWTEST_F(AudioOffloadStreamTest, CheckStreamOffloadMode_001, TestSize.Level3)
{
    // 模拟GetOffloadAvailableFromXml返回false
    audioOffloadStream_->isOffloadAvailable_ = false;
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int64_t activateSessionId = 1;

    bool result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckStreamOffloadMode_002
 * @tc.number: Audio_OffloadStream_007
 * @tc.desc  : Test CheckStreamOffloadMode when offload not available on current output device.
 */
HWTEST_F(AudioOffloadStreamTest, CheckStreamOffloadMode_002, TestSize.Level3)
{
    // 模拟GetOffloadAvailableFromXml返回true
    audioOffloadStream_->isOffloadAvailable_ = true;

    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int64_t activateSessionId = 1;
    bool result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckStreamOffloadMode_003
 * @tc.number: Audio_OffloadStream_008
 * @tc.desc  : Test CheckStreamOffloadMode when offload not available on current output device.
 */
HWTEST_F(AudioOffloadStreamTest, CheckStreamOffloadMode_003, TestSize.Level3)
{
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    int64_t activateSessionId = 6;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = 1;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 1;
    streamChangeInfo.audioRendererChangeInfo.channelCount = 3;
    streamChangeInfo.audioRendererChangeInfo.createrUID = 2;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RendererState::RENDERER_NEW;
    streamChangeInfo.audioRendererChangeInfo.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    audioOffloadStream_->streamCollector_.AddRendererStream(streamChangeInfo);

    audioOffloadStream_->isOffloadAvailable_ = true;
    audioOffloadStream_->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    bool result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);

    streamType = AudioStreamType::STREAM_MUSIC;
    result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);

    streamChangeInfo.audioRendererChangeInfo.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    audioOffloadStream_->streamCollector_.audioRendererChangeInfos_.clear();
    audioOffloadStream_->streamCollector_.AddRendererStream(streamChangeInfo);
    result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);

    streamChangeInfo.audioRendererChangeInfo.channelCount = 2;
    audioOffloadStream_->streamCollector_.audioRendererChangeInfos_.clear();
    audioOffloadStream_->streamCollector_.AddRendererStream(streamChangeInfo);
    result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);

    activateSessionId = 1;
    result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);

    streamChangeInfo.audioRendererChangeInfo.sessionId = 1041;
    audioOffloadStream_->streamCollector_.audioRendererChangeInfos_.clear();
    audioOffloadStream_->streamCollector_.AddRendererStream(streamChangeInfo);
    result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_FALSE(result);

    activateSessionId = 1041;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 1041;
    audioOffloadStream_->streamCollector_.audioRendererChangeInfos_.clear();
    audioOffloadStream_->streamCollector_.AddRendererStream(streamChangeInfo);
    result = audioOffloadStream_->CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_EQ(audioOffloadStream_->streamCollector_.GetUid(activateSessionId), 2);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckStreamOffloadMode_004
 * @tc.number: Audio_OffloadStream_008
 * @tc.desc  : Test CheckStreamOffloadMode with different offloadUID.
 */
HWTEST_F(AudioOffloadStreamTest, CheckStreamOffloadMode_004, TestSize.Level3)
{
    AudioOffloadStream audioOffloadStream;
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    int64_t activateSessionId = 10;
    AudioStreamChangeInfo streamChangeInfo{};
    auto& audioRendererChangeInfo = streamChangeInfo.audioRendererChangeInfo;
    audioRendererChangeInfo = {};
    audioRendererChangeInfo.clientUID = 1;
    audioRendererChangeInfo.sessionId = activateSessionId;
    audioRendererChangeInfo.channelCount = AudioChannel::STEREO;
    audioRendererChangeInfo.createrUID = 2;
    audioRendererChangeInfo.rendererState = RendererState::RENDERER_NEW;
    audioRendererChangeInfo.rendererInfo.pipeType = PIPE_TYPE_DIRECT_VOIP;
    audioOffloadStream.isOffloadAvailable_ = true;

    audioOffloadStream.streamCollector_.AddRendererStream(streamChangeInfo);
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.macAddress_ = "00:11:22:33:44:55";
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;

    bool result = audioOffloadStream.CheckStreamOffloadMode(activateSessionId, streamType);
    EXPECT_TRUE(result);

    audioOffloadStream.streamCollector_.audioRendererChangeInfos_.back()->createrUID = -1;
    audioOffloadStream.CheckStreamOffloadMode(activateSessionId, streamType);
    const int32_t UID_AUDIO = 1041;
    audioOffloadStream.streamCollector_.audioRendererChangeInfos_.back()->createrUID = UID_AUDIO;
    result = audioOffloadStream.CheckStreamOffloadMode(activateSessionId, streamType);

    audioOffloadStream.SetOffloadStatus(activateSessionId);
    EXPECT_EQ(audioOffloadStream.offloadSessionID_, activateSessionId);
    audioOffloadStream.SetOffloadStatus(activateSessionId);
    audioOffloadStream.SetOffloadStatus(activateSessionId + 1);
    EXPECT_EQ(audioOffloadStream.offloadSessionID_, activateSessionId);
}

/**
 * @tc.name  : MoveToNewPipe_ShouldReturnError_WhenStreamIsIllegal
 * @tc.number: AudioOffloadStreamTest_008
 * @tc.desc  : Test if MoveToNewPipe .
 */
HWTEST_F(AudioOffloadStreamTest, MoveToNewPipe_ShouldReturnError_WhenStreamIsIllegal, TestSize.Level3)
{
    AudioStreamChangeInfo streamChangeInfo;
    auto& audioRendererChangeInfo = streamChangeInfo.audioRendererChangeInfo;
    audioRendererChangeInfo = {};
    audioRendererChangeInfo.clientUID = 1;
    audioRendererChangeInfo.sessionId = 1;
    audioRendererChangeInfo.rendererState = RendererState::RENDERER_NEW;
    int32_t ret = audioOffloadStream_->streamCollector_.AddRendererStream(streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
    uint32_t sessionId = 1;
    AudioPipeType pipeType = AudioPipeType::PIPE_TYPE_NORMAL_OUT;
    // Arrange
    int32_t defaultUid = -1;
    // Act
    auto oldDeviceType = audioOffloadStream_->audioActiveDevice_.currentActiveDevice_.deviceType_;
    audioOffloadStream_->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t result = audioOffloadStream_->MoveToNewPipe(sessionId, pipeType);
    // Assert
    EXPECT_NE(result, ERROR);

    result = audioOffloadStream_->MoveToNewPipe(defaultUid, pipeType);
    EXPECT_EQ(result, ERROR);

    audioOffloadStream_->audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    result = audioOffloadStream_->MoveToNewPipe(sessionId, PIPE_TYPE_MULTICHANNEL);
    EXPECT_EQ(result, ERROR);

    audioOffloadStream_->MoveToOutputDevice(sessionId, BLUETOOTH_SPEAKER);
    audioOffloadStream_->audioActiveDevice_.currentActiveDevice_.deviceType_ = oldDeviceType;
}


/**
 * @tc.name  : DynamicUnloadOffloadModule_ShouldReturnSuccess_WhenOffloadIsOpened
 * @tc.number: AudioOffloadStreamTest_009
 * @tc.desc  : Test DynamicUnloadOffloadModule function when offload is opened.
 */
HWTEST_F(AudioOffloadStreamTest, DynamicUnloadOffloadModule_ShouldReturnSuccess_WhenOffloadIsOpened, TestSize.Level3)
{
    // Arrange
    audioOffloadStream_->isOffloadOpened_.store(true);
    // Act
    int32_t result = audioOffloadStream_->DynamicUnloadOffloadModule();
    // Assert
    EXPECT_EQ(result, SUCCESS);
    EXPECT_FALSE(audioOffloadStream_->isOffloadOpened_.load());
}

/**
 * @tc.name  : DynamicUnloadOffloadModule_ShouldReturnSuccess_WhenOffloadIsNotOpened
 * @tc.number: AudioOffloadStreamTest_010
 * @tc.desc  : Test DynamicUnloadOffloadModule function when offload is not opened.
 */
HWTEST_F(AudioOffloadStreamTest, DynamicUnloadOffloadModule_ShouldReturnSuccess_WhenOffloadIsNotOpened, TestSize.Level3)
{
    // Arrange
    audioOffloadStream_->isOffloadOpened_.store(false);
    // Act
    int32_t result = audioOffloadStream_->DynamicUnloadOffloadModule();
    // Assert
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : ReleaseOffloadStream_WhenSessionIdDoesNotMatch
 * @tc.number: AudioOffloadStreamTest_011
 * @tc.desc  : Test if the offload stream is not released when the session ID does not match.
 */
HWTEST_F(AudioOffloadStreamTest, ReleaseOffloadStream_WhenSessionIdDoesNotMatch, TestSize.Level0)
{
    // Arrange
    uint32_t sessionId = 123;
    uint32_t differentSessionId = 456;
    audioOffloadStream_->offloadSessionID_ = sessionId;
    // Act
    audioOffloadStream_->RemoteOffloadStreamRelease(differentSessionId);
    // Assert
    EXPECT_TRUE(audioOffloadStream_->offloadSessionID_.has_value());

    audioOffloadStream_->RemoteOffloadStreamRelease(sessionId);
    EXPECT_FALSE(audioOffloadStream_->offloadSessionID_.has_value());
    audioOffloadStream_->offloadSessionID_ = sessionId;
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_014
 * @tc.desc  : Test scenario when spatialization is enabled and effect offload is enabled.
 */
HWTEST_F(AudioOffloadStreamTest, SpatializationEnabledAndEffectOffloadEnabled, TestSize.Level0)
{
    AudioOffloadStream audioOffloadStream;
    std::vector<int32_t> allSessions = {1, 2, 3};
    audioOffloadStream.ResetOffloadModeOnSpatializationChanged(allSessions);

    EXPECT_EQ(audioOffloadStream.currentPowerState_, PowerMgr::PowerState::AWAKE);

    auto &service = AudioSpatializationService::GetAudioSpatializationService();
    service.spatializationEnabledReal_ = true;
    audioOffloadStream.ResetOffloadModeOnSpatializationChanged(allSessions);

    audioOffloadStream.isOffloadAvailable_ = true;
    audioOffloadStream.offloadSessionID_ = 1;
    audioOffloadStream.ResetOffloadModeOnSpatializationChanged(allSessions);

    audioOffloadStream.OffloadStreamReleaseCheck(0);
    audioOffloadStream.offloadSessionID_.reset();
    audioOffloadStream.OffloadStreamReleaseCheck(0);
    service.spatializationEnabledReal_ = false;
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_015
 * @tc.desc  : Test ConstructMchAudioModuleInfo Interface.
 */
HWTEST_F(AudioOffloadStreamTest, AudioOffloadStreamTest_015, TestSize.Level0)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioOffloadStream audioOffloadStream;
    AudioModuleInfo ret;

    ret = audioOffloadStream.ConstructMchAudioModuleInfo(deviceType);
    EXPECT_EQ(ret.channels, "6");
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_016
 * @tc.desc  : Test LoadMchModule Interface.
 */
HWTEST_F(AudioOffloadStreamTest, AudioOffloadStreamTest_016, TestSize.Level0)
{
    AudioOffloadStream audioOffloadStream;
    int32_t ret;

    ret = audioOffloadStream.LoadMchModule();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_017
 * @tc.desc  : Test ConstructOffloadAudioModuleInfo Interface.
 */
HWTEST_F(AudioOffloadStreamTest, AudioOffloadStreamTest_017, TestSize.Level0)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioOffloadStream audioOffloadStream;
    AudioModuleInfo ret;

    ret = audioOffloadStream.ConstructOffloadAudioModuleInfo(deviceType);
    EXPECT_EQ(ret.channels, "2");
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_018
 * @tc.desc  : Test UnloadMchModule Interface.
 */
HWTEST_F(AudioOffloadStreamTest, AudioOffloadStreamTest_018, TestSize.Level0)
{
    AudioOffloadStream audioOffloadStream;
    int32_t ret;

    ret = audioOffloadStream.UnloadMchModule();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_019
 * @tc.desc  : Test LoadOffloadModule Interface.
 */
HWTEST_F(AudioOffloadStreamTest, AudioOffloadStreamTest_019, TestSize.Level0)
{
    AudioIOHandle audioIOHandle = 3;
    AudioOffloadStream audioOffloadStream;
    int32_t ret;

    ret = audioOffloadStream.LoadOffloadModule();
    EXPECT_EQ(ret, SUCCESS);

    audioOffloadStream.audioIOHandleMap_.IOHandles_[OFFLOAD_PRIMARY_SPEAKER] = audioIOHandle;
    ret = audioOffloadStream.LoadOffloadModule();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_020
 * @tc.desc  : Test UnloadOffloadModule Interface.
 */
HWTEST_F(AudioOffloadStreamTest, AudioOffloadStreamTest_020, TestSize.Level0)
{
    AudioOffloadStream audioOffloadStream;
    int32_t ret;

    audioOffloadStream.isOffloadOpened_.store(true);
    ret = audioOffloadStream.UnloadOffloadModule();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : SpatializationEnabledAndEffectOffloadEnabled
 * @tc.number: AudioOffloadStreamTest_022
 * @tc.desc  : Test OffloadStreamSetCheck Interface.
 */
HWTEST_F(AudioOffloadStreamTest, AudioOffloadStreamTest_022, TestSize.Level0)
{
    uint32_t sessionId = 100;
    AudioOffloadStream audioOffloadStream;
    AudioStreamChangeInfo streamChangeInfo;
    audioOffloadStream.isOffloadAvailable_ = true;
    auto& audioRendererChangeInfo = streamChangeInfo.audioRendererChangeInfo;
    audioRendererChangeInfo = {};
    audioRendererChangeInfo.clientUID = 1;
    audioRendererChangeInfo.sessionId = sessionId;
    audioRendererChangeInfo.channelCount = AudioChannel::STEREO;
    audioRendererChangeInfo.rendererState = RendererState::RENDERER_NEW;
    audioRendererChangeInfo.rendererInfo.pipeType = PIPE_TYPE_DIRECT_VOIP;

    audioOffloadStream.streamCollector_.AddRendererStream(streamChangeInfo);
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.macAddress_ = "00:11:22:33:44:55";
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioOffloadStream.OffloadStreamSetCheck(sessionId);
    EXPECT_EQ(audioOffloadStream.audioActiveDevice_.GetCurrentOutputDeviceNetworkId(), LOCAL_NETWORK_ID);
    EXPECT_EQ(audioOffloadStream.audioActiveDevice_.GetCurrentOutputDeviceType(), DEVICE_TYPE_SPEAKER);
    audioOffloadStream.OffloadStreamSetCheck(sessionId);
    audioOffloadStream.OffloadStreamSetCheck(1);

    audioOffloadStream.streamCollector_.audioRendererChangeInfos_.back()->channelCount = AudioChannel::CHANNEL_3;
    EXPECT_FALSE(audioOffloadStream.CheckStreamMultichannelMode(sessionId));
    audioOffloadStream.streamCollector_.audioRendererChangeInfos_.back()->channelCount = AudioChannel::STEREO;

    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NONE;
    audioOffloadStream.OffloadStreamSetCheck(sessionId);
    EXPECT_EQ(audioOffloadStream.audioActiveDevice_.GetCurrentOutputDeviceNetworkId(), REMOTE_NETWORK_ID);

    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.networkId_ = "";
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NONE;
    audioOffloadStream.OffloadStreamSetCheck(sessionId);
    EXPECT_EQ(audioOffloadStream.audioActiveDevice_.GetCurrentOutputDeviceMacAddr(), "00:11:22:33:44:55");
    EXPECT_FALSE(audioOffloadStream.CheckStreamMultichannelMode(sessionId));

    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.networkId_ = "";
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.macAddress_ = "";
    audioOffloadStream.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NONE;
    audioOffloadStream.OffloadStreamSetCheck(sessionId);
    EXPECT_EQ(audioOffloadStream.streamCollector_.GetStreamType(sessionId), STREAM_MUSIC);
    audioOffloadStream.ResetOffloadStatus(sessionId);
    EXPECT_FALSE(audioOffloadStream.offloadSessionID_.has_value());
}
} // namespace AudioStandard
} // namespace OHOS
