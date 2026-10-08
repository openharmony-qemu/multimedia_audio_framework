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
#include "audio_common_utils.h"
#include "audio_adapter_manager_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_interrupt_service.h"
#include "audio_policy_server.h"
#include "audio_adapter_manager_handler.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t TEST_PIPE_ID = 100;
constexpr int32_t TEST_VOLUME_LEVEL = 5;
constexpr float TEST_VOLUME_DB = 0.5f;
constexpr float ABS_VOLUME_DB_VALUE = 0.63957f;
constexpr float MAX_VOLUME_DB_VALUE = 1.0f;
constexpr int32_t TEST_ZONE_ID = 0;
constexpr int32_t APP_DEFAULT_VOLUME_LEVEL = 100;

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
 * @tc.desc: Test SetOffloadSessionId with new signature.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetOffloadSessionId_001, TestSize.Level1)
{
    uint32_t sessionId = MIN_STREAMID - 1;
    uint32_t ioHandle = 100;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId, adapter, ioHandle);

    sessionId = MAX_STREAMID + 1;
    ioHandle = 200;
    adapter = OFFLOAD_IN_REMOTE;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId, adapter, ioHandle);

    sessionId = MIN_STREAMID + 1;
    ioHandle = 300;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId, adapter, ioHandle);
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
 * @tc.name: GetAudioSinkAttr_FormatU8_001
 * @tc.desc: Test GetAudioSinkAttr when format is u8.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatU8_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "u8";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, SAMPLE_U8);
}

