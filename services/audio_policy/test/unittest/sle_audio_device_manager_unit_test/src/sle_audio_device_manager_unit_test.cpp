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
    sleAudioDeviceManager_->startedSleStreamType_[device][streamType].sessionIds.insert(element);
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
    
    int32_t ret = sleAudioDeviceManager_->SendUserSelection(device, streamType, USER_SELECT_SLE);
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
    
    int32_t ret = sleAudioDeviceManager_->SendUserSelection(device, streamType, USER_SELECT_SLE);
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
 * @tc.name  : Test GetSleStreamTypeByStreamUsage.
 * @tc.number: GetSleStreamType_003
 * @tc.desc  : Test SleAudioDeviceManager::GetSleStreamTypeByStreamUsage && GetSleStreamTypeBySourceType.
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleStreamType_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    int32_t invalidUid1 = INVALID_UID;

    uint32_t ret = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, invalidUid1);
    EXPECT_EQ(ret, SLE_AUDIO_STREAM_MUSIC);

    int32_t invalidUid2 = 0;
    ret = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, invalidUid2);
    EXPECT_EQ(ret, SLE_AUDIO_STREAM_MUSIC);

    int32_t gameUid = 1;
    int32_t normalUid = 2;
    sleAudioDeviceManager_->clientTypeMap_[gameUid] = true;
    ret = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, normalUid);
    EXPECT_EQ(ret, SLE_AUDIO_STREAM_MUSIC);

    ret = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, gameUid);
    EXPECT_EQ(ret, SLE_AUDIO_STREAM_GAME);
    sleAudioDeviceManager_->clientTypeMap_.clear();
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
    
    uint32_t streamType_1 = SLE_AUDIO_STREAM_NONE;
    std::set<StreamUsage> StreamUsage = {};
    uint32_t streamType_2 = SLE_AUDIO_STREAM_NONE;
    std::set<SourceType> SourceType = {};

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
    EXPECT_EQ(result, SUCCESS);
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
    ret = sleAudioDeviceManager_->StopPlaying(deviceDesc, sourceType);
    EXPECT_EQ(ret, SUCCESS);
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

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, streamUsage, USER_SELECT_SLE);
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

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, streamUsage, USER_SELECT_SLE);
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

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, sourceType, USER_SELECT_SLE);
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

    int32_t result = sleAudioDeviceManager_->SendUserSelection(deviceDesc, sourceType, USER_SELECT_SLE);
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
    bool ret_1 = sleAudioDeviceManager_->IsNearlinkDevice(deviceType_1);
    EXPECT_FALSE(ret_1);

    DeviceType deviceType_2 = DEVICE_TYPE_NEARLINK;
    bool ret_2 = sleAudioDeviceManager_->IsNearlinkDevice(deviceType_2);
    EXPECT_TRUE(ret_2);

    DeviceType deviceType_3 = DEVICE_TYPE_NEARLINK_IN;
    bool ret_3 = sleAudioDeviceManager_->IsNearlinkDevice(deviceType_3);
    EXPECT_TRUE(ret_3);
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

    bool ret = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_FALSE(ret);
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

    bool ret = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_FALSE(ret);
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
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK_IN, INPUT_DEVICE);
    
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_1);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc_2);
    bool ret = sleAudioDeviceManager_->IsMoveToNearlinkDevice(streamDesc);
    EXPECT_TRUE(ret);
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
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc_1 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, INPUT_DEVICE);

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
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    
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
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc_2 =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    
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

/**
 * @tc.name  : Test ResetSleStreamTypeCount
 * @tc.number: ResetSleStreamTypeCount_001
 * @tc.desc  : Test SleAudioDeviceManager::ResetSleStreamTypeCount with nullptr device
 */
HWTEST(SleAudioDeviceManagerUnitTest, ResetSleStreamTypeCount_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string deviceAddr = "TestDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId = 1001;

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId, true);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;

    sleAudioDeviceManager_->ResetSleStreamTypeCount(deviceDesc);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);
    EXPECT_FALSE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test ResetSleStreamTypeCount
 * @tc.number: ResetSleStreamTypeCount_002
 * @tc.desc  : Test SleAudioDeviceManager::ResetSleStreamTypeCount with non-existent device
 */
