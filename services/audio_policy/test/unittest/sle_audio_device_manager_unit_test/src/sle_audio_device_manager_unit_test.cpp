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

#include "sle_audio_device_manager_unit_test.h"
#include "audio_stream_info.h"
#include "audio_info.h"

#include <vector>
#include <set>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
    void SleAudioDeviceManagerUnitTest::SetUpTestCase(void) {}
    void SleAudioDeviceManagerUnitTest::TearDownTestCase(void) {}
    void SleAudioDeviceManagerUnitTest::SetUp(void) {}
    void SleAudioDeviceManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test SetSleAudioOperationCallback.
 * @tc.number: SetSleAudioOperationCallback_001
 * @tc.desc  : Test SleAudioDeviceManager::SetSleAudioOperationCallback.
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetSleAudioOperationCallback_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;

    int32_t ret = sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetSleAudioOperationCallback.
 * @tc.number: SetSleAudioOperationCallback_002
 * @tc.desc  : Test SleAudioDeviceManager::SetSleAudioOperationCallback.
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetSleAudioOperationCallback_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();

    int32_t ret = sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetSleAudioDeviceList.
 * @tc.number: GetSleAudioDeviceList_001
 * @tc.desc  : Test SleAudioDeviceManager::GetSleAudioDeviceList.
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleAudioDeviceList_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;

    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::vector<AudioDeviceDescriptor> devices = {};
    sleAudioDeviceManager_->GetSleAudioDeviceList(devices);

    EXPECT_EQ(devices.size(), 0);
}

/**
 * @tc.name  : Test GetSleAudioDeviceList.
 * @tc.number: GetSleAudioDeviceList_002
 * @tc.desc  : Test SleAudioDeviceManager::GetSleAudioDeviceList.
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleAudioDeviceList_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor desc_1 = {};
    AudioDeviceDescriptor desc_2 = {};

    std::vector<AudioDeviceDescriptor> devices = {desc_1, desc_2};
    sleAudioDeviceManager_->GetSleAudioDeviceList(devices);

    EXPECT_NE(devices.size(), 0);
}

/**
 * @tc.name  : Test GetSleVirtualAudioDeviceList.
 * @tc.number: GetSleVirtualAudioDeviceList_001
 * @tc.desc  : Test SleAudioDeviceManager::GetSleVirtualAudioDeviceList.
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleVirtualAudioDeviceList_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::vector<AudioDeviceDescriptor> devices;
    sleAudioDeviceManager_->GetSleVirtualAudioDeviceList(devices);

    EXPECT_EQ(devices.size(), 0);
}

/**
 * @tc.name  : Test GetSleVirtualAudioDeviceList.
 * @tc.number: GetSleVirtualAudioDeviceList_002
 * @tc.desc  : Test SleAudioDeviceManager::GetSleVirtualAudioDeviceList.
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleVirtualAudioDeviceList_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new (std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor desc_1 = {};
    AudioDeviceDescriptor desc_2 = {};

    std::vector<AudioDeviceDescriptor> devices = {desc_1, desc_2};
    sleAudioDeviceManager_->GetSleVirtualAudioDeviceList(devices);

    EXPECT_NE(devices.size(), 0);
}

/**
 * @tc.name  : Test IsInBandRingOpen && GetSupportStreamType.
 * @tc.number: MixAudioDeviceTest_001
 * @tc.desc  : Test SleAudioDeviceManager::IsInBandRingOpen && GetSupportStreamType.
 */
HWTEST(SleAudioDeviceManagerUnitTest, MixAudioDeviceTest_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);
    std::string device = "LocalDevice";

    bool ret = sleAudioDeviceManager_->IsInBandRingOpen(device);
    EXPECT_EQ(ret, false);

    uint32_t result = sleAudioDeviceManager_->GetSupportStreamType(device);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test IsInBandRingOpen && GetSupportStreamType.
 * @tc.number: MixAudioDeviceTest_002
 * @tc.desc  : Test SleAudioDeviceManager::IsInBandRingOpen && GetSupportStreamType.
 */
HWTEST(SleAudioDeviceManagerUnitTest, MixAudioDeviceTest_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);
    std::string device = "LocalDevice";

    bool ret = sleAudioDeviceManager_->IsInBandRingOpen(device);
    EXPECT_EQ(ret, true);

    uint32_t result = sleAudioDeviceManager_->GetSupportStreamType(device);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SetActiveSinkDevice && StartPlaying && StopPlaying.
 * @tc.number: MixAudioDeviceTest_003
 * @tc.desc  : Test SleAudioDeviceManager::SetActiveSinkDevice && StartPlaying && StopPlaying.
 */