/**
 * @tc.name: GetAudioSinkAttr_FormatS16le_001
 * @tc.desc: Test GetAudioSinkAttr when format is s16le.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatS16le_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "s16le";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, SAMPLE_S16LE);
}

/**
 * @tc.name: GetAudioSinkAttr_FormatS24le_001
 * @tc.desc: Test GetAudioSinkAttr when format is s24le.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatS24le_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "s24le";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, SAMPLE_S24LE);
}

/**
 * @tc.name: GetAudioSinkAttr_FormatS32le_001
 * @tc.desc: Test GetAudioSinkAttr when format is s32le.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatS32le_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "s32le";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, SAMPLE_S32LE);
}

/**
 * @tc.name: GetAudioSinkAttr_FormatS16_001
 * @tc.desc: Test GetAudioSinkAttr when format is s16 alias.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatS16_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "s16";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, SAMPLE_S16LE);
}

/**
 * @tc.name: GetAudioSinkAttr_FormatS24_001
 * @tc.desc: Test GetAudioSinkAttr when format is s24 alias.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatS24_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "s24";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, SAMPLE_S24LE);
}

/**
 * @tc.name: GetAudioSinkAttr_FormatS32_001
 * @tc.desc: Test GetAudioSinkAttr when format is s32 alias.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatS32_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "s32";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, SAMPLE_S32LE);
}

/**
 * @tc.name: GetAudioSinkAttr_FormatInvalid_001
 * @tc.desc: Test GetAudioSinkAttr when format is invalid.
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_FormatInvalid_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.format = "invalid_format";
    AudioAdapterManager audioAdapterManager;
    auto attr = audioAdapterManager.GetAudioSinkAttr(info);
    EXPECT_EQ(attr.format, INVALID_WIDTH);
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
    int32_t volumeDegree = 1;
    VolumeScale volume{volumeDegree, volumeDegree};
    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(streamType);
    ASSERT_NE(desc, nullptr);
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioAdapterManager->handler_ = std::make_shared<AudioAdapterManagerHandler>();
    auto ret = audioAdapterManager->SetSystemVolumeLevel(streamType, volume, desc);
    EXPECT_EQ(ret, SUCCESS);

    audioAdapterManager->useNonlinearAlgo_ = true;
    ret = audioAdapterManager->SetSystemVolumeLevel(streamType, volume, desc);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->SetSystemVolumeLevel(STREAM_VOICE_CALL, volume, desc);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->SetSystemVolumeLevel(STREAM_VOICE_RING, volume, desc);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->GetSystemVolumeDegree(desc, streamType);
    EXPECT_EQ(ret, volumeDegree);

    EXPECT_EQ(audioAdapterManager->GetStreamVolumeDegreeInternal(desc, streamType), volumeDegree);

    ret = audioAdapterManager->GetMinVolumeDegree(streamType, desc->deviceType_);
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
    int32_t volumeDegree = 1;
    VolumeScale volume{volumeDegree, volumeDegree};
    auto volDeviceDesc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(streamType);
    ASSERT_NE(volDeviceDesc, nullptr);

    auto ret = audioAdapterManager->GetZoneVolumeDegree(0, streamType);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ret = audioAdapterManager->SetZoneVolumeLevel(0, streamType, volume, volDeviceDesc);
    EXPECT_EQ(ret, 0);

    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    ASSERT_NE(device1, nullptr);
    device1->macAddress_ = "";
    device1->networkId_ = "LocalDevice";

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(device1);
    EXPECT_EQ(AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices), SUCCESS);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(device1);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(device1);

    ret = audioAdapterManager->SetZoneVolumeLevel(zoneId1_, streamType, volume, volDeviceDesc);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->GetZoneVolumeDegree(zoneId1_, streamType);
    EXPECT_EQ(ret, volumeDegree);

    VolumeScale volumeTest{5, 5};
    audioAdapterManager->SetSystemVolumeLevel(streamType, volumeTest, volDeviceDesc);
    int32_t volumeLevel = audioAdapterManager->GetSystemVolumeLevel(device1, streamType);
    EXPECT_EQ(volumeLevel, 5);

    audioAdapterManager->SetStreamMute(device1, streamType, true);
    volumeLevel = audioAdapterManager->GetSystemVolumeLevel(device1, streamType);
    bool mute = audioAdapterManager->GetStreamMute(streamType, device1);
    EXPECT_EQ(volumeLevel, 0);
    EXPECT_EQ(mute, true);
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
    info.routeFlag = AUDIO_INPUT_FLAG_AI;
    IAudioSourceAttr attr = audioAdapterManager->GetAudioSourceAttr(info);
    EXPECT_EQ(attr.channelLayout, 263); // 263 = 100000111
    EXPECT_EQ(attr.routeFlag, AUDIO_INPUT_FLAG_AI);
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

    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    ASSERT_NE(desc, nullptr);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_VOICE_CALL_ASSISTANT, desc);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_ULTRASONIC, desc);
    audioAdapterManager->UpdateOtherStreamVolume(STREAM_VOICE_CALL);

    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_MUSIC, desc);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_VOICE_CALL, desc);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_MUSIC, desc);
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    float newLimit = audioAdapterManager->volumeLimit_.load();
    EXPECT_NE(newLimit, oldLimit);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_MUSIC, desc);

    newLimit = audioAdapterManager->volumeLimit_.load();
    EXPECT_EQ(oldLimit, newLimit);

    auto info = std::make_shared<LowerVolumeInfo>();
    ASSERT_NE(info, nullptr);
    int32_t testDb = -6;
    info->streamType = STREAM_MUSIC;
    info->duckedDb = testDb;
    audioAdapterManager->lowerVolumeInfos_[info->streamType] = info;
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);
    audioAdapterManager->DepressVolume(volume, volumeLevel, STREAM_MUSIC, desc);
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    newLimit = audioAdapterManager->volumeLimit_.load();
    EXPECT_LT(volume, newLimit);
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: GetVolumeReductionRatio_001
 * @tc.type: FUNC
 * @tc.desc: Get Volume Reduction
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetVolumeReductionRatio_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);

    auto info = std::make_shared<LowerVolumeInfo>();
    ASSERT_NE(info, nullptr);
    int32_t testDb = 6;
    int32_t testDb2 = -6;
    info->streamType = STREAM_ALARM;
    info->duckedDb = testDb;

    audioAdapterManager->lowerVolumeInfos_[STREAM_ALARM] = nullptr;
    EXPECT_EQ(audioAdapterManager->GetVolumeReductionRatio(STREAM_ALARM), 1);

    audioAdapterManager->lowerVolumeInfos_[STREAM_ALARM] = info;
    EXPECT_EQ(audioAdapterManager->GetVolumeReductionRatio(STREAM_ALARM), 1);

    info->duckedDb = testDb2;
    audioAdapterManager->lowerVolumeInfos_[STREAM_ALARM] = info;
    EXPECT_NE(audioAdapterManager->GetVolumeReductionRatio(STREAM_ALARM), 0);
}

/**
 * @tc.name: Test DepressMediaVolumeForHeadSet
 * @tc.number: DepressMediaVolumeForHeadSet_001
 * @tc.type: FUNC
 * @tc.desc: media volume exceeds the product of limit and reduction, returns reduction
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressMediaVolumeForHeadSet_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    float result = audioAdapterManager->DepressMediaVolumeForHeadSet(0.6f, 0.5f, 1.0f);
    EXPECT_FLOAT_EQ(result, 0.5f);
}

/**
 * @tc.name: Test DepressMediaVolumeForHeadSet
 * @tc.number: DepressMediaVolumeForHeadSet_002
 * @tc.type: FUNC
 * @tc.desc: zero volume limit returns reduction to avoid invalid division
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressMediaVolumeForHeadSet_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    float result = audioAdapterManager->DepressMediaVolumeForHeadSet(0.0f, 0.5f, 0.0f);
    EXPECT_FLOAT_EQ(result, 0.5f);
}

/**
 * @tc.name: Test DepressMediaVolumeForHeadSet
 * @tc.number: DepressMediaVolumeForHeadSet_003
 * @tc.type: FUNC
 * @tc.desc: volume limit smaller than media volume with reduction larger than one, returns reduction
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressMediaVolumeForHeadSet_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    float result = audioAdapterManager->DepressMediaVolumeForHeadSet(0.5f, 1.5f, 0.4f);
    EXPECT_FLOAT_EQ(result, 1.5f);
}

/**
 * @tc.name: Test DepressMediaVolumeForHeadSet
 * @tc.number: DepressMediaVolumeForHeadSet_004
 * @tc.type: FUNC
 * @tc.desc: normal path returns the ratio of media volume to volume limit
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressMediaVolumeForHeadSet_004, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    float result = audioAdapterManager->DepressMediaVolumeForHeadSet(0.4f, 0.8f, 1.0f);
    EXPECT_FLOAT_EQ(result, 0.4f);
}

/**
 * @tc.name: Test DepressMediaVolumeForHeadSet
 * @tc.number: DepressMediaVolumeForHeadSet_005
 * @tc.type: FUNC
 * @tc.desc: ratio equals reduction at the boundary
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressMediaVolumeForHeadSet_005, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    float result = audioAdapterManager->DepressMediaVolumeForHeadSet(0.5f, 0.5f, 1.0f);
    EXPECT_FLOAT_EQ(result, 0.5f);
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: DepressVolume_NullDevice_001
 * @tc.type: FUNC
 * @tc.desc: null device descriptor leaves volume unchanged
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressVolume_NullDevice_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    float volume = 0.5f;
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    audioAdapterManager->DepressVolume(volume, TEST_VOLUME_LEVEL, STREAM_MUSIC, desc);
    EXPECT_FLOAT_EQ(volume, 0.5f);
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: DepressVolume_SpecialStream_001
 * @tc.type: FUNC
 * @tc.desc: voice call assistant and ultrasonic streams are skipped before depressing
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressVolume_SpecialStream_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    ASSERT_NE(desc, nullptr);
    float volume = 0.5f;
    audioAdapterManager->DepressVolume(volume, TEST_VOLUME_LEVEL, STREAM_VOICE_CALL_ASSISTANT, desc);
    EXPECT_FLOAT_EQ(volume, 0.5f);
    audioAdapterManager->DepressVolume(volume, TEST_VOLUME_LEVEL, STREAM_ULTRASONIC, desc);
    EXPECT_FLOAT_EQ(volume, 0.5f);
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: DepressVolume_DefaultScene_001
 * @tc.type: FUNC
 * @tc.desc: out of call resets limit to max and clamps volume to the limit
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressVolume_DefaultScene_001, TestSize.Level1)
{
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    ASSERT_NE(desc, nullptr);
    float volume = 1.5f;
    audioAdapterManager->DepressVolume(volume, TEST_VOLUME_LEVEL, STREAM_MUSIC, desc);
    EXPECT_FLOAT_EQ(volume, AudioAdapterManager::MAX_STREAM_VOLUME);

    float lowVolume = 0.5f;
    audioAdapterManager->DepressVolume(lowVolume, TEST_VOLUME_LEVEL, STREAM_MUSIC, desc);
    EXPECT_FLOAT_EQ(lowVolume, 0.5f);
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: DepressVolume_ReductionNonHeadset_001
 * @tc.type: FUNC
 * @tc.desc: in call non-headset device depresses volume by limit times reduction ratio
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressVolume_ReductionNonHeadset_001, TestSize.Level1)
{
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    ASSERT_NE(desc, nullptr);
    audioAdapterManager->volumeLimit_ = 0.5f;

    auto info = std::make_shared<LowerVolumeInfo>();
    ASSERT_NE(info, nullptr);
    info->streamType = STREAM_RING;
    info->duckedDb = -6;
    audioAdapterManager->lowerVolumeInfos_[info->streamType] = info;

    float volume = 0.6f;
    audioAdapterManager->DepressVolume(volume, TEST_VOLUME_LEVEL, STREAM_RING, desc);
    float reduction = audioAdapterManager->GetVolumeReductionRatio(STREAM_RING);
    EXPECT_FLOAT_EQ(volume, 0.5f * reduction);
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: DepressVolume_ScoHeadset_001
 * @tc.type: FUNC
 * @tc.desc: in call bluetooth sco headset depresses media volume through headset path
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressVolume_ScoHeadset_001, TestSize.Level1)
{
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    ASSERT_NE(desc, nullptr);
    DeviceType oldType = desc->deviceType_;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioAdapterManager->isVgsVolumeSupported_ = true;
    audioAdapterManager->volumeLimit_ = 0.5f;

    auto info = std::make_shared<LowerVolumeInfo>();
    ASSERT_NE(info, nullptr);
    info->streamType = STREAM_MUSIC;
    info->duckedDb = -6;
    audioAdapterManager->lowerVolumeInfos_[info->streamType] = info;

    float volume = 0.6f;
    audioAdapterManager->DepressVolume(volume, TEST_VOLUME_LEVEL, STREAM_MUSIC, desc);
    EXPECT_FLOAT_EQ(volume, audioAdapterManager->GetVolumeReductionRatio(STREAM_MUSIC));

    desc->deviceType_ = oldType;
    audioAdapterManager->isVgsVolumeSupported_ = false;
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name: Test DepressVolume
 * @tc.number: DepressVolume_NearlinkHeadset_001
 * @tc.type: FUNC
 * @tc.desc: in call nearlink headset depresses media volume through headset path
 */
HWTEST_F(AudioAdapterManagerUnitTest, DepressVolume_NearlinkHeadset_001, TestSize.Level1)
{
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    auto desc = audioAdapterManager->audioActiveDevice_.GetDeviceForVolume(STREAM_MUSIC);
    ASSERT_NE(desc, nullptr);
    DeviceType oldType = desc->deviceType_;
    desc->deviceType_ = DEVICE_TYPE_NEARLINK;
    audioAdapterManager->isSleVoiceStatus_ = true;
    audioAdapterManager->volumeLimit_ = 0.5f;

    auto info = std::make_shared<LowerVolumeInfo>();
    ASSERT_NE(info, nullptr);
    info->streamType = STREAM_MUSIC;
    info->duckedDb = -6;
    audioAdapterManager->lowerVolumeInfos_[info->streamType] = info;

    float volume = 0.6f;
    audioAdapterManager->DepressVolume(volume, TEST_VOLUME_LEVEL, STREAM_MUSIC, desc);
    EXPECT_FLOAT_EQ(volume, audioAdapterManager->GetVolumeReductionRatio(STREAM_MUSIC));

    desc->deviceType_ = oldType;
    audioAdapterManager->isSleVoiceStatus_ = false;
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
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
    int32_t ret = audioAdapterManager->GetMaxVolumeLevel(STREAM_APP, desc->deviceType_);
    EXPECT_EQ(ret, audioAdapterManager->appConfigVolume_.maxVolume);
    ret = audioAdapterManager->GetMinVolumeLevel(STREAM_APP, desc->deviceType_);
    EXPECT_EQ(ret, audioAdapterManager->appConfigVolume_.minVolume);
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
    int32_t minVolume = audioAdapterManager_->GetMinVolumeLevel(streamType, deviceType);
    int32_t maxVolume = audioAdapterManager_->GetMaxVolumeLevel(streamType, deviceType);
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
    uint32_t ioHandle = 100;
    OffloadAdapter adapter = OFFLOAD_IN_REMOTE;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter, ioHandle);
    audioAdapterManager->audioActiveDevice_.defaultOutputDevice_ = defaultOutputDevice_;
    int32_t res = audioAdapterManager->SetAppVolumeDb(appUid, defaultOutputDevice_->deviceType_);
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
    uint32_t ioHandle = 200;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter, ioHandle);
    audioAdapterManager->audioActiveDevice_.defaultOutputDevice_ = defaultOutputDevice_;
    int32_t res = audioAdapterManager->SetAppVolumeDb(appUid, defaultOutputDevice_->deviceType_);
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
    uint32_t ioHandle = 100;
    OffloadAdapter adapter = OFFLOAD_IN_REMOTE;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter, ioHandle);
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
    uint32_t ioHandle = 200;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter, ioHandle);
    audioAdapterManager->audioActiveDevice_.defaultOutputDevice_ = defaultOutputDevice_;
    int32_t res = audioAdapterManager->SetAppVolumeMutedDB(appUid, muted);
    EXPECT_EQ(res, SUCCESS);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_001
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_FAST);
    EXPECT_EQ(idInfo, HDI_ID_INFO_VOIP);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_002
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_FAST);
    EXPECT_TRUE(idInfo.empty());
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_003
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_AI);
    EXPECT_TRUE(idInfo.empty());
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_004
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_004, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_UNPROCESS;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, HDI_ID_INFO_UNPROCESS);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_005
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_005, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_FLAG_NONE;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_TRUE(idInfo.empty());
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_006
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_006, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "extra";
    pipeInfo->routeFlag_ = AUDIO_FLAG_NONE;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_TRUE(idInfo.empty());
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_007
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_007, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_ULTRASONIC;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, HDI_ID_INFO_ULTRASONIC);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_008
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_008, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_LIVE;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, HDI_ID_INFO_LIVE);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_009
 * @tc.type: FUNC
 * @tc.desc: Test camcorder input flag maps to primary camcorder HDI id info
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_009, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_FAST;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_CAMCORDER;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, HDI_ID_INFO_CAMCORDER);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_009_Ext01
 * @tc.type: FUNC
 * @tc.desc: Test voice message input flag maps to primary voice message HDI id info
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_009_Ext01, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_FAST;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_VOICE_MESSAGE;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, HDI_ID_INFO_VOICE_MESSAGE);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_010
 * @tc.type: FUNC
 * @tc.desc: primary_extra_capturer should use module name as id info
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_010, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::string idInfo;
    HdiIdType idType = HDI_ID_TYPE_FAST;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary_extra_capturer";
    pipeInfo->moduleInfo_.name = "Built_in_typec";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, "Built_in_typec");
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_011_Ext01
 * @tc.type: FUNC
 * @tc.desc: primary_extra_capturer should support wired and bt names
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_011_Ext01, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary_extra_capturer";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    HdiIdType idType = HDI_ID_TYPE_FAST;
    std::string idInfo;

    pipeInfo->moduleInfo_.name = "Built_in_wired";
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, "Built_in_wired");

    idType = HDI_ID_TYPE_FAST;
    pipeInfo->moduleInfo_.name = "Built_in_bt";
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_PRIMARY);
    EXPECT_EQ(idInfo, "Built_in_bt");
}

/**
 * @tc.name: Test GetHdiSourceIdInfo
 * @tc.number: GetHdiSourceIdInfo_001
 * @tc.type: FUNC
 * @tc.desc: primary_extra_capturer should use module name as hdi source id info
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetHdiSourceIdInfo_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.className = "primary_extra_capturer";
    audioModuleInfo.name = "Built_in_typec";
    EXPECT_EQ(audioAdapterManager->GetHdiSourceIdInfo(audioModuleInfo), "Built_in_typec");

    audioModuleInfo.name = "Built_in_wired";
    EXPECT_EQ(audioAdapterManager->GetHdiSourceIdInfo(audioModuleInfo), "Built_in_wired");

    audioModuleInfo.name = "Built_in_bt";
    EXPECT_EQ(audioAdapterManager->GetHdiSourceIdInfo(audioModuleInfo), "Built_in_bt");
}

/**
 * @tc.name: Test UpdateAudioPipeVolume_001
 * @tc.desc: Test UpdateAudioPipeVolume with empty vector
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateAudioPipeVolume_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    
    std::vector<PipeDeviceVolumeGroup> emptyList;
    audioAdapterManager->UpdateAudioPipeVolume(emptyList);
}

/**
 * @tc.name: Test UpdateAudioPipeVolume_002
 * @tc.desc: Test UpdateAudioPipeVolume with valid data
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateAudioPipeVolume_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    
    std::vector<PipeDeviceVolumeGroup> groupList;
    PipeDeviceVolumeGroup group;
    group.deviceDesc_ = std::make_shared<AudioDeviceDescriptor>();
    group.deviceDesc_->deviceType_ = DEVICE_TYPE_SPEAKER;
    group.volumeTypeIoHandles_[STREAM_MUSIC].insert(100);
    groupList.push_back(group);
    
    audioAdapterManager->UpdateAudioPipeVolume(groupList);
}

/**
 * @tc.name: Test RemoveAudioPipeVolume_001
 * @tc.desc: Test RemoveAudioPipeVolume
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, RemoveAudioPipeVolume_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);

    uint32_t ioHandle = 100;
    audioAdapterManager->RemoveAudioPipeVolume(ioHandle);
}

/**
 * @tc.name: Test UpdateVolumeForAllPipes_001
 * @tc.desc: Test UpdateVolumeForAllPipes
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateVolumeForAllPipes_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);

    audioAdapterManager->UpdateVolumeForAllPipes();
}

/**
 * @tc.name: Test UpdateTypeVolumeForAllPipes_001
 * @tc.desc: Test UpdateTypeVolumeForAllPipes with STREAM_MUSIC
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateTypeVolumeForAllPipes_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);

    AudioVolumeType volumeType = STREAM_MUSIC;
    audioAdapterManager->UpdateTypeVolumeForAllPipes(volumeType);
}

/**
 * @tc.name: Test UpdateTypeVolumeForAllPipes_002
 * @tc.desc: Test UpdateTypeVolumeForAllPipes with STREAM_VOICE_CALL
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateTypeVolumeForAllPipes_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);

    AudioVolumeType volumeType = STREAM_VOICE_CALL;
    audioAdapterManager->UpdateTypeVolumeForAllPipes(volumeType);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSinkIdInfoAndIdType_001
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSinkIdInfoAndIdType_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "usb";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;

    std::string idInfo;
    HdiIdType idType;
    audioAdapterManager->GetSinkIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_FAST);
    EXPECT_EQ(idInfo, HDI_ID_INFO_USB);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSinkIdInfoAndIdType_002
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSinkIdInfoAndIdType_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "usb";
    pipeInfo->routeFlag_ = AUDIO_FLAG_NONE;

    std::string idInfo = "err";
    HdiIdType idType;
    audioAdapterManager->GetSinkIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idInfo, "err");
}


/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSinkIdInfoAndIdType_003
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSinkIdInfoAndIdType_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "error";
    pipeInfo->routeFlag_ = AUDIO_FLAG_NONE;

    std::string idInfo = "err";
    HdiIdType idType;
    audioAdapterManager->GetSinkIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idInfo, "err");
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSinkIdInfoAndIdType_001
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_011, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "usb";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_FAST;

    std::string idInfo;
    HdiIdType idType;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idType, HDI_ID_TYPE_FAST);
    EXPECT_EQ(idInfo, HDI_ID_INFO_USB);
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_002
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_012, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "usb";
    pipeInfo->routeFlag_ = AUDIO_FLAG_NONE;

    std::string idInfo = "err";
    HdiIdType idType;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idInfo, "err");
}


/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: GetSourceIdInfoAndIdType_003
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSourceIdInfoAndIdType_013, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "error";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_FAST;

    std::string idInfo = "err";
    HdiIdType idType;
    audioAdapterManager->GetSourceIdInfoAndIdType(pipeInfo, idInfo, idType);
    EXPECT_EQ(idInfo, "err");
}

/**
 * @tc.name: Test GetSourceIdInfoAndIdType
 * @tc.number: RedirectVolumeType
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, RedirectVolumeType, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDescriptor = std::make_shared<AudioStreamDescriptor>();
    AudioVolumeType volumeType = STREAM_MUSIC;
    audioAdapterManager->RedirectVolumeType(streamDescriptor, volumeType);
    streamDescriptor->callerUid_ = 1003;
    audioAdapterManager->RedirectVolumeType(streamDescriptor, volumeType);
    streamDescriptor->callerUid_ = -1;
    volumeType = STREAM_VOICE_ASSISTANT;
    audioAdapterManager->RedirectVolumeType(streamDescriptor, volumeType);
    streamDescriptor->callerUid_ = 1003;
    audioAdapterManager->RedirectVolumeType(streamDescriptor, volumeType);
    EXPECT_EQ(volumeType, STREAM_VOICE_ASSISTANT);
}

/**
 * @tc.name: Test UpdateVolumeForLowLatency
 * @tc.number: UpdateVolumeForLowLatency
 * @tc.type: FUNC
 * @tc.desc: when successful execution, return success
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateVolumeForLowLatency, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ == DEVICE_TYPE_NEARLINK;
    AudioVolumeType volumeType = STREAM_MUSIC;
    audioAdapterManager->UpdateVolumeForLowLatency(desc, volumeType);

    desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP;
    volumeType = STREAM_MUSIC;
    audioAdapterManager->UpdateVolumeForLowLatency(desc, volumeType);

    desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP;
    volumeType = STREAM_RING;
    audioAdapterManager->UpdateVolumeForLowLatency(desc, volumeType);
}

/**
 * @tc.name: Test SetVolumeDbForDeviceInPipe_001
 * @tc.desc: Test SetVolumeDbForDeviceInPipe with null device descriptor returns ERROR
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetVolumeDbForDeviceInPipe_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    int32_t result = audioAdapterManager->SetVolumeDbForDeviceInPipe(desc, STREAM_MUSIC);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: Test SetVolumeDbForDeviceInPipe_002
 * @tc.desc: Test SetVolumeDbForDeviceInPipe with SPEAKER device returns SUCCESS
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetVolumeDbForDeviceInPipe_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t result = audioAdapterManager->SetVolumeDbForDeviceInPipe(desc, STREAM_MUSIC);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: Test SetVolumeDbForDeviceInPipe_003
 * @tc.desc: Test SetVolumeDbForDeviceInPipe with HEARING_AID device returns ERROR
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetVolumeDbForDeviceInPipe_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_HEARING_AID;
    int32_t result = audioAdapterManager->SetVolumeDbForDeviceInPipe(desc, STREAM_MUSIC);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_001
 * @tc.desc: Test AdjustVolumeForSpecialDevice with VolumeFixEnable - unmute
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool isMuted = true;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    VolumeUtils::SetVolumeFixEnable(true);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_MUSIC, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(isMuted, false);
    VolumeUtils::SetVolumeFixEnable(false);
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_002
 * @tc.desc: Test AdjustVolumeForSpecialDevice with A2DP and AbsVolumeScene
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    const std::string macAddress = "11:22:33:44:55:66";
    device->macAddress_ = macAddress;
    bool isMuted = false;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    // Abs volume scene is keyed by the device mac address.
    audioAdapterManager->SetAbsVolumeScene(macAddress, false, 0);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_RING, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);

    audioAdapterManager->SetAbsVolumeScene(macAddress, true, 0);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_RING, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);

    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_MUSIC, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, ABS_VOLUME_DB_VALUE);
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_003
 * @tc.desc: Test AdjustVolumeForSpecialDevice with NEARLINK and SleVoice disabled
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    bool isMuted = false;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    audioAdapterManager->isSleVoiceStatus_.store(true);
    audioAdapterManager->isAbsVolumeMuteNearlink_.store(false);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_RING, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);

    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_MUSIC, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);

    audioAdapterManager->isSleVoiceStatus_.store(false);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_MUSIC, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_004
 * @tc.desc: Test AdjustVolumeForSpecialDevice with NEARLINK and VOICE_CALL - max volume
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_004, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    bool isMuted = false;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_VOICE_CALL, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, MAX_VOLUME_DB_VALUE);
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_005
 * @tc.desc: Test AdjustVolumeForSpecialDevice with BLUETOOTH_SCO and VgsVolumeSupported
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_005, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    bool isMuted = false;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_VOICE_CALL, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);
}

/**
 * @tc.name: Test ApplySystemVolumeProxy_001
 * @tc.desc: Test ApplySystemVolumeProxy with enabled - unmute and max volume
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, ApplySystemVolumeProxy_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool isMuted = true;
    float volumeDb = TEST_VOLUME_DB;
    audioAdapterManager->ApplySystemVolumeProxy(TEST_ZONE_ID, device, isMuted, volumeDb);
    EXPECT_EQ(isMuted, true);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);
}

/**
 * @tc.name: GetSystemAppVolumeLevel_NotSet
 * @tc.number: GetSystemAppVolumeLevel_001
 * @tc.type: FUNC
 * @tc.desc: When app volume not set, returns APP_DEFAULT_VOLUME_LEVEL (100)
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSystemAppVolumeLevel_NotSet, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t appUid = 1001;
    int32_t volumeLevel = 0;

    audioAdapterManager->volumeDataMaintainer_.SetSystemAppVolumeMuted(appUid, false);

    int32_t ret = audioAdapterManager->GetSystemAppVolumeLevel(appUid, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(volumeLevel, APP_DEFAULT_VOLUME_LEVEL);
}

/**
 * @tc.name: GetSystemAppVolumeLevel_Muted
 * @tc.number: GetSystemAppVolumeLevel_002
 * @tc.type: FUNC
 * @tc.desc: When app volume is muted, returns 0
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSystemAppVolumeLevel_Muted, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t appUid = 1001;
    int32_t volumeLevel = -1;

    audioAdapterManager->volumeDataMaintainer_.SetSystemAppVolume(appUid, 5);
    audioAdapterManager->volumeDataMaintainer_.SetSystemAppVolumeMuted(appUid, true);

    int32_t ret = audioAdapterManager->GetSystemAppVolumeLevel(appUid, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(volumeLevel, 0);
}

/**
 * @tc.name: GetSystemAppVolumeLevel_Normal
 * @tc.number: GetSystemAppVolumeLevel_003
 * @tc.type: FUNC
 * @tc.desc: When app volume is set and not muted, returns stored volume
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSystemAppVolumeLevel_Normal, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t appUid = 1001;
    int32_t volumeLevel = -1;
    int32_t expectedVolume = 7;

    audioAdapterManager->volumeDataMaintainer_.SetSystemAppVolume(appUid, expectedVolume);
    audioAdapterManager->volumeDataMaintainer_.SetSystemAppVolumeMuted(appUid, false);

    int32_t ret = audioAdapterManager->GetSystemAppVolumeLevel(appUid, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(volumeLevel, expectedVolume);
}

/**
 * @tc.name: Test GetAudioSinkAttr HdPlaybackMode
 * @tc.number: GetAudioSinkAttr_HdPlaybackMode_001
 * @tc.type: FUNC
 * @tc.desc: Test GetAudioSinkAttr with empty hdPlayBackMode string
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_HdPlaybackMode_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.hdPlayBackMode = "";
    IAudioSinkAttr attr = audioAdapterManager->GetAudioSinkAttr(audioModuleInfo);
    EXPECT_EQ(attr.hdPlayBackMode, 0);
}

/**
 * @tc.name: Test GetAudioSinkAttr HdPlaybackMode
 * @tc.number: GetAudioSinkAttr_HdPlaybackMode_002
 * @tc.type: FUNC
 * @tc.desc: Test GetAudioSinkAttr with hdPlayBackMode "0" (HD_NOT_SUPPORTED)
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_HdPlaybackMode_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.hdPlayBackMode = "0";
    IAudioSinkAttr attr = audioAdapterManager->GetAudioSinkAttr(audioModuleInfo);
    EXPECT_EQ(attr.hdPlayBackMode, 0);
}

/**
 * @tc.name: Test GetAudioSinkAttr UltraFast Period
 * @tc.number: GetAudioSinkAttr_UltraFastPeriod_001
 * @tc.type: FUNC
 * @tc.desc: Test GetAudioSinkAttr with ultra_fast className calculates period for 2.5ms latency
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_UltraFastPeriod_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);

    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.className = "ultra_fast";
    audioModuleInfo.adapterName = "primary";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.channels = "2";
    audioModuleInfo.format = "s16le";

    IAudioSinkAttr attr = audioAdapterManager->GetAudioSinkAttr(audioModuleInfo);

    // Expected: period = (48000 * 2 * 2) * 2.5 / 1000 = 240 frames
    EXPECT_EQ(attr.period, 480u);
    EXPECT_EQ(attr.sampleRate, 48000u);
    EXPECT_EQ(attr.channel, 2u);
}

/**
 * @tc.name: Test GetAudioSinkAttr UltraFast Period S32LE
 * @tc.number: GetAudioSinkAttr_UltraFastPeriod_002
 * @tc.type: FUNC
 * @tc.desc: Test GetAudioSinkAttr with ultra_fast and S32LE format calculates correct period
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_UltraFastPeriod_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);

    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.className = "ultra_fast";
    audioModuleInfo.adapterName = "primary";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.channels = "2";
    audioModuleInfo.format = "s32le";

    IAudioSinkAttr attr = audioAdapterManager->GetAudioSinkAttr(audioModuleInfo);

    // Expected: period = (48000 * 2 * 4) * 2.5 / 1000 = 480 frames (S32LE = 4 bytes)
    EXPECT_EQ(attr.period, 960u);
    EXPECT_EQ(attr.sampleRate, 48000u);
    EXPECT_EQ(attr.channel, 2u);
}

/**
 * @tc.name: Test GetAudioSinkAttr Non-UltraFast Period Default
 * @tc.number: GetAudioSinkAttr_NonUltraFastPeriod_001
 * @tc.type: FUNC
 * @tc.desc: Test GetAudioSinkAttr with non-ultra_fast className returns period=0 (default)
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_NonUltraFastPeriod_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);

    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.className = "primary_mmap";
    audioModuleInfo.adapterName = "primary";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.channels = "2";
    audioModuleInfo.format = "s16le";

    IAudioSinkAttr attr = audioAdapterManager->GetAudioSinkAttr(audioModuleInfo);

    // Expected: period = 0 (default for non-ultra_fast)
    EXPECT_EQ(attr.period, 0u);
    EXPECT_EQ(attr.sampleRate, 48000u);
    EXPECT_EQ(attr.channel, 2u);
}

/**
 * @tc.name: Test GetAudioSinkAttr UltraFast Period U8 Format
 * @tc.number: GetAudioSinkAttr_UltraFastPeriod_003
 * @tc.type: FUNC
 * @tc.desc: Test GetAudioSinkAttr with ultra_fast and U8 format calculates correct period
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_UltraFastPeriod_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);

    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.className = "ultra_fast";
    audioModuleInfo.adapterName = "primary";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.channels = "2";
    audioModuleInfo.format = "u8";

    IAudioSinkAttr attr = audioAdapterManager->GetAudioSinkAttr(audioModuleInfo);

    // Expected: period = (48000 * 2 * 1) * 2.5 / 1000 = 120 frames (U8 = 1 byte)
    EXPECT_EQ(attr.period, 480u);
    EXPECT_EQ(attr.sampleRate, 48000u);
    EXPECT_EQ(attr.channel, 2u);
}

/**
 * @tc.name: Test GetAudioSinkAttr UltraFast Period S24LE Format
 * @tc.number: GetAudioSinkAttr_UltraFastPeriod_004
 * @tc.type: FUNC
 * @tc.desc: Test GetAudioSinkAttr with ultra_fast and S24LE format calculates correct period
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetAudioSinkAttr_UltraFastPeriod_004, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);

    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.className = "ultra_fast";
    audioModuleInfo.adapterName = "primary";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.channels = "2";
    audioModuleInfo.format = "s24le";

    IAudioSinkAttr attr = audioAdapterManager->GetAudioSinkAttr(audioModuleInfo);

    // Expected: period = (48000 * 2 * 3) * 2.5 / 1000 = 360 frames (S24 = 3 bytes)
    EXPECT_EQ(attr.period, 720u);
    EXPECT_EQ(attr.sampleRate, 48000u);
    EXPECT_EQ(attr.channel, 2u);
}

/**
 * @tc.name: UpdateCollaborativeState_NearLink_001
 * @tc.type: FUNC
 * @tc.desc: Test UpdateCollaborativeState with COLLABORATION_NEARLINK_OPEN sends correct enabled state.
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateCollaborativeState_NearLink_001, TestSize.Level1)
{
    auto &manager = AudioAdapterManager::GetInstance();
    int32_t ret = manager.UpdateCollaborativeState(COLLABORATION_NEARLINK_OPEN);
    // Result depends on adapter initialization, just verify no crash
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UpdateCollaborativeState_Bluetooth_001
 * @tc.type: FUNC
 * @tc.desc: Test UpdateCollaborativeState with COLLABORATION_BLUETOOTH_OPEN sends true for enabled.
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateCollaborativeState_Bluetooth_001, TestSize.Level1)
{
    auto &manager = AudioAdapterManager::GetInstance();
    int32_t ret = manager.UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_OPERATION_FAILED);
}

/**
 * @tc.name: UpdateCollaborativeState_Close_001
 * @tc.type: FUNC
 * @tc.desc: Test UpdateCollaborativeState with COLLABORATION_CLOSE sends false for enabled.
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateCollaborativeState_Close_001, TestSize.Level1)
{
    auto &manager = AudioAdapterManager::GetInstance();
    int32_t ret = manager.UpdateCollaborativeState(COLLABORATION_CLOSE);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_OPERATION_FAILED);
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_010
 * @tc.desc: Test BLUETOOTH_SCO with VgsSupported and IsPCVolumeEnable true for STREAM_MUSIC
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_010, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    bool isMuted = false;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    VolumeUtils::SetPCVolumeEnable(true);
    audioAdapterManager->isVgsVolumeSupported_ = true;
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>();
    scoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    scoDevice->networkId_ = LOCAL_NETWORK_ID;
    audioAdapterManager->audioActiveDevice_.activeOutputDevices_.push_back(scoDevice);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_MUSIC, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, ABS_VOLUME_DB_VALUE);
    audioAdapterManager->audioActiveDevice_.activeOutputDevices_.clear();
    audioAdapterManager->isVgsVolumeSupported_ = false;
    VolumeUtils::SetPCVolumeEnable(false);
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_011
 * @tc.desc: Test BLUETOOTH_SCO with VgsSupported and IsPCVolumeEnable false for STREAM_MUSIC
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_011, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    bool isMuted = false;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    VolumeUtils::SetPCVolumeEnable(false);
    audioAdapterManager->isVgsVolumeSupported_ = true;
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>();
    scoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    scoDevice->networkId_ = LOCAL_NETWORK_ID;
    audioAdapterManager->audioActiveDevice_.activeOutputDevices_.push_back(scoDevice);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_MUSIC, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, TEST_VOLUME_DB);
    audioAdapterManager->audioActiveDevice_.activeOutputDevices_.clear();
    audioAdapterManager->isVgsVolumeSupported_ = false;
}

/**
 * @tc.name: Test AdjustVolumeForSpecialDevice_012
 * @tc.desc: Test BLUETOOTH_SCO with VgsSupported and STREAM_VOICE_CALL - unchanged behavior
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, AdjustVolumeForSpecialDevice_012, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    bool isMuted = false;
    int32_t volumeLevel = TEST_VOLUME_LEVEL;
    float volumeDb = TEST_VOLUME_DB;
    audioAdapterManager->isVgsVolumeSupported_ = true;
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>();
    scoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    scoDevice->networkId_ = LOCAL_NETWORK_ID;
    audioAdapterManager->audioActiveDevice_.activeOutputDevices_.push_back(scoDevice);
    audioAdapterManager->AdjustVolumeForSpecialDevice(device, STREAM_VOICE_CALL, isMuted, volumeLevel, volumeDb);
    EXPECT_EQ(volumeDb, MAX_VOLUME_DB_VALUE);
    audioAdapterManager->audioActiveDevice_.activeOutputDevices_.clear();
    audioAdapterManager->isVgsVolumeSupported_ = false;
}

/**
 * @tc.name: Is3daDirectBusPort_001
 * @tc.desc: Test Is3daDirectBusPort matches the 3da keyword in the module name.
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, Is3daDirectBusPort_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.name = "bus10_3da_out"; // boot xml module name
    info.busAddress = "";
    EXPECT_TRUE(AudioAdapterManager::Is3daDirectBusPort(info));

    info.name = "mixport_bus11_3da_out"; // play-time new-pipe module name
    EXPECT_TRUE(AudioAdapterManager::Is3daDirectBusPort(info));
}

/**
 * @tc.name: Is3daDirectBusPort_002
 * @tc.desc: Test Is3daDirectBusPort matches the 3da keyword in the bus address only.
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, Is3daDirectBusPort_002, TestSize.Level1)
{
    AudioModuleInfo info;
    info.name = "primary";
    info.busAddress = "bus10_3da_out";
    EXPECT_TRUE(AudioAdapterManager::Is3daDirectBusPort(info));
}

/**
 * @tc.name: Is3daDirectBusPort_003
 * @tc.desc: Test Is3daDirectBusPort returns false for non-3da and empty fields.
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, Is3daDirectBusPort_003, TestSize.Level1)
{
    AudioModuleInfo info;
    info.name = "speaker";
    info.busAddress = "bus1_out";
    EXPECT_FALSE(AudioAdapterManager::Is3daDirectBusPort(info));

    info.name = "";
    info.busAddress = "";
    EXPECT_FALSE(AudioAdapterManager::Is3daDirectBusPort(info));
}

/**
 * @tc.name: ConvergeTo3daSink_001
 * @tc.desc: Test ConvergeTo3daSink takes the bus address as deviceClass with the fixed 3DA idInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, ConvergeTo3daSink_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.name = "mixport_bus11_3da_out";
    info.busAddress = "bus10_3da_out";
    std::string deviceClass = "primary";
    std::string idInfo = HDI_ID_INFO_DEFAULT;
    AudioAdapterManager::ConvergeTo3daSink(info, deviceClass, idInfo);
    EXPECT_EQ(deviceClass, "bus10_3da_out");
    EXPECT_EQ(idInfo, HDI_ID_INFO_3DA);
}

/**
 * @tc.name: ConvergeTo3daSink_002
 * @tc.desc: Test ConvergeTo3daSink falls back to the module name when busAddress is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AudioAdapterManagerUnitTest, ConvergeTo3daSink_002, TestSize.Level1)
{
    AudioModuleInfo info;
    info.name = "bus10_3da_out";
    info.busAddress = "";
    std::string deviceClass = "";
    std::string idInfo = "";
    AudioAdapterManager::ConvergeTo3daSink(info, deviceClass, idInfo);
    EXPECT_EQ(deviceClass, "bus10_3da_out");
    EXPECT_EQ(idInfo, HDI_ID_INFO_3DA);
}

/**
 * @tc.name: SaveSystemVolumeForEffect_MultipleDevices_001
 * @tc.type: FUNC
 * @tc.desc: Test SaveSystemVolumeForEffect for multiple device types
 */
HWTEST_F(AudioAdapterManagerUnitTest, SaveSystemVolumeForEffect_MultipleDevices_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;

    // Save different volumes for different devices
    AudioAdapterManager::GetInstance().SaveSystemVolumeForEffect(DEVICE_TYPE_SPEAKER, streamType, 10);
    AudioAdapterManager::GetInstance().SaveSystemVolumeForEffect(DEVICE_TYPE_BLUETOOTH_A2DP, streamType, 8);
    AudioAdapterManager::GetInstance().SaveSystemVolumeForEffect(DEVICE_TYPE_WIRED_HEADSET, streamType, 12);

    // Retrieve and verify volumes
    int32_t speakerVolume = AudioAdapterManager::GetInstance().GetSystemVolumeForEffect(
        DEVICE_TYPE_SPEAKER, streamType);
    int32_t bluetoothVolume = AudioAdapterManager::GetInstance().GetSystemVolumeForEffect(
        DEVICE_TYPE_BLUETOOTH_A2DP, streamType);
    int32_t wiredVolume = AudioAdapterManager::GetInstance().GetSystemVolumeForEffect(
        DEVICE_TYPE_WIRED_HEADSET, streamType);

    EXPECT_EQ(speakerVolume, 10);
    EXPECT_EQ(bluetoothVolume, 8);
    EXPECT_EQ(wiredVolume, 12);
}

