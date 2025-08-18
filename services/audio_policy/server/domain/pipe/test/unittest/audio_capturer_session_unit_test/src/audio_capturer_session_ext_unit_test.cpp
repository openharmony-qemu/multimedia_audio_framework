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

#include "audio_capturer_session_ext_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t THP_EXTRA_SA_UID = 5000;
const std::string PIPE_WAKEUP_INPUT = "wakeup_input";

void AudioCapturerSessionExtTest::SetUpTestCase(void) {}
void AudioCapturerSessionExtTest::TearDownTestCase(void) {}

void AudioCapturerSessionExtTest::SetUp(void)
{
    audioCapturerSession_ = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession_, nullptr);
}

void AudioCapturerSessionExtTest::TearDown(void)
{
    audioCapturerSession_ = nullptr;
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: FindRunningNormalSession_001
 * @tc.desc  : Test !info || sessionWithNormalSourceType_.find(info->sessionId) == sessionWithNormalSourceType_.end()
 */
HWTEST_F(AudioCapturerSessionExtTest, FindRunningNormalSession_001, TestSize.Level4)
{
    AudioStreamCollector &streamCollector = AudioStreamCollector::GetAudioStreamCollector();
    auto info1 = std::make_shared<AudioCapturerChangeInfo>();
    info1->clientUID = THP_EXTRA_SA_UID + 1;
    auto info2 = std::make_shared<AudioCapturerChangeInfo>();
    info2->clientUID = THP_EXTRA_SA_UID + 2;
    streamCollector.audioCapturerChangeInfos_ = { info1, info2 };

    audioCapturerSession_->sessionWithNormalSourceType_.insert({ info2->sessionId, SessionInfo() });
    AudioStreamDescriptor runningSessionInfo;
    EXPECT_FALSE(audioCapturerSession_->FindRunningNormalSession(info1->sessionId, runningSessionInfo));
    EXPECT_FALSE(audioCapturerSession_->FindRunningNormalSession(info2->sessionId, runningSessionInfo));
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadCaptureSession_001
 * @tc.desc  : Test sessionWithNormalSourceType_.count(sessionId) == 0 ||
 *             (specialSourceTypeSet_.count(sessionWithNormalSourceType_[sessionId].sourceType) != 0)
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadCaptureSession_001, TestSize.Level4)
{
    uint32_t sessionId = 1001;
    SessionOperation operation = SessionOperation::SESSION_OPERATION_START;
    audioCapturerSession_->sessionWithNormalSourceType_.clear();
    EXPECT_EQ(audioCapturerSession_->ReloadCaptureSession(sessionId, operation), ERROR);

    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    audioCapturerSession_->sessionWithNormalSourceType_[sessionId] = sessionInfo;
    EXPECT_EQ(audioCapturerSession_->ReloadCaptureSession(sessionId, operation), ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadCaptureSession_002
 * @tc.desc  : Test operation == SESSION_OPERATION_START
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadCaptureSession_002, TestSize.Level4)
{
    uint32_t sessionId = 1001;
    SessionOperation operation = SessionOperation::SESSION_OPERATION_START;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_INVALID;
    audioCapturerSession_->sessionWithNormalSourceType_[sessionId] = sessionInfo;
    EXPECT_EQ(audioCapturerSession_->ReloadCaptureSession(sessionId, operation), ERROR);

    sessionInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    EXPECT_EQ(audioCapturerSession_->ReloadCaptureSession(sessionId, operation), ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadCaptureSession_003
 * @tc.desc  : Test operation == SESSION_OPERATION_PAUSE or SESSION_OPERATION_STOP
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadCaptureSession_003, TestSize.Level4)
{
    uint32_t sessionId = 1001;
    SessionOperation operation = SessionOperation::SESSION_OPERATION_PAUSE;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_INVALID;
    audioCapturerSession_->sessionWithNormalSourceType_[sessionId] = sessionInfo;
    EXPECT_EQ(audioCapturerSession_->ReloadCaptureSession(sessionId, operation), ERROR);

    sessionInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    EXPECT_EQ(audioCapturerSession_->ReloadCaptureSession(sessionId, operation), ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadCaptureSession_004
 * @tc.desc  : Test operation == SESSION_OPERATION_RELEASE
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadCaptureSession_004, TestSize.Level4)
{
    uint32_t sessionId = 1001;
    SessionOperation operation = SessionOperation::SESSION_OPERATION_RELEASE;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_INVALID;
    audioCapturerSession_->sessionWithNormalSourceType_[sessionId] = sessionInfo;
    EXPECT_EQ(audioCapturerSession_->ReloadCaptureSession(sessionId, operation), ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ConstructWakeupAudioModuleInfo_001
 * @tc.desc  : Test audioConfigManager_.GetAdapterInfoByType(type, info) == false
 */
HWTEST_F(AudioCapturerSessionExtTest, ConstructWakeupAudioModuleInfo_001, TestSize.Level4)
{
    audioCapturerSession_->audioConfigManager_.isAdapterInfoMap_ = true;
    audioCapturerSession_->audioConfigManager_.audioPolicyConfig_.adapterInfoMap.clear();
    AudioStreamInfo streamInfo;
    AudioModuleInfo audioModuleInfo;
    EXPECT_FALSE(audioCapturerSession_->ConstructWakeupAudioModuleInfo(streamInfo, audioModuleInfo));
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ConstructWakeupAudioModuleInfo_002
 * @tc.desc  : Test pipeInfo == nullptr
 */
HWTEST_F(AudioCapturerSessionExtTest, ConstructWakeupAudioModuleInfo_002, TestSize.Level4)
{
    audioCapturerSession_->audioConfigManager_.isAdapterInfoMap_ = true;
    std::shared_ptr<PolicyAdapterInfo> audioAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    audioAdapterInfo->pipeInfos.clear();
    audioCapturerSession_->audioConfigManager_.audioPolicyConfig_.adapterInfoMap.emplace(
        AudioAdapterType::TYPE_PRIMARY, audioAdapterInfo);
    AudioStreamInfo streamInfo;
    AudioModuleInfo audioModuleInfo;
    EXPECT_FALSE(audioCapturerSession_->ConstructWakeupAudioModuleInfo(streamInfo, audioModuleInfo));
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ConstructWakeupAudioModuleInfo_003
 * @tc.desc  : Test FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo) == false
 */
HWTEST_F(AudioCapturerSessionExtTest, ConstructWakeupAudioModuleInfo_003, TestSize.Level4)
{
    audioCapturerSession_->audioConfigManager_.isAdapterInfoMap_ = true;
    std::shared_ptr<PolicyAdapterInfo> audioAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    pipeInfo->name_ = PIPE_WAKEUP_INPUT;
    pipeInfo->streamPropInfos_.clear();
    audioAdapterInfo->pipeInfos.push_back(pipeInfo);
    audioCapturerSession_->audioConfigManager_.audioPolicyConfig_.adapterInfoMap.emplace(
        AudioAdapterType::TYPE_PRIMARY, audioAdapterInfo);
    AudioStreamInfo streamInfo;
    AudioModuleInfo audioModuleInfo;
    EXPECT_FALSE(audioCapturerSession_->ConstructWakeupAudioModuleInfo(streamInfo, audioModuleInfo));
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadSourceForDeviceChange_001
 * @tc.desc  : Test sessionWithNormalSourceType_.find(sessionId) != sessionWithNormalSourceType_.end()
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadSourceForDeviceChange_001, TestSize.Level4)
{
    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_INVALID;
    AudioDeviceDescriptor outputDevice;
    std::string caller = "test";
    audioCapturerSession_->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession_->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_MIC;
    uint32_t sessionId = 1001;
    audioCapturerSession_->audioEcManager_.sessionIdUsedToOpenSource_ = sessionId;
    audioCapturerSession_->sessionWithNormalSourceType_[sessionId] = SessionInfo();
    EXPECT_NO_THROW(audioCapturerSession_->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller));
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: GetEnhancePropByName_001
 * @tc.desc  : Test iter == propertyArray.property.end()
 */
HWTEST_F(AudioCapturerSessionExtTest, GetEnhancePropByName_001, TestSize.Level4)
{
    AudioEnhancePropertyArray propertyArray;
    AudioEnhanceProperty property;
    property.enhanceClass = "testClass";
    propertyArray.property.push_back(property);
    std::string propName = "testProperty";
    EXPECT_EQ(audioCapturerSession_->GetEnhancePropByName(propertyArray, propName), "");
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadSourceForEffect_001
 * @tc.desc  : Test audioEcManager_.GetMicRefFeatureEnable() == false
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadSourceForEffect_001, TestSize.Level4)
{
    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;
    audioCapturerSession_->audioEcManager_.isMicRefFeatureEnable_ = false;
    EXPECT_NO_THROW(audioCapturerSession_->ReloadSourceForEffect(oldPropertyArray, newPropertyArray));
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadSourceForEffect_002
 * @tc.desc  : Test audioEcManager_.GetSourceOpened() != SOURCE_TYPE_MIC
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadSourceForEffect_002, TestSize.Level4)
{
    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;
    audioCapturerSession_->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession_->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_NO_THROW(audioCapturerSession_->ReloadSourceForEffect(oldPropertyArray, newPropertyArray));
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: ReloadSourceForEffect_003
 * @tc.desc  : Test audioEcManager_.GetSourceOpened() == SOURCE_TYPE_INVALID
 */
HWTEST_F(AudioCapturerSessionExtTest, ReloadSourceForEffect_003, TestSize.Level4)
{
    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;
    audioCapturerSession_->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession_->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_INVALID;
    EXPECT_NO_THROW(audioCapturerSession_->ReloadSourceForEffect(oldPropertyArray, newPropertyArray));
}
} // namespace AudioStandard
} // namespace OHOS