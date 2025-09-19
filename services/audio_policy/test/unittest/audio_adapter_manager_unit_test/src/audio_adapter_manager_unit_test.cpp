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
 * @tc.name: SetAdjustVolumeForZone_001
 * @tc.desc: Test SetAdjustVolumeForZone
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAdjustVolumeForZone_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto ret = audioAdapterManager->SetAdjustVolumeForZone(0);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE);
    desc->networkId_ = "LocalDevice";
    devices.push_back(desc);
    AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(desc);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(desc);

    ret = audioAdapterManager->SetAdjustVolumeForZone(zoneId2_);
    EXPECT_EQ(ret, SUCCESS);

    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    ret = audioAdapterManager->SetAdjustVolumeForZone(zoneId2_);
    EXPECT_EQ(ret, SUCCESS);

    audioAdapterManager->volumeDataExtMaintainer_.clear();
    desc->networkId_ = "RemoteDevice";
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;

    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    ret = audioAdapterManager->SetAdjustVolumeForZone(zoneId2_);
    EXPECT_EQ(ret, SUCCESS);
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
} // namespace AudioStandard
} // namespace OHOS