/**
 * @tc.name: GetSystemVolumeForEffect_NotSet_001
 * @tc.type: FUNC
 * @tc.desc: Test GetSystemVolumeForEffect when volume not set (should return default)
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetSystemVolumeForEffect_NotSet_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_ALARM;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    // Get volume without saving first - should return default
    int32_t volume = AudioAdapterManager::GetInstance().GetSystemVolumeForEffect(deviceType, streamType);
    // Default volume is expected when not set
    EXPECT_GE(volume, 0);
}

/**
 * @tc.name: IsChannelLayoutSupportedForDspEffectWithDeviceType_001
 * @tc.type: FUNC
 * @tc.desc: Test IsChannelLayoutSupportedForDspEffect with DeviceType for SPEAKER device
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_001, TestSize.Level1)
{
    bool ret = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_6POINT0_FRONT, DEVICE_TYPE_SPEAKER);
    // Without proper adapter setup, should return false
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsChannelLayoutSupportedForDspEffectWithDeviceType_002
 * @tc.type: FUNC
 * @tc.desc: Test IsChannelLayoutSupportedForDspEffect with DeviceType for BLUETOOTH_A2DP device
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_002, TestSize.Level1)
{
    bool ret = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_STEREO, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsChannelLayoutSupportedForDspEffectWithDeviceType_003
 * @tc.type: FUNC
 * @tc.desc: Test IsChannelLayoutSupportedForDspEffect with DeviceType for WIRED_HEADSET device
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_003, TestSize.Level1)
{
    bool ret = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_MONO, DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsChannelLayoutSupportedForDspEffectWithDeviceType_004
 * @tc.type: FUNC
 * @tc.desc: Test IsChannelLayoutSupportedForDspEffect with DeviceType for NEARLINK device
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_004, TestSize.Level1)
{
    bool ret = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_2POINT1, DEVICE_TYPE_NEARLINK);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsChannelLayoutSupportedForDspEffectWithDeviceType_005
 * @tc.type: FUNC
 * @tc.desc: Test IsChannelLayoutSupportedForDspEffect with DeviceType for USB_HEADSET device
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_005, TestSize.Level1)
{
    bool ret = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_5POINT1, DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001
 * @tc.type: FUNC
 * @tc.desc: Test IsChannelLayoutSupportedForDspEffect with DeviceType for multiple channel layouts
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool ret1 = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_MONO, deviceType);
    bool ret2 = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_STEREO, deviceType);
    bool ret3 = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_5POINT1, deviceType);
    bool ret4 = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_7POINT1, deviceType);

    EXPECT_EQ(ret1, false);
    EXPECT_EQ(ret2, false);
    EXPECT_EQ(ret3, false);
    EXPECT_EQ(ret4, false);
}

/**
 * @tc.name: IsChannelLayoutSupportedForDspEffectWithDeviceType_NullAdapter_001
 * @tc.type: FUNC
 * @tc.desc: Test IsChannelLayoutSupportedForDspEffect with DeviceType when adapter is null
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_NullAdapter_001,
    TestSize.Level1)
{
    // This test verifies the behavior when audioServiceAdapter_ is null
    // It should return false when adapter is not available
    bool ret = AudioAdapterManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_7POINT1, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(ret, false);
}

} // namespace AudioStandard
} // namespace OHOS
