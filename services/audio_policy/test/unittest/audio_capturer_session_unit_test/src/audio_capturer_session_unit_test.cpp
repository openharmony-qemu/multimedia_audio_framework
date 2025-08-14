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
#include "audio_capturer_session_mock_interface.h"

using namespace testing;
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
 * @tc.number: AudioCapturerSession_029
 * @tc.desc  : Test pipe valid
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_029, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_030
 * @tc.desc  : Test AI pipe/pipe role满足
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_030, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_031
 * @tc.desc  : Test AI pipe/pipe role不满足
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_031, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_032
 * @tc.desc  : Test AI pipe/管道为空
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_032, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_033
 * @tc.desc  : Test pipe列表为空，函数返回false
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_033, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_034
 * @tc.desc  : Test pipe列表输出或者NONE，函数返回false
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_034, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_035
 * @tc.desc  : Test routerflag是AI或者fast，函数返回false
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_035, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_036
 * @tc.desc  : Test sessionid相同时，函数返回false
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_036, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_037
 * @tc.desc  : Test stream为空时，函数返回false
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_037, TestSize.Level1)
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
 * @tc.number: AudioCapturerSession_038
 * @tc.desc  : Test IsStreamValid 打桩true
 */
HWTEST(AudioCapturerSessionTest, AudioCapturerSession_038, TestSize.Level1)
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
    
    // IsStreamValid 打桩成true
    auto mockaudioCapturerSession = std::make_shared<AudioCapturerSessionMockInterface>();
    EXPECT_CALL(*(mockaudioCapturerSession), IsStreamValid(_))
        .Times(1)
        .WillOnce(Return(true));

    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCapturerSession->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, true);
    EXPECT_EQ(hasSession, true);
}
} // namespace AudioStandard
} // namespace OHOS
