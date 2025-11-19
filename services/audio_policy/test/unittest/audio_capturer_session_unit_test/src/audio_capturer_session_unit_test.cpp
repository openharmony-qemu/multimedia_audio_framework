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

#include "audio_capturer_session_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_001
 * @tc.desc  : Test AudioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_001, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = false;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_002
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_002, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_003
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_003, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_WAKEUP;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_004
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_004, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_MIC;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_005
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_005, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_MIC;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_006
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_006, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_007
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_007, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "ABC"}, {"voip_up", "ABC"}};

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_008
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_008, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "ABC"}, {"voip_up", "PNR"}};

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_009
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_009, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_010
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_010, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEffectPropertyArrayV3 oldPropertyArray;
    AudioEffectPropertyArrayV3 newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_011
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_011, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEffectPropertyArrayV3 oldPropertyArray;
    AudioEffectPropertyArrayV3 newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "ABC"}, {"voip_up", "PNR"}};

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_012
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_012, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioEffectPropertyArrayV3 oldPropertyArray;
    AudioEffectPropertyArrayV3 newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "ABC"}, {"voip_up", "ABC"}};

    audioCapturerSession->audioEcManager_.isMicRefFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_013
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_013, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_WAKEUP;

    audioCapturerSession->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_014
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_014, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_MIC;

    audioCapturerSession->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_015
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_015, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_016
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_016, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_DEFAULT;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCapturerSession->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_017
 * @tc.desc  : Test AudioCapturerSession::IsVoipDeviceChanged()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_017, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;

    auto ret = audioCapturerSession->IsVoipDeviceChanged(inputDevice, outputDevice);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_018
 * @tc.desc  : Test AudioCapturerSession::FillWakeupStreamPropInfo()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_018, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioStreamInfo streamInfo;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    AudioModuleInfo audioModuleInfo;

    auto ret = audioCapturerSession->FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_019
 * @tc.desc  : Test AudioCapturerSession::FillWakeupStreamPropInfo()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_019, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioStreamInfo streamInfo;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    EXPECT_NE(pipeInfo, nullptr);
    AudioModuleInfo audioModuleInfo;

    auto ret = audioCapturerSession->FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_020
 * @tc.desc  : Test AudioCapturerSession::GetInstance()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_020, TestSize.Level1)
{
    shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = nullptr;
    {
        auto& audioCapturerSession = AudioCapturerSession::GetInstance();

        audioA2dpOffloadManager = make_shared<AudioA2dpOffloadManager>();
        audioCapturerSession.Init(audioA2dpOffloadManager);
        audioCapturerSession.SetConfigParserFlag();

        audioCapturerSession.DeInit();
    }
    EXPECT_EQ(audioA2dpOffloadManager.use_count(), 1);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_021
 * @tc.desc  : Test AudioCapturerSession::OnCapturerSessionAdded()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_021, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    sessionInfo.rate = 44100;
    sessionInfo.channels = 2;
    AudioStreamInfo streamInfo;
    audioCapturerSession->SetConfigParserFlag();

    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.SetDefaultDeviceLoadFlag(true);


    uint64_t sessionID = 1;
    audioCapturerSession->OnCapturerSessionRemoved(sessionID);

    auto ret = audioCapturerSession->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_022
 * @tc.desc  : Test AudioCapturerSession::OnCapturerSessionAdded()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_022, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    SessionInfo sessionInfo;
    AudioStreamInfo streamInfo;

    uint64_t sessionID = 1;

    auto ret = audioCapturerSession->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_023
 * @tc.desc  : Test AudioCapturerSession::SetWakeUpAudioCapturerFromAudioServer()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_023, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioStreamInfo streamInfo;
    AudioProcessConfig config;
    config.streamInfo = streamInfo;

    auto ret = audioCapturerSession->SetWakeUpAudioCapturerFromAudioServer(config);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_024
 * @tc.desc  : Test AudioCapturerSession::CloseWakeUpAudioCapturer()
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_024, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    auto ret = audioCapturerSession->CloseWakeUpAudioCapturer();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_025
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_025, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor outputDevice;
    std::string caller = "testCase";

    const uint64_t testSessionId = 99;
    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_MIC;
    audioCapturerSession->audioEcManager_.sessionIdUsedToOpenSource_ = testSessionId;

    audioCapturerSession->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
    EXPECT_EQ(audioCapturerSession->audioEcManager_.GetOpenedNormalSourceSessionId(), testSessionId);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_026
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_026, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    uint32_t sessionId = 0;
    SessionOperation operation = SESSION_OPERATION_START;
    audioCapturerSession->ReloadCaptureSession(sessionId, operation);
    EXPECT_NE(audioCapturerSession->ReloadCaptureSession(sessionId, operation), ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_027
 * @tc.desc  : Test ReloadSourceForDeviceChange() for inputDeviceForReload default
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_027, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor outputDevice;
    std::string caller = "testCase";

    const uint64_t testSessionId = 99;
    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_MIC;
    audioCapturerSession->audioEcManager_.sessionIdUsedToOpenSource_ = testSessionId;
    audioCapturerSession->inputDeviceForReload_.deviceType_ = DEVICE_TYPE_DEFAULT;

    audioCapturerSession->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
    EXPECT_EQ(audioCapturerSession->inputDeviceForReload_.deviceType_, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_028
 * @tc.desc  : Test ReloadSourceForDeviceChange() for inputDeviceForReload_ valid
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_028, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor outputDevice;
    std::string caller = "testCase";

    const uint64_t testSessionId = 99;
    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    audioCapturerSession->audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_MIC;
    audioCapturerSession->audioEcManager_.sessionIdUsedToOpenSource_ = testSessionId;
    audioCapturerSession->inputDeviceForReload_.deviceType_ = DEVICE_TYPE_MIC;

    audioCapturerSession->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
    EXPECT_EQ(audioCapturerSession->inputDeviceForReload_.deviceType_, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_032
 * @tc.desc  : Test ReloadCaptureSessionSoftLink
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_032, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
    auto pipeManager = AudioPipeManager::GetPipeManager();
    EXPECT_NE(pipeManager, nullptr);

    auto pipeInfoOne = std::make_shared<AudioPipeInfo>();
    pipeInfoOne->name_ = "test_output_one";
    pipeInfoOne->adapterName_ = "test_one";
    pipeManager->AddAudioPipeInfo(pipeInfoOne);

    auto pipeInfoTwo = std::make_shared<AudioPipeInfo>();
    pipeInfoTwo->name_ = "test_output_two";
    pipeInfoTwo->adapterName_ = "test_two";
    pipeInfoTwo->pipeRole_ = AudioPipeRole::PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> streamDescriptor = std::make_shared<AudioStreamDescriptor>();
    streamDescriptor->sessionId_ = 0;
    pipeInfoTwo->streamDescriptors_.push_back(streamDescriptor);
    pipeManager->AddAudioPipeInfo(pipeInfoTwo);

    auto pipeInfoThree = std::make_shared<AudioPipeInfo>();
    pipeInfoThree->name_ = "test_input_three";
    pipeInfoThree->adapterName_ = "primary";
    pipeInfoThree->pipeRole_ = AudioPipeRole::PIPE_ROLE_INPUT;
    pipeInfoThree->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorOne = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorOne->sessionId_ = 1;
    pipeInfoThree->streamDescriptors_.push_back(streamDescriptorOne);
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorTwo = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorTwo->sessionId_ = 2;
    pipeInfoThree->streamDescriptors_.push_back(streamDescriptorTwo);
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorThree = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorThree->sessionId_ = 3;
    pipeInfoThree->streamDescriptors_.push_back(streamDescriptorThree);
    pipeInfoThree->softLinkFlag_ = true;
    pipeManager->AddAudioPipeInfo(pipeInfoThree);

    SessionInfo sessionInfoTwo;
    sessionInfoTwo.sourceType = SourceType::SOURCE_TYPE_WAKEUP;
    SessionInfo sessionInfoTheee;
    sessionInfoTheee.sourceType = SourceType::SOURCE_TYPE_VOICE_CALL;
    audioCapturerSession->sessionWithNormalSourceType_[2] = sessionInfoTwo;
    audioCapturerSession->sessionWithNormalSourceType_[3] = sessionInfoTheee;

    auto ret = audioCapturerSession->ReloadCaptureSessionSoftLink();
    EXPECT_EQ(ret, SUCCESS);

    auto ioRet = AudioIOHandleMap::GetInstance().ClosePortAndEraseIOHandle("test");
    EXPECT_EQ(ioRet, SUCCESS);

    auto pipeRet = pipeManager->GetUnusedRecordPipe();
    EXPECT_EQ(pipeRet.size(), 0);
    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_033
 * @tc.desc  : Test AI pipe/pipe role is input
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_033, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);
 
    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_034
 * @tc.desc  : Test AI pipe/pipe role is not input
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_034, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);
 
    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_035
 * @tc.desc  : Test AI pipe is null
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_035, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
 
    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_036
 * @tc.desc  : Test pipe list is null
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_036, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_037
 * @tc.desc  : Test pipe is out or none
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_037, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeList.push_back(pipe);
    
    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
 
    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipenew->pipeRole_ = PIPE_ROLE_NONE;
    pipeList.push_back(pipenew);
    result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_038
 * @tc.desc  : Test routerflag is AI or Fast
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_038, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);
    
    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
 
    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeList.push_back(pipenew);
    result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_039
 * @tc.desc  : Test sessionid is same
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_039, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
 
    uint32_t sessionId = 1;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = sessionId;
    pipe->streamDescriptors_.push_back(stream);
    pipeList.push_back(pipe);
    
    // sessionId相同
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_040
 * @tc.desc  : Test stream is null
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_040, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
 
    uint32_t sessionId = 1;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 2;
    pipe->streamDescriptors_.push_back(stream);
    pipeList.push_back(pipe);
    
    // stream为空
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_041
 * @tc.desc  : Test IsStreamValid
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_041, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 1;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    bool result = audioCapturerSession->IsStreamValid(stream);
 
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_042
 * @tc.desc  : Test IsStreamValid
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_042, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 1;
    stream->streamStatus_ = STREAM_STATUS_STOPPED; // 无效的 streamStatus_
    audioCapturerSession->sessionWithNormalSourceType_[stream->sessionId_].sourceType = SOURCE_TYPE_MIC;
    bool result = audioCapturerSession->IsStreamValid(stream);
 
    EXPECT_FALSE(result);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_043
 * @tc.desc  : Test IsStreamValid
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_043, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 1;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    audioCapturerSession->sessionWithNormalSourceType_[stream->sessionId_].sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    bool result = audioCapturerSession->IsStreamValid(stream);
 
    EXPECT_FALSE(result);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_044
 * @tc.desc  : Test IsStreamValid
 */
 HWTEST(AudioCapturerSessionTest, AudioCapturerSession_044, TestSize.Level1)
 {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 1;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    audioCapturerSession->sessionWithNormalSourceType_[stream->sessionId_].sourceType = SOURCE_TYPE_MIC;
    bool result = audioCapturerSession->IsStreamValid(stream);
 
    EXPECT_TRUE(result);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_045
 * @tc.desc  : Test FindRunningNormalSession pipe is null
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_045, TestSize.Level1) {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCapturerSession->FindRunningNormalSession(1, runningSessionInfo);
 
    EXPECT_FALSE(result);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_046
 * @tc.desc  : Test FindRunningNormalSession pipe is output
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_046, TestSize.Level1) {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 1;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 1;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeInfo->name_ = "AIPipe";
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;

    pipeInfo->streamDescriptors_.push_back(streamDesc);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->AddAudioPipeInfo(pipeInfo);
 
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCapturerSession->FindRunningNormalSession(1, runningSessionInfo);
 
    EXPECT_FALSE(result);
    pipeManager->curPipeList_.clear();
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_047
 * @tc.desc  : Test FindRunningNormalSession AI valid pipe
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_047, TestSize.Level1) {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 1;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;

    auto streamDescSecond = std::make_shared<AudioStreamDescriptor>();
    streamDescSecond->sessionId_ = 2;
    streamDescSecond->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    streamDescSecond->streamStatus_ = STREAM_STATUS_STARTED;
    streamDescSecond->capturerInfo_.sourceType = SOURCE_TYPE_UNPROCESSED;

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 1;
    pipeInfo->routeFlag_ = (uint32_t)AUDIO_INPUT_FLAG_AI;
    pipeInfo->name_ = "AIPipe";
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;

    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeInfo->streamDescriptors_.push_back(streamDescSecond);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->AddAudioPipeInfo(pipeInfo);
 
    AudioStreamDescriptor runningSessionInfo;
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMap =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_TRANSCRIPTION,
        AudioSourceStrategyType {
        "AUDIO_INPUT_VOICE_TRANSCRIPTION",  // hdiSource
        "primary",               // adapterName
        "primary_input_AI",      // pipeName
        AUDIO_INPUT_FLAG_AI, // audioFlag
        1                       // priority
        });
    sourceStrategyMap->emplace(SOURCE_TYPE_UNPROCESSED,
        AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input_AI",     // pipeName
        AUDIO_INPUT_FLAG_AI, // audioFlag
        2                       // priority
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);
    bool result = audioCapturerSession->FindRunningNormalSession(1, runningSessionInfo);
 
    EXPECT_TRUE(result);
    pipeManager->curPipeList_.clear();
    AudioSourceStrategyData::GetInstance().GetSourceStrategyMap()->clear();
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_048
 * @tc.desc  : Test FindRunningNormalSession normal pipe
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_048, TestSize.Level1) {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 1;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;

    auto streamDescSecond = std::make_shared<AudioStreamDescriptor>();
    streamDescSecond->sessionId_ = 2;
    streamDescSecond->streamStatus_ = STREAM_STATUS_STARTED;
    streamDescSecond->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 1;
    pipeInfo->routeFlag_ = (uint32_t)AUDIO_INPUT_FLAG_NORMAL;
    pipeInfo->name_ = "normalPipe";
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;

    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeInfo->streamDescriptors_.push_back(streamDescSecond);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->AddAudioPipeInfo(pipeInfo);
 
    AudioStreamDescriptor runningSessionInfo;
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMap =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_MIC,
        AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input",      // pipeName
        AUDIO_INPUT_FLAG_NORMAL, // audioFlag
        1                       // priority
        });
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_RECOGNITION,
        AudioSourceStrategyType {
        "AUDIO_INPUT_VOICE_RECOGNITION_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input",     // pipeName
        AUDIO_INPUT_FLAG_NORMAL, // audioFlag
        2                       // priority
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);

    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    audioCapturerSession->sessionWithNormalSourceType_[streamDesc->sessionId_] = sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioCapturerSession->sessionWithNormalSourceType_[streamDescSecond->sessionId_] = sessionInfo;
    audioCapturerSession->audioEcManager_.isEcFeatureEnable_ = true;
    bool result = audioCapturerSession->FindRunningNormalSession(1, runningSessionInfo);
 
    EXPECT_TRUE(result);
    pipeManager->curPipeList_.clear();
    AudioSourceStrategyData::GetInstance().GetSourceStrategyMap()->clear();
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_049
 * @tc.desc  : Test CompareIndependentxmlPriority sourcestrategy map is null
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_049, TestSize.Level1) {
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->streamDescriptors_.push_back(std::make_shared<AudioStreamDescriptor>());
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
 
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(nullptr);
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
    bool result = audioCapturerSession->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);
 
    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_052
 * @tc.desc  : Test CompareIndependentxmlPriority stream is null
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_052, TestSize.Level1) {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    auto pipe = std::make_shared<AudioPipeInfo>();
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCapturerSession->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);
 
    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_053
 * @tc.desc  : Test CompareIndependentxmlPriority sessionid is same
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_053, TestSize.Level1) {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    auto pipe = std::make_shared<AudioPipeInfo>();
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 1;
    pipe->streamDescriptors_.push_back(stream);
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCapturerSession->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);
 
    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_054
 * @tc.desc  : Test CompareIndependentxmlPriority streamStatus_is not start
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_054, TestSize.Level1) {
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    auto pipe = std::make_shared<AudioPipeInfo>();
    auto firstStream = std::make_shared<AudioStreamDescriptor>();
    firstStream->streamStatus_ = STREAM_STATUS_STOPPED;
    firstStream->sessionId_ = 1;

    auto secondStream = std::make_shared<AudioStreamDescriptor>();
    secondStream->streamStatus_ = STREAM_STATUS_STOPPED;
    secondStream->sessionId_ = 2;

    pipe->streamDescriptors_.push_back(firstStream);
    pipe->streamDescriptors_.push_back(secondStream);
 
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCapturerSession->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);
 
    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_055
 * @tc.desc  : Test CompareIndependentxmlPriority source strategy map can’t find sourcetype
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_055, TestSize.Level1) {
    auto pipe = std::make_shared<AudioPipeInfo>();
    auto firstStream = std::make_shared<AudioStreamDescriptor>();
    firstStream->streamStatus_ = STREAM_STATUS_STARTED;
    firstStream->sessionId_ = 1;

    auto secondStream = std::make_shared<AudioStreamDescriptor>();
    secondStream->streamStatus_ = STREAM_STATUS_STARTED;
    secondStream->sessionId_ = 2;

    pipe->streamDescriptors_.push_back(firstStream);
    pipe->streamDescriptors_.push_back(secondStream);
 
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;

    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
    bool result = audioCapturerSession->CompareIndependentxmlPriority(pipe, 2, runningSessionInfo, hasSession);
 
    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}
 
/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_056
 * @tc.desc  : Test CompareIndependentxmlPriority enter check
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_056, TestSize.Level1) {
    auto pipe = std::make_shared<AudioPipeInfo>();
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    stream1->streamStatus_ = STREAM_STATUS_STARTED;
    stream1->sessionId_ = 1;
    pipe->streamDescriptors_.push_back(stream1);
 
    auto stream2 = std::make_shared<AudioStreamDescriptor>();
    stream2->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    stream2->streamStatus_ = STREAM_STATUS_STARTED;
    stream2->sessionId_ = 2;
    pipe->streamDescriptors_.push_back(stream2);
 
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
 
    // 创建sourceStrategyMap，SOURCE_TYPE_VOICE_CALL的优先级高于SOURCE_TYPE_MIC
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_MIC,
        AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input",         // pipeName
        AUDIO_INPUT_FLAG_NORMAL, // audioFlag
        5                       // priority
        });
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_CALL,
        AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input",         // pipeName
        AUDIO_INPUT_FLAG_NORMAL, // audioFlag
        6                       // priority
        });

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
    bool result = audioCapturerSession->CompareIndependentxmlPriority(pipe, stream2->sessionId_, runningSessionInfo,
        hasSession);
 
    EXPECT_TRUE(result);
    EXPECT_TRUE(hasSession);
    EXPECT_EQ(runningSessionInfo.sessionId_, stream1->sessionId_);
    AudioSourceStrategyData::GetInstance().GetSourceStrategyMap()->clear();
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_057
 * @tc.desc  : Test AI pipe/pipe role is input
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_057, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
 
    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    incommingPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    bool isinvalidpipe = audioCapturerSession->IsInvalidPipeRole(incommingPipe);
    EXPECT_EQ(isinvalidpipe, true);
 
    incommingPipe->pipeRole_ = PIPE_ROLE_NONE;
    isinvalidpipe = audioCapturerSession->IsInvalidPipeRole(incommingPipe);
    EXPECT_EQ(isinvalidpipe, true);
 
    incommingPipe->pipeRole_ = PIPE_ROLE_INPUT;
    isinvalidpipe = audioCapturerSession->IsInvalidPipeRole(incommingPipe);
    EXPECT_EQ(isinvalidpipe, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_058
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_058, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    SessionInfo sessionInfo;
    sessionInfo.sourceType = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    uint32_t sessionId = 0;
    SessionOperation operation = SESSION_OPERATION_RELEASE;
    audioCapturerSession->sessionWithNormalSourceType_[sessionId] = sessionInfo;
    audioCapturerSession->ReloadCaptureSession(sessionId, operation);
    EXPECT_EQ(audioCapturerSession->ReloadCaptureSession(sessionId, operation), ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_062
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_062, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_063
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_063, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_064
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_064, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->adapterName_ = ADAPTER_TYPE_VA;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_065
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_065, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->adapterName_ = ADAPTER_TYPE_VA;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_066
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_066, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->adapterName_ = "test_one";
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_067
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_067, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->adapterName_ = "test_one";
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_068
 * @tc.desc  : Test ReloadCaptureSessionSoftLink with invalid result.
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_068, TestSize.Level1)
{
    auto audioCapturerSession = std::make_shared<AudioCapturerSession>();
    EXPECT_NE(audioCapturerSession, nullptr);
    auto pipeManager = AudioPipeManager::GetPipeManager();
    EXPECT_NE(pipeManager, nullptr);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->name_ = "primary_input";
    pipeInfo->adapterName_ = "primary";
    pipeInfo->pipeRole_ = AudioPipeRole::PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorOne = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorOne->sessionId_ = 1;
    streamDescriptorOne->streamStatus_ = STREAM_STATUS_STARTED;
    pipeInfo->streamDescriptors_.push_back(streamDescriptorOne);
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorTwo = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorTwo->sessionId_ = 2;
    streamDescriptorTwo->streamStatus_ = STREAM_STATUS_STARTED;
    pipeInfo->streamDescriptors_.push_back(streamDescriptorTwo);
    pipeInfo->softLinkFlag_ = true;
    pipeManager->AddAudioPipeInfo(pipeInfo);

    SessionInfo sessionInfoMic;
    sessionInfoMic.sourceType = SourceType::SOURCE_TYPE_MIC;
    SessionInfo sessionInfoCall;
    sessionInfoCall.sourceType = SourceType::SOURCE_TYPE_VOICE_CALL;
    audioCapturerSession->sessionWithNormalSourceType_[1] = sessionInfoMic;
    audioCapturerSession->sessionWithNormalSourceType_[2] = sessionInfoCall;

    auto ret = audioCapturerSession->ReloadCaptureSessionSoftLink();
    EXPECT_EQ(ret, ERROR);
}
} // namespace AudioStandard
} // namespace OHOS