HWTEST(SleAudioDeviceManagerUnitTest, ResetSleStreamTypeCount_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "NonExistentDevice";

    sleAudioDeviceManager_->ResetSleStreamTypeCount(deviceDesc);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice("NonExistentDevice");
    EXPECT_TRUE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test ResetSleStreamTypeCount
 * @tc.number: ResetSleStreamTypeCount_003
 * @tc.desc  : Test SleAudioDeviceManager::ResetSleStreamTypeCount with active sessions
 */
HWTEST(SleAudioDeviceManagerUnitTest, ResetSleStreamTypeCount_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string deviceAddr = "TestDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId = 1001;

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId, true);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = deviceAddr;

    sleAudioDeviceManager_->ResetSleStreamTypeCount(deviceDesc);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);
    EXPECT_TRUE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test ResetSleStreamTypeCount
 * @tc.number: ResetSleStreamTypeCount_004
 * @tc.desc  : Test SleAudioDeviceManager::ResetSleStreamTypeCount with multiple stream types
 */
HWTEST(SleAudioDeviceManagerUnitTest, ResetSleStreamTypeCount_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string deviceAddr = "MultiStreamDevice";
    uint32_t sessionId1 = 2001;
    uint32_t sessionId2 = 2002;

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, SLE_AUDIO_STREAM_MUSIC, sessionId1, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, SLE_AUDIO_STREAM_RING, sessionId2, true);

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = deviceAddr;

    sleAudioDeviceManager_->ResetSleStreamTypeCount(deviceDesc);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);
    EXPECT_TRUE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test GetNearlinkStreamTypeMapByDevice
 * @tc.number: GetNearlinkStreamTypeMapByDevice_001
 * @tc.desc  : Test SleAudioDeviceManager::GetNearlinkStreamTypeMapByDevice with empty device
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetNearlinkStreamTypeMapByDevice_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string deviceAddr = "EmptyDevice";

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);

    EXPECT_TRUE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test GetNearlinkStreamTypeMapByDevice
 * @tc.number: GetNearlinkStreamTypeMapByDevice_002
 * @tc.desc  : Test SleAudioDeviceManager::GetNearlinkStreamTypeMapByDevice with single session
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetNearlinkStreamTypeMapByDevice_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string deviceAddr = "SingleSessionDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId = 3001;

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId, true);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);

    EXPECT_EQ(streamTypeMap.size(), 1);
    EXPECT_TRUE(streamTypeMap.find(streamType) != streamTypeMap.end());
    EXPECT_EQ(streamTypeMap[streamType].size(), 1);
    EXPECT_TRUE(streamTypeMap[streamType].find(sessionId) != streamTypeMap[streamType].end());
}

/**
 * @tc.name  : Test GetNearlinkStreamTypeMapByDevice
 * @tc.number: GetNearlinkStreamTypeMapByDevice_003
 * @tc.desc  : Test SleAudioDeviceManager::GetNearlinkStreamTypeMapByDevice with multiple sessions
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetNearlinkStreamTypeMapByDevice_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string deviceAddr = "MultiSessionDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId1 = 4001;
    uint32_t sessionId2 = 4002;
    uint32_t sessionId3 = 4003;

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId1, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId2, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId3, true);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);

    EXPECT_EQ(streamTypeMap.size(), 1);
    EXPECT_TRUE(streamTypeMap.find(streamType) != streamTypeMap.end());
    EXPECT_EQ(streamTypeMap[streamType].size(), 3);
}

/**
 * @tc.name  : Test GetNearlinkStreamTypeMapByDevice
 * @tc.number: GetNearlinkStreamTypeMapByDevice_004
 * @tc.desc  : Test SleAudioDeviceManager::GetNearlinkStreamTypeMapByDevice after session removal
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetNearlinkStreamTypeMapByDevice_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string deviceAddr = "RemoveSessionDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId1 = 5001;
    uint32_t sessionId2 = 5002;

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId1, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId2, true);

    auto streamTypeMap1 = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);
    EXPECT_EQ(streamTypeMap1[streamType].size(), 2);

    sleAudioDeviceManager_->UpdateStreamTypeMap(deviceAddr, streamType, sessionId1, false);

    auto streamTypeMap2 = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(deviceAddr);
    EXPECT_EQ(streamTypeMap2[streamType].size(), 1);
    EXPECT_TRUE(streamTypeMap2[streamType].find(sessionId2) != streamTypeMap2[streamType].end());
}

/**
 * @tc.name  : Test IsGameApp
 * @tc.number: IsGameApp_001
 * @tc.desc  : Test SleAudioDeviceManager::IsGameApp with invalid UID
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsGameApp_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t invalidUid = INVALID_UID;

    bool isGame = sleAudioDeviceManager_->IsGameApp(invalidUid);

    EXPECT_FALSE(isGame);
}

/**
 * @tc.name  : Test IsGameApp
 * @tc.number: IsGameApp_002
 * @tc.desc  : Test SleAudioDeviceManager::IsGameApp with non-game UID
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsGameApp_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t normalUid = 1000;

    bool isGame = sleAudioDeviceManager_->IsGameApp(normalUid);

    EXPECT_FALSE(isGame);
}

/**
 * @tc.name  : Test IsGameApp
 * @tc.number: IsGameApp_003
 * @tc.desc  : Test SleAudioDeviceManager::IsGameApp with game app UID
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsGameApp_003, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t gameUid = 1001;
    sleAudioDeviceManager_->clientTypeMap_[gameUid] = true;

    bool isGame = sleAudioDeviceManager_->IsGameApp(gameUid);

    EXPECT_TRUE(isGame);

    sleAudioDeviceManager_->clientTypeMap_.clear();
}

/**
 * @tc.name  : Test IsGameApp
 * @tc.number: IsGameApp_004
 * @tc.desc  : Test SleAudioDeviceManager::IsGameApp with cached result
 */
HWTEST(SleAudioDeviceManagerUnitTest, IsGameApp_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t uid = 1002;
    sleAudioDeviceManager_->clientTypeMap_[uid] = false;

    bool isGame1 = sleAudioDeviceManager_->IsGameApp(uid);
    EXPECT_FALSE(isGame1);

    sleAudioDeviceManager_->clientTypeMap_[uid] = true;
    bool isGame2 = sleAudioDeviceManager_->IsGameApp(uid);
    EXPECT_TRUE(isGame2);

    sleAudioDeviceManager_->clientTypeMap_.clear();
}

/**
 * @tc.name  : Test GetSleStreamTypeByStreamUsage
 * @tc.number: GetSleStreamTypeByStreamUsage_004
 * @tc.desc  : Test GetSleStreamTypeByStreamUsage with non-game app
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleStreamTypeByStreamUsage_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t normalUid = 2000;
    StreamUsage streamUsage = STREAM_USAGE_GAME;

    uint32_t sleStreamType = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, normalUid);

    EXPECT_EQ(sleStreamType, SLE_AUDIO_STREAM_GAME);
}

/**
 * @tc.name  : Test GetSleStreamTypeByStreamUsage
 * @tc.number: GetSleStreamTypeByStreamUsage_005
 * @tc.desc  : Test GetSleStreamTypeByStreamUsage with game app and media usage
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleStreamTypeByStreamUsage_005, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t gameUid = 2001;
    sleAudioDeviceManager_->clientTypeMap_[gameUid] = true;

    StreamUsage streamUsage = STREAM_USAGE_MEDIA;

    uint32_t sleStreamType = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, gameUid);

    EXPECT_EQ(sleStreamType, SLE_AUDIO_STREAM_GAME);

    sleAudioDeviceManager_->clientTypeMap_.clear();
}

/**
 * @tc.name  : Test GetSleStreamTypeByStreamUsage
 * @tc.number: GetSleStreamTypeByStreamUsage_006
 * @tc.desc  : Test GetSleStreamTypeByStreamUsage with game app and unsupported usage
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleStreamTypeByStreamUsage_006, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t gameUid = 2002;
    sleAudioDeviceManager_->clientTypeMap_[gameUid] = true;

    StreamUsage streamUsage = STREAM_USAGE_NOTIFICATION;

    uint32_t sleStreamType = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, gameUid);

    EXPECT_EQ(sleStreamType, SLE_AUDIO_STREAM_UNDEFINED);

    sleAudioDeviceManager_->clientTypeMap_.clear();
}

/**
 * @tc.name  : Test GetSleStreamTypeByStreamUsage
 * @tc.number: GetSleStreamTypeByStreamUsage_007
 * @tc.desc  : Test GetSleStreamTypeByStreamUsage with game app and music usage
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetSleStreamTypeByStreamUsage_007, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    int32_t gameUid = 2003;
    sleAudioDeviceManager_->clientTypeMap_[gameUid] = true;

    StreamUsage streamUsage = STREAM_USAGE_MUSIC;

    uint32_t sleStreamType = sleAudioDeviceManager_->GetSleStreamTypeByStreamUsage(streamUsage, gameUid);

    EXPECT_EQ(sleStreamType, SLE_AUDIO_STREAM_GAME);

    sleAudioDeviceManager_->clientTypeMap_.clear();
}

/**
 * @tc.name  : Test StartPlaying edge case
 * @tc.number: StartPlaying_EdgeCase_001
 * @tc.desc  : Test StartPlaying with already started stream
 */
HWTEST(SleAudioDeviceManagerUnitTest, StartPlaying_EdgeCase_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "AlreadyStartedDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;

    sleAudioDeviceManager_->UpdateStreamTypeMap(device, streamType, 6001, true);
    int32_t ret1 = sleAudioDeviceManager_->StartPlaying(device, streamType);
    EXPECT_EQ(ret1, SUCCESS);

    int32_t ret2 = sleAudioDeviceManager_->StartPlaying(device, streamType);
    EXPECT_EQ(ret2, SUCCESS);
}

/**
 * @tc.name  : Test UpdateStreamTypeMap edge case
 * @tc.number: UpdateStreamTypeMap_EdgeCase_001
 * @tc.desc  : Test UpdateStreamTypeMap with session removal
 */
HWTEST(SleAudioDeviceManagerUnitTest, UpdateStreamTypeMap_EdgeCase_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "RemovalDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId = 7001;

    sleAudioDeviceManager_->UpdateStreamTypeMap(device, streamType, sessionId, true);

    auto streamTypeMap1 = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device);
    EXPECT_EQ(streamTypeMap1.size(), 1);

    sleAudioDeviceManager_->UpdateStreamTypeMap(device, streamType, sessionId, false);

    auto streamTypeMap2 = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device);
    EXPECT_TRUE(streamTypeMap2.empty());
}

/**
 * @tc.name  : Test UpdateSleStreamTypeCount edge case
 * @tc.number: UpdateSleStreamTypeCount_EdgeCase_001
 * @tc.desc  : Test UpdateSleStreamTypeCount for move scenario
 */
HWTEST(SleAudioDeviceManagerUnitTest, UpdateSleStreamTypeCount_EdgeCase_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 8001;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->rendererInfo_ = AudioRendererInfo();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;

    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    oldDeviceDesc->macAddress_ = "OldDevice";
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    newDeviceDesc->macAddress_ = "NewDevice";

    streamDesc->oldDeviceDescs_.push_back(oldDeviceDesc);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);

    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, false);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice("NewDevice");
    EXPECT_FALSE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test UpdateSleStreamTypeCount edge case
 * @tc.number: UpdateSleStreamTypeCount_EdgeCase_002
 * @tc.desc  : Test UpdateSleStreamTypeCount with isRemoved=true for move scenario
 */
HWTEST(SleAudioDeviceManagerUnitTest, UpdateSleStreamTypeCount_EdgeCase_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 9001;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->rendererInfo_ = AudioRendererInfo();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;

    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    oldDeviceDesc->macAddress_ = "OldNearlinkDevice";
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    newDeviceDesc->macAddress_ = "NewSpeakerDevice";

    streamDesc->oldDeviceDescs_.push_back(oldDeviceDesc);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);

    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, true);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice("OldNearlinkDevice");
    EXPECT_TRUE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test SetNearlinkDeviceMute edge case
 * @tc.number: SetNearlinkDeviceMute_EdgeCase_001
 * @tc.desc  : Test SetNearlinkDeviceMute for STREAM_VOICE_CALL
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceMute_EdgeCase_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "VoiceCallDevice";

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = device;
    deviceDesc.mediaVolume_ = 5;
    deviceDesc.callVolume_ = 5;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool isMute = true;

    int32_t ret = sleAudioDeviceManager_->SetNearlinkDeviceMute(device, streamType, isMute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetNearlinkDeviceMute edge case
 * @tc.number: SetNearlinkDeviceMute_EdgeCase_002
 * @tc.desc  : Test SetNearlinkDeviceMute toggle on/off
 */
HWTEST(SleAudioDeviceManagerUnitTest, SetNearlinkDeviceMute_EdgeCase_002, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "MuteToggleDevice";

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = device;
    deviceDesc.mediaVolume_ = 6;
    deviceDesc.callVolume_ = 6;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    AudioStreamType streamType = STREAM_MUSIC;

    sleAudioDeviceManager_->SetNearlinkDeviceMute(device, streamType, true);

    AudioDeviceDescriptor getVolDesc = {};
    getVolDesc.macAddress_ = device;
    int32_t volLevel1 = sleAudioDeviceManager_->GetVolumeLevelByVolumeType(streamType, getVolDesc);
    EXPECT_EQ(volLevel1, 0);

    sleAudioDeviceManager_->SetNearlinkDeviceMute(device, streamType, false);

    int32_t volLevel2 = sleAudioDeviceManager_->GetVolumeLevelByVolumeType(streamType, getVolDesc);
    EXPECT_EQ(volLevel2, 6);
}

/**
 * @tc.name  : Test GetVolumeLevelByVolumeType edge case
 * @tc.number: GetVolumeLevelByVolumeType_EdgeCase_001
 * @tc.desc  : Test GetVolumeLevelByVolumeType when mute is true
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetVolumeLevelByVolumeType_EdgeCase_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "MuteVolumeDevice";

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = device;
    deviceDesc.mediaVolume_ = 7;
    deviceDesc.callVolume_ = 7;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    AudioStreamType streamType = STREAM_MUSIC;
    sleAudioDeviceManager_->SetNearlinkDeviceMute(device, streamType, true);

    AudioDeviceDescriptor getVolDesc = {};
    getVolDesc.macAddress_ = device;
    int32_t volLevel = sleAudioDeviceManager_->GetVolumeLevelByVolumeType(streamType, getVolDesc);

    EXPECT_EQ(volLevel, 0);
}

/**
 * @tc.name  : Test Multiple sessions for same stream type
 * @tc.number: MultipleSessions_001
 * @tc.desc  : Test handling multiple sessions for the same stream type
 */
HWTEST(SleAudioDeviceManagerUnitTest, MultipleSessions_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "MultiSessionTestDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId1 = 10001;
    uint32_t sessionId2 = 10002;
    uint32_t sessionId3 = 10003;

    sleAudioDeviceManager_->UpdateStreamTypeMap(device, streamType, sessionId1, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(device, streamType, sessionId2, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(device, streamType, sessionId3, true);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device);
    EXPECT_EQ(streamTypeMap.size(), 1);
    EXPECT_EQ(streamTypeMap[streamType].size(), 3);

    sleAudioDeviceManager_->UpdateStreamTypeMap(device, streamType, sessionId2, false);

    auto streamTypeMap2 = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device);
    EXPECT_EQ(streamTypeMap2[streamType].size(), 2);
    EXPECT_TRUE(streamTypeMap2[streamType].find(sessionId1) != streamTypeMap2[streamType].end());
    EXPECT_TRUE(streamTypeMap2[streamType].find(sessionId3) != streamTypeMap2[streamType].end());
}

/**
 * @tc.name  : Test Moving from nearlink to other devices
 * @tc.number: NearlinkMove_001
 * @tc.desc  : Test moving audio stream from nearlink to speaker
 */
HWTEST(SleAudioDeviceManagerUnitTest, NearlinkMove_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string nearlinkDevice = "NearlinkToSpeakerDevice";
    uint32_t streamType = SLE_AUDIO_STREAM_MUSIC;
    uint32_t sessionId = 11001;

    sleAudioDeviceManager_->UpdateStreamTypeMap(nearlinkDevice, streamType, sessionId, true);

    std::shared_ptr<AudioStreamDescriptor> streamDesc =
        std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = sessionId;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->rendererInfo_ = AudioRendererInfo();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;

    std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    oldDeviceDesc->macAddress_ = nearlinkDevice;
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    newDeviceDesc->macAddress_ = "SpeakerDevice";

    streamDesc->oldDeviceDescs_.push_back(oldDeviceDesc);
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);

    sleAudioDeviceManager_->UpdateSleStreamTypeCount(streamDesc, false);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(nearlinkDevice);
    EXPECT_TRUE(streamTypeMap.empty());
}

/**
 * @tc.name  : Test Multiple stream types on same device
 * @tc.number: MultipleStreamTypes_001
 * @tc.desc  : Test handling multiple stream types on the same device
 */
HWTEST(SleAudioDeviceManagerUnitTest, MultipleStreamTypes_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string device = "MultiStreamTypeDevice";
    uint32_t musicSession = 12001;
    uint32_t ringSession = 12002;
    uint32_t gameSession = 12003;

    sleAudioDeviceManager_->UpdateStreamTypeMap(device, SLE_AUDIO_STREAM_MUSIC, musicSession, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(device, SLE_AUDIO_STREAM_RING, ringSession, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(device, SLE_AUDIO_STREAM_GAME, gameSession, true);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device);
    EXPECT_EQ(streamTypeMap.size(), 3);
    EXPECT_TRUE(streamTypeMap.find(SLE_AUDIO_STREAM_MUSIC) != streamTypeMap.end());
    EXPECT_TRUE(streamTypeMap.find(SLE_AUDIO_STREAM_RING) != streamTypeMap.end());
    EXPECT_TRUE(streamTypeMap.find(SLE_AUDIO_STREAM_GAME) != streamTypeMap.end());
}

/**
 * @tc.name  : Test GetVolumeLevelByVolumeType for all stream types
 * @tc.number: GetVolumeLevelByVolumeType_004
 * @tc.desc  : Test GetVolumeLevelByVolumeType returns 0 for unsupported stream types
 */
HWTEST(SleAudioDeviceManagerUnitTest, GetVolumeLevelByVolumeType_004, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "UnsupportedStreamDevice";

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = device;
    deviceDesc.mediaVolume_ = 8;
    deviceDesc.callVolume_ = 8;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;

    sleAudioDeviceManager_->AddNearlinkDevice(deviceDesc);

    AudioVolumeType unsupportedVolumeType = STREAM_NOTIFICATION;

    int32_t volLevel = sleAudioDeviceManager_->GetVolumeLevelByVolumeType(unsupportedVolumeType, deviceDesc);
    EXPECT_EQ(volLevel, 0);
}

/**
 * @tc.name  : Test StartPlaying with extended timeout
 * @tc.number: StartPlaying_ExtendedTimeout_001
 * @tc.desc  : Test StartPlaying with extended timeout for specific stream types
 */
HWTEST(SleAudioDeviceManagerUnitTest, StartPlaying_ExtendedTimeout_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    deviceDesc.macAddress_ = "ExtendedTimeoutDevice";

    StreamUsage streamUsage = STREAM_USAGE_VOICE_MESSAGE;

    int32_t ret = sleAudioDeviceManager_->StartPlaying(deviceDesc, streamUsage);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test concurrent stream management
 * @tc.number: ConcurrentStreams_001
 * @tc.desc  : Test managing multiple concurrent streams on different devices
 */
HWTEST(SleAudioDeviceManagerUnitTest, ConcurrentStreams_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();

    std::string device1 = "ConcurrentDevice1";
    std::string device2 = "ConcurrentDevice2";
    uint32_t sessionId1 = 13001;
    uint32_t sessionId2 = 13002;

    sleAudioDeviceManager_->UpdateStreamTypeMap(device1, SLE_AUDIO_STREAM_MUSIC, sessionId1, true);
    sleAudioDeviceManager_->UpdateStreamTypeMap(device2, SLE_AUDIO_STREAM_RING, sessionId2, true);

    auto streamTypeMap1 = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device1);
    auto streamTypeMap2 = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device2);

    EXPECT_EQ(streamTypeMap1.size(), 1);
    EXPECT_EQ(streamTypeMap2.size(), 1);
    EXPECT_TRUE(streamTypeMap1.find(SLE_AUDIO_STREAM_MUSIC) != streamTypeMap1.end());
    EXPECT_TRUE(streamTypeMap2.find(SLE_AUDIO_STREAM_RING) != streamTypeMap2.end());
}

/**
 * @tc.name  : Test device removal with active streams
 * @tc.number: DeviceRemoval_001
 * @tc.desc  : Test removing device with active streams
 */
HWTEST(SleAudioDeviceManagerUnitTest, DeviceRemoval_001, TestSize.Level1)
{
    std::shared_ptr<SleAudioDeviceManager> sleAudioDeviceManager_ =
        std::make_shared<SleAudioDeviceManager>();
    sptr<IStandardSleAudioOperationCallbackTest> callback =
        new(std::nothrow) IStandardSleAudioOperationCallbackTest();
    sleAudioDeviceManager_->SetSleAudioOperationCallback(callback);

    std::string device = "RemovalWithActiveStreams";
    uint32_t sessionId = 14001;

    sleAudioDeviceManager_->UpdateStreamTypeMap(device, SLE_AUDIO_STREAM_MUSIC, sessionId, true);

    AudioDeviceDescriptor deviceDesc = {};
    deviceDesc.macAddress_ = device;
    deviceDesc.deviceType_ = DEVICE_TYPE_NEARLINK;
    deviceDesc.connectState_ = CONNECTED;

    sleAudioDeviceManager_->RemoveNearlinkDevice(deviceDesc);

    auto streamTypeMap = sleAudioDeviceManager_->GetNearlinkStreamTypeMapByDevice(device);
    EXPECT_TRUE(streamTypeMap.empty());
}
} // namespace AudioStandard
} // namespace OHOS
