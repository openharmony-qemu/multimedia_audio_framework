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
#include "audio_policy_utils.h"
#include "audio_adapter_manager_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_interrupt_service.h"
#include "audio_adapter_manager_handler.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static AudioAdapterManager *audioAdapterManager_;

void AudioAdapterManagerUnitTest::SetUpTestCase(void) {}
void AudioAdapterManagerUnitTest::TearDownTestCase(void) {}

std::shared_ptr<AudioInterruptService> GetTnterruptServiceTest()
{
    return std::make_shared<AudioInterruptService>();
}
/**
 * @tc.name: IsAppVolumeMute_001
 * @tc.desc: Test IsAppVolumeMute when owned is true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsAppVolumeMute_001, TestSize.Level1)
{
    int32_t appUid = 12345;
    bool owned = true;
    bool isMute = true;
    bool result = AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: IsAppVolumeMute_002
 * @tc.desc: Test IsAppVolumeMute when owned is false.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsAppVolumeMute_002, TestSize.Level1)
{
    int32_t appUid = 12345;
    bool owned = false;
    bool isMute = true;
    bool result = AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: HandleStreamMuteStatus_001
 * @tc.desc: Test HandleStreamMuteStatus when deviceType is not DEVICE_TYPE_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, HandleStreamMuteStatus_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, deviceType);
    EXPECT_TRUE(mute);
}

/**
 * @tc.name: HandleStreamMuteStatus_002
 * @tc.desc: Test HandleStreamMuteStatus when deviceType is DEVICE_TYPE_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, HandleStreamMuteStatus_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    DeviceType deviceType = DEVICE_TYPE_NONE;
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, deviceType);
    EXPECT_TRUE(mute);
}

/**
 * @tc.name: IsHandleStreamMute_001
 * @tc.desc: Test IsHandleStreamMute when streamType is STREAM_VOICE_CALL.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsHandleStreamMute_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t SUCCESS = 0;
    int32_t result = audioAdapterManager_->IsHandleStreamMute(streamType, mute, streamUsage);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: IsHandleStreamMute_002
 * @tc.desc: Test IsHandleStreamMute when streamType is STREAM_VOICE_CALL.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsHandleStreamMute_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = false;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t result = audioAdapterManager_->IsHandleStreamMute(streamType, mute, streamUsage);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: SetOffloadSessionId_001
 * @tc.desc: Test SetOffloadSessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetOffloadSessionId_001, TestSize.Level1)
{
    uint32_t sessionId = MIN_STREAMID - 1;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId, adapter);

    sessionId = MAX_STREAMID + 1;
    adapter = OFFLOAD_IN_REMOTE;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId, adapter);

    sessionId = MIN_STREAMID + 1;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId, adapter);
}

/**
 * @tc.name: UpdateSinkArgs_001
 * @tc.desc: Test UpdateSinkArgs all args have value
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateSinkArgs_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.name = "hello";
    info.adapterName = "world";
    info.className = "CALSS";
    info.fileName = "sink.so";
    info.sinkLatency = "300ms";
    info.networkId = "ASD**G124";
    info.deviceType = "AE00";
    info.extra = "1:13:2";
    info.needEmptyChunk = true;
    std::string ret {};
    AudioAdapterManager::UpdateSinkArgs(info, ret);
    EXPECT_EQ(ret,
    " sink_name=hello"
    " adapter_name=world"
    " device_class=CALSS"
    " file_path=sink.so"
    " sink_latency=300ms"
    " network_id=ASD**G124"
    " device_type=AE00"
    " split_mode=1:13:2"
    " need_empty_chunk=1");
}

/**
 * @tc.name: UpdateSinkArgs_002
 * @tc.desc: Test UpdateSinkArgs no value: network_id
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateSinkArgs_002, TestSize.Level1)
{
    AudioModuleInfo info;
    std::string ret {};
    AudioAdapterManager::UpdateSinkArgs(info, ret);
    EXPECT_EQ(ret, " network_id=LocalDevice");
}

/**
 * @tc.name: Test SetSystemVolumeDegree
 * @tc.desc: SetSystemVolumeDegree_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSystemVolumeDegree_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeDegree = 44;
    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(streamType);
    ASSERT_NE(desc, nullptr);
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioAdapterManager->handler_ = std::make_shared<AudioAdapterManagerHandler>();
    auto ret = audioAdapterManager->SetSystemVolumeDegree(streamType, volumeDegree);
    EXPECT_EQ(ret, SUCCESS);

    audioAdapterManager->useNonlinearAlgo_ = true;
    ret = audioAdapterManager->SetSystemVolumeDegree(streamType, volumeDegree);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->SetSystemVolumeDegree(STREAM_VOICE_CALL, volumeDegree);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->SetSystemVolumeDegree(STREAM_VOICE_RING, volumeDegree);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->GetSystemVolumeDegree(streamType);
    EXPECT_EQ(ret, volumeDegree);

    EXPECT_EQ(audioAdapterManager->GetStreamVolumeDegreeInternal(desc, streamType), volumeDegree);

    ret = audioAdapterManager->GetMinVolumeDegree(streamType);
    EXPECT_EQ(ret, 0);

    audioAdapterManager->volumeDataMaintainer_.muteStatusMap_[desc->GetName()][streamType] = true;
    ret = audioAdapterManager->GetSystemVolumeDegree(streamType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: Test SetZoneVolumeDegree
 * @tc.desc: SetZoneVolumeDegree_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetZoneVolumeDegree_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeDegree = 44;

    auto ret = audioAdapterManager->GetZoneVolumeDegree(0, streamType);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ret = audioAdapterManager->SetZoneVolumeDegreeToMap(0, streamType, volumeDegree);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(device1, nullptr);
    device1->macAddress_ = "";
    device1->networkId_ = "LocalDevice";

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(device1);
    EXPECT_EQ(AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices), SUCCESS);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device1);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(device1);

    ret = audioAdapterManager->SetZoneVolumeDegreeToMap(zoneId1_, streamType, -1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    ret = audioAdapterManager->SetZoneVolumeDegreeToMap(zoneId1_, streamType, volumeDegree);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->GetZoneVolumeDegree(zoneId1_, streamType);
    EXPECT_EQ(ret, volumeDegree);

    audioAdapterManager->volumeDataMaintainer_.muteStatusMap_[device1->GetName()][streamType] = true;
    ret = audioAdapterManager->GetZoneVolumeDegree(zoneId1_, streamType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: Test SetVolumeData
 * @tc.desc: SaveVolumeDegree_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AudioAdapterManagerUnitTest, SaveVolumeDegree_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 10;

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(desc, nullptr);

    audioAdapterManager->SaveVolumeData(desc, streamType, volumeLevel, false, true);
    audioAdapterManager->SaveVolumeData(desc, streamType, volumeLevel, false, false);
    audioAdapterManager->SaveVolumeData(desc, streamType, volumeLevel, true, false);
    audioAdapterManager->SaveVolumeData(desc, streamType, volumeLevel, true, true);

    int32_t out = audioAdapterManager->GetStreamVolumeInternal(desc, streamType);
    EXPECT_EQ(out, volumeLevel);
    int32_t outDegree = audioAdapterManager->GetStreamVolumeDegreeInternal(desc, streamType);
    EXPECT_NE(outDegree, 0);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_001
 * @tc.type: FUNC
 * @tc.desc: the volumeType is STREAM_APP, return appConfigVolume_.maxVolume.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_001, TestSize.Level1)
{
    int32_t ret = audioAdapterManager_->GetMaxVolumeLevel(STREAM_APP, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, audioAdapterManager_->appConfigVolume_.maxVolume);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_002
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is valid, return the device maxLevel.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_002, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_VOICE_CALL;
    DeviceVolumeType deviceType = SPEAKER_VOLUME_TYPE;
    audioAdapterManager_->Init();
    if (audioAdapterManager_->streamVolumeInfos_.end() != audioAdapterManager_->streamVolumeInfos_.find(volumeType)) {
        if ((audioAdapterManager_->streamVolumeInfos_[volumeType] != nullptr) &&
            (audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos.end() !=
            audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos.find(deviceType)) &&
            (audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType] != nullptr)) {
            audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType]->maxLevel = 10;
        }
    }

    int32_t ret = audioAdapterManager_->GetMaxVolumeLevel(volumeType, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 10);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_003
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is not valid, return maxVolumeIndexMap_[volumeType].
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_003, TestSize.Level1)
{
    int32_t ret = audioAdapterManager_->GetMaxVolumeLevel(STREAM_MUSIC, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, audioAdapterManager_->maxVolumeIndexMap_[STREAM_MUSIC]);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_004
 * @tc.type: FUNC
 * @tc.desc: the volume Type is not valid, return ERR_INVALID_PARAM.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_004, TestSize.Level1)
{
    int32_t ret = audioAdapterManager_->GetMaxVolumeLevel(STREAM_DEFAULT, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_001
 * @tc.type: FUNC
 * @tc.desc: the volumeType is STREAM_APP, return appConfigVolume_.minVolume.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_001, TestSize.Level1)
{
    int32_t ret = audioAdapterManager_->GetMinVolumeLevel(STREAM_APP, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, audioAdapterManager_->appConfigVolume_.minVolume);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_002
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is valid, return the device maxLevel.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_002, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_VOICE_CALL;
    DeviceVolumeType deviceType = SPEAKER_VOLUME_TYPE;
    audioAdapterManager_->Init();
    if (audioAdapterManager_->streamVolumeInfos_.end() != audioAdapterManager_->streamVolumeInfos_.find(volumeType)) {
        if ((audioAdapterManager_->streamVolumeInfos_[volumeType] != nullptr) &&
            (audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos.end() !=
            audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos.find(deviceType)) &&
            (audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType] != nullptr)) {
            audioAdapterManager_->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType]->minLevel = 2;
        }
    }

    int32_t ret = audioAdapterManager_->GetMinVolumeLevel(volumeType, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 2);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_003
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is not valid, return minVolumeIndexMap_[volumeType].
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_003, TestSize.Level1)
{
    int32_t ret = audioAdapterManager_->GetMinVolumeLevel(STREAM_MUSIC, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, audioAdapterManager_->minVolumeIndexMap_[STREAM_MUSIC]);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_004
 * @tc.type: FUNC
 * @tc.desc: the volume Type is not valid, return ERR_INVALID_PARAM.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_004, TestSize.Level1)
{
    int32_t ret = audioAdapterManager_->GetMinVolumeLevel(STREAM_DEFAULT, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: Test GetAudioSourceAttr
 * @tc.number: GetAudioSourceAttr_001
 * @tc.type: FUNC
 * @tc.desc: when inof layout is not empty, passthrought layout to attr
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSourceAttr_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    AudioModuleInfo info;
    info.channelLayout = "263"; // 263 = 100000111
    IAudioSourceAttr attr = audioAdapterManager->GetAudioSourceAttr(info);
    EXPECT_EQ(attr.channelLayout, 263); // 263 = 100000111
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: SetVolumeLimit_001
 * @tc.type: FUNC
 * @tc.desc: Depress volume
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetVolumeLimit_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    float oldLimit = audioAdapterManager->volumeLimit_.load();
    float volume = 0.5f;
    int32_t volumeLevel = 5;

    EXPECT_EQ(audioAdapterManager->SetVolumeDb(STREAM_MUSIC), SUCCESS);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_VOICE_CALL_ASSISTANT, DEVICE_TYPE_SPEAKER);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_ULTRASONIC, DEVICE_TYPE_SPEAKER);
    audioAdapterManager->UpdateOtherStreamVolume(STREAM_VOICE_CALL);

    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_MUSIC, DEVICE_TYPE_SPEAKER);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_VOICE_CALL, DEVICE_TYPE_SPEAKER);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_MUSIC, DEVICE_TYPE_SPEAKER);
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    float newLimit = audioAdapterManager->volumeLimit_.load();
    EXPECT_NE(newLimit, oldLimit);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_MUSIC, DEVICE_TYPE_SPEAKER);

    newLimit = audioAdapterManager->volumeLimit_.load();
    EXPECT_EQ(oldLimit, newLimit);
}

/**
 * @tc.name: Test GetMaxVolumeLevel_New
 * @tc.number: GetMaxVolumeLevel_New
 * @tc.type: FUNC
 * @tc.desc: GetMaxVolumeLevel_New
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_New, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = audioAdapterManager->GetMaxVolumeLevel(STREAM_APP, desc);
    EXPECT_EQ(ret, audioAdapterManager->appConfigVolume_.maxVolume);
    ret = audioAdapterManager->GetMinVolumeLevel(STREAM_APP, desc);
    EXPECT_EQ(ret, audioAdapterManager->appConfigVolume_.minVolume);
}

/**
 * @tc.name: Test SetAudioVolume
 * @tc.number: SetAudioVolume
 * @tc.type: FUNC
 * @tc.desc: SetAudioVolume
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAudioVolume, TestSize.Level1)
{
    auto ad = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamType type = STREAM_MUSIC;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    ad->isAbsVolumeScene_ = true;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), true);

    type = STREAM_APP;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), true);
    
    ad->isAbsVolumeScene_ = false;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), false);

    type = STREAM_MUSIC;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), false);

    desc->deviceType_ = DEVICE_TYPE_NEARLINK;

    ad->isAbsVolumeScene_ = true;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), true);

    type = STREAM_APP;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), true);
    
    ad->isAbsVolumeScene_ = false;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), false);

    type = STREAM_MUSIC;
    ad->SetAudioVolume(desc, type, 0);
    EXPECT_EQ(ad->IsAbsVolumeScene(), false);
}

/**
 * @tc.name: GetDeviceVolume_001
 * @tc.desc: Test GetDeviceVolume
 * @tc.type: FUNC
 * @tc.require: #ICMEH8
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetDeviceVolume_001, TestSize.Level1)
{
    audioAdapterManager_->Init();
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t minVolume = audioAdapterManager_->GetMinVolumeLevel(streamType);
    int32_t maxVolume = audioAdapterManager_->GetMaxVolumeLevel(streamType);
    ASSERT_TRUE(volumeLevel >= minVolume && volumeLevel <= maxVolume);
    int32_t result = audioAdapterManager_->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    ASSERT_EQ(result, 0);
    auto volume = audioAdapterManager_->GetDeviceVolume(deviceType, streamType);
    EXPECT_EQ(volume, volumeLevel);
}

/**
 * @tc.name: Test SetAppVolumeDb
 * @tc.number: SetAppVolumeDb_001
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAppVolumeDb_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t appUid = 123456;
    int32_t volumeLevel = 2;
    audioAdapterManager->volumeDataMaintainer_.SetAppVolume(appUid, volumeLevel);
    std::shared_ptr<AudioDeviceDescriptor> defaultOutputDevice_ =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    defaultOutputDevice_->deviceType_ = DEVICE_TYPE_SPEAKER;
    defaultOutputDevice_->networkId_ = "RemoteDevice";
    uint32_t sessionId = 100001;
    OffloadAdapter adapter = OFFLOAD_IN_REMOTE;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    audioAdapterManager->audioActiveDevice_.defaultOutputDevice_ = defaultOutputDevice_;
    int32_t res = audioAdapterManager->SetAppVolumeDb(appUid);
    EXPECT_EQ(res, SUCCESS);
}

/**
 * @tc.name: Test SetAppVolumeDb
 * @tc.number: SetAppVolumeDb_002
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAppVolumeDb_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t appUid = 123456;
    int32_t volumeLevel = 2;
    audioAdapterManager->volumeDataMaintainer_.SetAppVolume(appUid, volumeLevel);
    std::shared_ptr<AudioDeviceDescriptor> defaultOutputDevice_ =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    defaultOutputDevice_->deviceType_ = DEVICE_TYPE_INVALID;
    defaultOutputDevice_->networkId_ = "RemoteDevice";
    uint32_t sessionId = 100001;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    audioAdapterManager->audioActiveDevice_.defaultOutputDevice_ = defaultOutputDevice_;
    int32_t res = audioAdapterManager->SetAppVolumeDb(appUid);
    EXPECT_EQ(res, SUCCESS);
}

/**
 * @tc.name: Test SetAppVolumeMutedDB
 * @tc.number: SetAppVolumeMutedDB_001
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAppVolumeMutedDB_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t appUid = 123456;
    int32_t volumeLevel = 2;
    bool muted = true;
    audioAdapterManager->volumeDataMaintainer_.SetAppVolume(appUid, volumeLevel);
    std::shared_ptr<AudioDeviceDescriptor> defaultOutputDevice_ =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    defaultOutputDevice_->deviceType_ = DEVICE_TYPE_SPEAKER;
    defaultOutputDevice_->networkId_ = "RemoteDevice";
    uint32_t sessionId = 100001;
    OffloadAdapter adapter = OFFLOAD_IN_REMOTE;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    audioAdapterManager->audioActiveDevice_.defaultOutputDevice_ = defaultOutputDevice_;
    int32_t res = audioAdapterManager->SetAppVolumeMutedDB(appUid, muted);
    EXPECT_EQ(res, SUCCESS);
}

/**
 * @tc.name: Test SetAppVolumeMutedDB
 * @tc.number: SetAppVolumeMutedDB_002
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAppVolumeMutedDB_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t appUid = 123456;
    int32_t volumeLevel = 2;
    bool muted = true;
    audioAdapterManager->volumeDataMaintainer_.SetAppVolume(appUid, volumeLevel);
    std::shared_ptr<AudioDeviceDescriptor> defaultOutputDevice_ =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    defaultOutputDevice_->deviceType_ = DEVICE_TYPE_INVALID;
    defaultOutputDevice_->networkId_ = "RemoteDevice";
    uint32_t sessionId = 100001;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    audioAdapterManager->audioActiveDevice_.defaultOutputDevice_ = defaultOutputDevice_;
    int32_t res = audioAdapterManager->SetAppVolumeMutedDB(appUid, muted);
    EXPECT_EQ(res, SUCCESS);
}


} // namespace AudioStandard
} // namespace OHOS