HWTEST(SleAudioDeviceManagerUnitTest, MixAudioDeviceTest_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "LocalDevice";
    uint32_t streamType = 1;

    int32_t ret = sleAudioDeviceManager_->SetActiveSinkDevice(device, streamType);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = sleAudioDeviceManager_->StartPlaying(device, streamType);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = sleAudioDeviceManager_->StopPlaying(device, streamType);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetActiveSinkDevice && StartPlaying && StopPlaying.
 * @tc.number: MixAudioDeviceTest_004
 * @tc.desc  : Test SleAudioDeviceManager::SetActiveSinkDevice && StartPlaying && StopPlaying.
 */
HWTEST(SleAudioDeviceManagerUnitTest, MixAudioDeviceTest_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "LocalDevice";
    uint32_t streamType = 1;

    int32_t ret = sleAudioDeviceManager_->SetActiveSinkDevice(device, streamType);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t element = 100;
    sleAudioDeviceManager_->startedSleStreamType_[device][streamType].insert(element);
    ret = sleAudioDeviceManager_->StartPlaying(device, streamType);
    EXPECT_EQ(ret, SUCCESS);

    ret = sleAudioDeviceManager_->StopPlaying(device, streamType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ConnectAllowedProfiles.
 * @tc.number: ConnectAllowedProfiles_001
 * @tc.desc  : Test SleAudioDeviceManager::ConnectAllowedProfiles.
 */
HWTEST(SleAudioDeviceManagerUnitTest, ConnectAllowedProfiles_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string remoteAddr = "1234567890";

    int32_t ret = sleAudioDeviceManager_->ConnectAllowedProfiles(remoteAddr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test ConnectAllowedProfiles.
 * @tc.number: ConnectAllowedProfiles_002
 * @tc.desc  : Test SleAudioDeviceManager::ConnectAllowedProfiles.
 */
HWTEST(SleAudioDeviceManagerUnitTest, ConnectAllowedProfiles_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string remoteAddr = "1234567890";

    int32_t ret = sleAudioDeviceManager_->ConnectAllowedProfiles(remoteAddr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetDeviceAbsVolume.
 * @tc.number: SetDeviceAbsVolume_001
 * @tc.desc  : Test SleAudioDeviceManager::SetDeviceAbsVolume.
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetDeviceAbsVolume_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string remoteAddr = "1234567890";
    uint32_t volume = 4;
    uint32_t streamType = 1;

    int32_t ret = sleAudioDeviceManager_->SetDeviceAbsVolume(remoteAddr, volume, streamType);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetDeviceAbsVolume.
 * @tc.number: SetDeviceAbsVolume_002
 * @tc.desc  : Test SleAudioDeviceManager::SetDeviceAbsVolume.
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetDeviceAbsVolume_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string remoteAddr = "1234567890";
    uint32_t volume = 4;
    uint32_t streamType = 1;

    int32_t ret = sleAudioDeviceManager_->SetDeviceAbsVolume(remoteAddr, volume, streamType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SendUserSelection && GetRenderPosition.
 * @tc.number: Send_Get_001
 * @tc.desc  : Test SleAudioDeviceManager::SendUserSelection && GetRenderPosition.
 */
HWTEST(SleAudioDeviceManagerUnitTest, Send_Get_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "LocalDevice";
    uint32_t streamType = 1;
    uint32_t delayValue = 0;
    
    int32_t ret = sleAudioDeviceManager_->SendUserSelection(device, streamType);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    int32_t result = sleAudioDeviceManager_->GetRenderPosition(device, delayValue);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SendUserSelection && GetRenderPosition.
 * @tc.number: Send_Get_002
 * @tc.desc  : Test SleAudioDeviceManager::SendUserSelection && GetRenderPosition.
 */
HWTEST(SleAudioDeviceManagerUnitTest, Send_Get_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "LocalDevice";
    uint32_t streamType = 1;
    uint32_t delayValue = 0;
    
    int32_t ret = sleAudioDeviceManager_->SendUserSelection(device, streamType);
    EXPECT_EQ(ret, SUCCESS);

    int32_t result = sleAudioDeviceManager_->GetRenderPosition(device, delayValue);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test GetSleStreamTypeByStreamUsage && GetSleStreamTypeBySourceType.
 * @tc.number: GetSleStreamType_001
 * @tc.desc  : Test SleAudioDeviceManager::GetSleStreamTypeByStreamUsage && GetSleStreamTypeBySourceType.
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleStreamType_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    StreamUsage streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;

    uint32_t ret = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage);
    EXPECT_EQ(ret, SLE_AUDIO_STREAM_UNDEFINED);

    uint32_t result = sleAudioDeviceManager_->GetSleStreamTypeBySourceType(sourceType);
    EXPECT_EQ(result, SLE_AUDIO_STREAM_VOICE_CALL);
}

/**
 * @tc.name  : Test GetSleStreamTypeByStreamUsage && GetSleStreamTypeBySourceType.
 * @tc.number: GetSleStreamType_002
 * @tc.desc  : Test SleAudioDeviceManager::GetSleStreamTypeByStreamUsage && GetSleStreamTypeBySourceType.
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleStreamType_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    SourceType sourceType = SOURCE_TYPE_INVALID;

    uint32_t ret = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage);
    EXPECT_EQ(ret, SLE_AUDIO_STREAM_NONE);

    uint32_t result = sleAudioDeviceManager_->GetSleStreamTypeBySourceType(sourceType);
    EXPECT_EQ(result, SLE_AUDIO_STREAM_NONE);
}

/**
 * @tc.name  : Test GetSourceTypesBySleStreamType && GetStreamUsagesBySleStreamType.
 * @tc.number: BySleStreamType_001
 * @tc.desc  : Test SleAudioDeviceManager::GetSourceTypesBySleStreamType && GetStreamUsagesBySleStreamType.
 */
HWTEST(SleAudioDeviceManagerUnitTest, BySleStreamType_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    uint32_t streamType_1 = SLE_AUDIO_STREAM_MUSIC;
    std::set<StreamUsage> StreamUsage = {STREAM_USAGE_UNKNOWN, STREAM_USAGE_MEDIA, STREAM_USAGE_MUSIC,
        STREAM_USAGE_AUDIOBOOK, STREAM_USAGE_VOICE_ASSISTANT};
    uint32_t streamType_2 = SLE_AUDIO_STREAM_VOICE_ASSISTANT;
    std::set<SourceType> SourceType = {SOURCE_TYPE_VOICE_RECOGNITION, SOURCE_TYPE_VOICE_TRANSCRIPTION};

    auto ret = sleAudioDeviceManager_->GetStreamUsagesBySleStreamType(streamType_1);
    EXPECT_EQ(ret, StreamUsage);

    auto result = sleAudioDeviceManager_->GetSourceTypesBySleStreamType(streamType_2);
    EXPECT_EQ(result, SourceType);
}

/**
 * @tc.name  : Test GetSourceTypesBySleStreamType && GetStreamUsagesBySleStreamType.
 * @tc.number: BySleStreamType_002
 * @tc.desc  : Test SleAudioDeviceManager::GetSourceTypesBySleStreamType && GetStreamUsagesBySleStreamType.
 */
HWTEST(SleAudioDeviceManagerUnitTest, BySleStreamType_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    uint32_t streamType_1 = STREAM_USAGE_INVALID;
    uint32_t streamType_2 = SOURCE_TYPE_INVALID;

    auto ret = sleAudioDeviceManager_->GetStreamUsagesBySleStreamType(streamType_1);
    EXPECT_TRUE(ret.empty());

    auto result = sleAudioDeviceManager_->GetSourceTypesBySleStreamType(streamType_2);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name  : Test SetActiveDevice
 * @tc.number: SetActiveDevice_001
 * @tc.desc  : Test SleAudioDeviceManager::SetActiveDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetActiveDevice_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    deviceDesc.macAddress_ = "LocalDevice";
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;

    int32_t ret = sleAudioDeviceManager_->SetActiveDevice(deviceDesc, streamUsage);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetActiveDevice
 * @tc.number: SetActiveDevice_002
 * @tc.desc  : Test SleAudioDeviceManager::SetActiveDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetActiveDevice_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);
    
    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    deviceDesc.macAddress_ = "LocalDevice";
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;

    int32_t ret = sleAudioDeviceManager_->SetActiveDevice(deviceDesc, sourceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test StartPlaying
 * @tc.number: StartPlaying_001
 * @tc.desc  : Test SleAudioDeviceManager::StartPlaying
 */
HWTEST(SleAudioDeviceManagerUnitTest, StartPlaying_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_FILE_SOURCE;

    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;

    int32_t ret = sleAudioDeviceManager_->StartPlaying(deviceDesc, streamUsage);
    EXPECT_EQ(ret, ERROR);

    int32_t result = sleAudioDeviceManager_->StartPlaying(deviceDesc, sourceType);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test StartPlaying
 * @tc.number: StartPlaying_002
 * @tc.desc  : Test SleAudioDeviceManager::StartPlaying
 */
HWTEST(SleAudioDeviceManagerUnitTest, StartPlaying_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);
    
    AudioDeviceDescriptor deviceDesc = {};

    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t ret = sleAudioDeviceManager_->StartPlaying(deviceDesc, streamUsage);
    EXPECT_EQ(ret, SUCCESS);

    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;
    int32_t result = sleAudioDeviceManager_->StartPlaying(deviceDesc, sourceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test StopPlaying
 * @tc.number: StopPlaying_001
 * @tc.desc  : Test SleAudioDeviceManager::StopPlaying
 */
HWTEST(SleAudioDeviceManagerUnitTest, StopPlaying_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_FILE_SOURCE;

    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;

    int32_t ret = sleAudioDeviceManager_->StopPlaying(deviceDesc, streamUsage);
    EXPECT_EQ(ret, ERROR);

    int32_t result = sleAudioDeviceManager_->StopPlaying(deviceDesc, sourceType);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test StopPlaying
 * @tc.number: StopPlaying_002
 * @tc.desc  : Test SleAudioDeviceManager::StopPlaying
 */
HWTEST(SleAudioDeviceManagerUnitTest, StopPlaying_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor deviceDesc = {};

    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    int32_t ret = sleAudioDeviceManager_->StopPlaying(deviceDesc, streamUsage);
    EXPECT_EQ(ret, SUCCESS);

    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;
    int32_t result = sleAudioDeviceManager_->StopPlaying(deviceDesc, sourceType);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SetDeviceAbsVolume
 * @tc.number: SetDeviceAbsVolume_003
 * @tc.desc  : Test SleAudioDeviceManager::SetDeviceAbsVolume
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetDeviceAbsVolume_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string device = "LocalDevice";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volume = 4;

    int32_t result = sleAudioDeviceManager_->SetDeviceAbsVolume(device, streamType, volume);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetDeviceAbsVolume
 * @tc.number: SetDeviceAbsVolume_004
 * @tc.desc  : Test SleAudioDeviceManager::SetDeviceAbsVolume
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetDeviceAbsVolume_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string device = "LocalDevice";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volume = -1;

    int32_t result = sleAudioDeviceManager_->SetDeviceAbsVolume(device, streamType, volume);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SendUserSelection
 * @tc.number: SendUserSelection_001
 * @tc.desc  : Test SleAudioDeviceManager::SendUserSelection
 */
HWTEST(SleAudioDeviceManagerUnitTest, SendUserSelection_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_FILE_SOURCE;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, streamUsage);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test SendUserSelection
 * @tc.number: SendUserSelection_002
 * @tc.desc  : Test SleAudioDeviceManager::SendUserSelection
 */
HWTEST(SleAudioDeviceManagerUnitTest, SendUserSelection_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, streamUsage);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test SendUserSelection
 * @tc.number: SendUserSelection_003
 * @tc.desc  : Test SleAudioDeviceManager::SendUserSelection
 */
HWTEST(SleAudioDeviceManagerUnitTest, SendUserSelection_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback = nullptr;
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, sourceType);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test SendUserSelection
 * @tc.number: SendUserSelection_004
 * @tc.desc  : Test SleAudioDeviceManager::SendUserSelection
 */
HWTEST(SleAudioDeviceManagerUnitTest, SendUserSelection_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);
    
    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    SourceType sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, sourceType);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AddNearlinkDevice
 * @tc.number: AddNearlinkDevice_001
 * @tc.desc  : Test SleAudioDeviceManager::AddNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, AddNearlinkDevice_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK_IN;

    int32_t result = sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test AddNearlinkDevice
 * @tc.number: AddNearlinkDevice_002
 * @tc.desc  : Test SleAudioDeviceManager::AddNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, AddNearlinkDevice_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";
    deviceDesc.mediaVolume_ = 4;
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    int32_t result = sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test RemoveNearlinkDevice
 * @tc.number: RemoveNearlinkDevice_001
 * @tc.desc  : Test SleAudioDeviceManager::RemoveNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, RemoveNearlinkDevice_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK_IN;

    int32_t result = sleAudioDeviceManager_->RemoveNearlinkDevice(deviceDesc);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test RemoveNearlinkDevice
 * @tc.number: RemoveNearlinkDevice_002
 * @tc.desc  : Test SleAudioDeviceManager::RemoveNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, RemoveNearlinkDevice_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    int32_t result = sleAudioDeviceManager_->RemoveNearlinkDevice(deviceDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test IsNearlinkDevice
 * @tc.number: IsNearlinkDevice_001
 * @tc.desc  : Test SleAudioDeviceManager::IsNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsNearlinkDevice_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    DeviceType deviceType_1 = DEVICE_TYPE_FILE_SOURCE;
    int32_t ret_1 = sleAudioDeviceManager_->IsNearlinkDevice(deviceType_1);
    EXPECT_EQ(ret_1, false);

    DeviceType deviceType_2 = DEVICE_TYPE_NEARLINK;
    int32_t ret_2 = sleAudioDeviceManager_->IsNearlinkDevice(deviceType_2);
    EXPECT_EQ(ret_2, true);

    DeviceType deviceType_3 = DEVICE_TYPE_NEARLINK_IN;
    int32_t ret_3 = sleAudioDeviceManager_->IsNearlinkDevice(deviceType_3);
    EXPECT_EQ(ret_3, true);
}

/**
 * @tc.name  : Test IsMoveToNearlinkDevice
 * @tc.number: IsMoveToNearlinkDevice_001
 * @tc.desc  : Test SleAudioDeviceManager::IsMoveToNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsMoveToNearlinkDevice_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;

    int32_t ret = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsMoveToNearlinkDevice
 * @tc.number: IsMoveToNearlinkDevice_002
 * @tc.desc  : Test SleAudioDeviceManager::IsMoveToNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsMoveToNearlinkDevice_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();

    int32_t ret = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsMoveToNearlinkDevice
 * @tc.number: IsMoveToNearlinkDevice_003
 * @tc.desc  : Test SleAudioDeviceManager::IsMoveToNearlinkDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsMoveToNearlinkDevice_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();

    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_1 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE, 1, 1, "");
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK_IN, INPUT_DEVICE, 1, 1, "");
    
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_1);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_2);
    int32_t ret = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsNearlinkMoveToOtherDevice
 * @tc.number: IsNearlinkMoveToOtherDevice_001
 * @tc.desc  : Test SleAudioDeviceManager::IsNearlinkMoveToOtherDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsNearlinkMoveToOtherDevice_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;

    int32_t ret = sleAudioDeviceManager_->IsNearlinkMoveToOtherDevice(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsNearlinkMoveToOtherDevice
 * @tc.number: IsNearlinkMoveToOtherDevice_002
 * @tc.desc  : Test SleAudioDeviceManager::IsNearlinkMoveToOtherDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsNearlinkMoveToOtherDevice_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();
    
    int32_t ret = sleAudioDeviceManager_->IsNearlinkMoveToOtherDevice(streamDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsNearlinkMoveToOtherDevice
 * @tc.number: IsNearlinkMoveToOtherDevice_003
 * @tc.desc  : Test SleAudioDeviceManager::IsNearlinkMoveToOtherDevice
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsNearlinkMoveToOtherDevice_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();

    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_1 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, 1, 1, "");
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE, 1, 1, "");
    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc_1 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, INPUT_DEVICE, 1, 1, "");

    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_1);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_2);
    streamDesc->oldDeviceDescs_.push_back(oldDeviceDesc_1);
    
    int32_t ret = sleAudioDeviceManager_->IsNearlinkMoveToOtherDevice(streamDesc);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test UpdateStreamTypeMap
 * @tc.number: UpdateStreamTypeMap_001
 * @tc.desc  : Test SleAudioDeviceManager::UpdateStreamTypeMap
 */
HWTEST(SleAudioDeviceManagerUnitTest, UpdateStreamTypeMap_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string deviceAddr = "1234567890";
    uint32_t streamType = 1;
    uint32_t sessionId = 1000;
    bool isAdd = true;

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId, isAdd);
    int32_t ret = sleAudioDeviceManager_->StartPlaying(deviceAddr, streamType);
    EXPECT_EQ(ret, SUCCESS);

    isAdd = false;
    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId, isAdd);
    ret = sleAudioDeviceManager_->StopPlaying(deviceAddr, streamType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateSleStreamTypeCount
 * @tc.number: UpdateSleStreamTypeCount_001
 * @tc.desc  : Test SleAudioDeviceManager::UpdateSleStreamTypeCount
 */
HWTEST(SleAudioDeviceManagerUnitTest, UpdateSleStreamTypeCount_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();
    bool isRemoved = false;
    streamDesc->sessionId_ = 1000;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->rendererInfo_ = AudioRendererInfo();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;

    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    uint32_t ret_1 = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamDesc->rendererInfo_.streamUsage);
    EXPECT_EQ(ret_1, SLE_AUDIO_STREAM_MUSIC);

    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_1 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE, 1, 1, "");
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, 1, 1, "");
    
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_1);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_2);

    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    bool ret_2 = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_EQ(ret_2, true);

    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    EXPECT_FALSE(sleAudioDeviceManager_->startedSleStreamType_.empty());

    isRemoved = true;
    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    EXPECT_FALSE(sleAudioDeviceManager_->startedSleStreamType_.empty());

    bool ret_3 =  sleAudioDeviceManager_->IsNearlinkMoveToOtherDevice(streamDesc);
    EXPECT_EQ(ret_3, false);
}

/**
 * @tc.name  : Test UpdateSleStreamTypeCount
 * @tc.number: UpdateSleStreamTypeCount_002
 * @tc.desc  : Test SleAudioDeviceManager::UpdateSleStreamTypeCount
 */
HWTEST(SleAudioDeviceManagerUnitTest, UpdateSleStreamTypeCount_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();
    bool isRemoved = false;
    streamDesc->sessionId_ = 1000;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->capturerInfo_ = AudioCapturerInfo();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;

    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    uint32_t ret_1 = sleAudioDeviceManager_->GetSleStreamTypeBySourceType(streamDesc->capturerInfo_.sourceType);
    EXPECT_EQ(ret_1, SLE_AUDIO_STREAM_VOICE_CALL);

    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_1 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE, 1, 1, "");
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, 1, 1, "");
    
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_1);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_2);

    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    bool ret_2 = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_EQ(ret_2, true);

    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    EXPECT_FALSE(sleAudioDeviceManager_->startedSleStreamType_.empty());

    isRemoved = true;
    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, isRemoved);
    EXPECT_FALSE(sleAudioDeviceManager_->startedSleStreamType_.empty());

    bool ret_3 =  sleAudioDeviceManager_->IsNearlinkMoveToOtherDevice(streamDesc);
    EXPECT_EQ(ret_3, false);
}

/**
 * @tc.name  : Test SetNearlinkDeviceMute
 * @tc.number: SetNearlinkDeviceMute_001
 * @tc.desc  : Test SleAudioDeviceManager::SetNearlinkDeviceMute
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceMute_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "1234567890";
    AudioStreamType streamType = STREAM_MUSIC;
    bool isMute = true;

    int32_t ret =  sleAudioDeviceManager_->SetNearlinkDeviceMute(device, streamType, isMute);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetNearlinkDeviceMute
 * @tc.number: SetNearlinkDeviceMute_002
 * @tc.desc  : Test SleAudioDeviceManager::SetNearlinkDeviceMute
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceMute_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "1234567890";
    AudioStreamType streamType_1 = STREAM_MUSIC;
    bool isMute = true;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";
    deviceDesc.mediaVolume_ = 4;
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    int32_t ret =  sleAudioDeviceManager_->SetNearlinkDeviceMute(device, streamType_1, isMute);
    EXPECT_EQ(ret, SUCCESS);

    AudioStreamType streamType_2 = STREAM_MEDIA;
    ret =  sleAudioDeviceManager_->SetNearlinkDeviceMute(device, streamType_2, isMute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetNearlinkDeviceVolumeLevel
 * @tc.number: SetNearlinkDeviceVolumeLevel_001
 * @tc.desc  : Test SleAudioDeviceManager::SetNearlinkDeviceVolumeLevel
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceVolumeLevel_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "1234567890";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 4;

    int32_t ret =  sleAudioDeviceManager_->SetNearlinkDeviceVolumeLevel(device, streamType, volumeLevel);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetNearlinkDeviceVolumeLevel
 * @tc.number: SetNearlinkDeviceVolumeLevel_002
 * @tc.desc  : Test SleAudioDeviceManager::SetNearlinkDeviceVolumeLevel
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceVolumeLevel_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "1234567890";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 4;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";
    deviceDesc.mediaVolume_ = 4;
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    int32_t ret =  sleAudioDeviceManager_->SetNearlinkDeviceVolumeLevel(device, streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetVolumeLevelByVolumeType
 * @tc.number: GetVolumeLevelByVolumeType_001
 * @tc.desc  : Test SleAudioDeviceManager::GetVolumeLevelByVolumeType
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetVolumeLevelByVolumeType_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioVolumeType volumeType = STREAM_MUSIC;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";

    int32_t ret =  sleAudioDeviceManager_->GetVolumeLevelByVolumeType(volumeType, deviceDesc);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetVolumeLevelByVolumeType
 * @tc.number: GetVolumeLevelByVolumeType_002
 * @tc.desc  : Test SleAudioDeviceManager::GetVolumeLevelByVolumeType
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetVolumeLevelByVolumeType_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioVolumeType volumeType = STREAM_ENFORCED_AUDIBLE;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    int32_t ret =  sleAudioDeviceManager_->GetVolumeLevelByVolumeType(volumeType, deviceDesc);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test GetVolumeLevelByVolumeType
 * @tc.number: GetVolumeLevelByVolumeType_003
 * @tc.desc  : Test SleAudioDeviceManager::GetVolumeLevelByVolumeType
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetVolumeLevelByVolumeType_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioVolumeType volumeType = STREAM_VOICE_CALL;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "LocalDevice";
    deviceDesc.mediaVolume_ = 4;
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    std::string device = "LocalDevice";
    SleVolumeConfigInfo volumeConfigInfo_1;
    SleVolumeConfigInfo volumeConfigInfo_2;

    sleAudioDeviceManager_->deviceVolumeConfigInfo_[device].first = volumeConfigInfo_1;
    sleAudioDeviceManager_->deviceVolumeConfigInfo_[device].second = volumeConfigInfo_2;

    int32_t ret =  sleAudioDeviceManager_->GetVolumeLevelByVolumeType(volumeType, deviceDesc);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name  : Test SetDeviceAbsVolume.
 * @tc.number: SetDeviceAbsVolume_005
 * @tc.desc  : Test SleAudioDeviceManager::SetDeviceAbsVolume.
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetDeviceAbsVolume_005, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "1234567890";
    AudioStreamType streamType1 = STREAM_MUSIC;
    AudioStreamType streamType2 = STREAM_RING;
    int32_t volume = 4;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";
    deviceDesc.mediaVolume_ = 4;
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    int32_t ret = sleAudioDeviceManager_->SetDeviceAbsVolume(device, streamType1, volume);
    EXPECT_EQ(ret, SUCCESS);
    ret = sleAudioDeviceManager_->SetDeviceAbsVolume(device, streamType2, volume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetNearlinkDeviceVolumeLevel
 * @tc.number: SetNearlinkDeviceVolumeLevel_003
 * @tc.desc  : Test SleAudioDeviceManager::SetNearlinkDeviceVolumeLevel
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceVolumeLevel_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "1234567890";
    AudioStreamType streamType = STREAM_RING;
    int32_t volumeLevel = 4;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";
    deviceDesc.mediaVolume_ = 4;
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    int32_t ret =  sleAudioDeviceManager_->SetNearlinkDeviceVolumeLevel(device, streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetNearlinkDeviceVolumeLevel
 * @tc.number: SetNearlinkDeviceVolumeLevel_004
 * @tc.desc  : Test SleAudioDeviceManager::SetNearlinkDeviceVolumeLevel
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceVolumeLevel_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "1234567890";
    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volumeLevel = 4;

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = "1234567890";
    deviceDesc.mediaVolume_ = 4;
    deviceDesc.callVolume_ = 4;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    int32_t ret =  sleAudioDeviceManager_->SetNearlinkDeviceVolumeLevel(device, streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);

    streamType = STREAM_VOICE_CALL;
    volumeLevel = -4;
    ret =  sleAudioDeviceManager_->SetNearlinkDeviceVolumeLevel(device, streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);

    streamType = STREAM_RING;
    volumeLevel = 4;
    ret =  sleAudioDeviceManager_->SetNearlinkDeviceVolumeLevel(device, streamType, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS