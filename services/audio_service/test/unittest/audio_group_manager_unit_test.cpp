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
#include <gtest/gtest.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "system_ability_definition.h"
#include "audio_group_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioGroupManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: SetVolume_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, SetVolume_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.SetVolume(STREAM_VOICE_CALL, 0, 0);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: SetVolume_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, SetVolume_002, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.SetVolume(STREAM_ULTRASONIC, 0, 0);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: SetVolume_003
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, SetVolume_003, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.SetVolume(STREAM_WAKEUP, 0, 0);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetActiveVolumeType_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetActiveVolumeType_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    AudioStreamType ret = audioGroupManager.GetActiveVolumeType(1);
    EXPECT_EQ(ret, STREAM_MUSIC);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetVolume_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetVolume_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.SetVolume(STREAM_MUSIC, 7, 7);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioGroupManager.GetVolume(STREAM_MUSIC);
    EXPECT_EQ(ret, BT_HEADSET_NREC);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetVolume_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetVolume_002, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.GetVolume(STREAM_ULTRASONIC);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetVolume_003
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetVolume_003, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.GetVolume(STREAM_WAKEUP);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetMaxVolume_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetMaxVolume_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = CONNECT_TYPE_DISTRIBUTED;
    int32_t ret = audioGroupManager.GetMaxVolume(STREAM_WAKEUP);
    EXPECT_EQ(ret, D_ALL_DEVICES);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetMaxVolume_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetMaxVolume_002, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.GetMaxVolume(STREAM_ALL);
    EXPECT_EQ(ret, D_ALL_DEVICES);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetMaxVolume_003
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetMaxVolume_003, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.GetMaxVolume(STREAM_ULTRASONIC);
    EXPECT_EQ(ret, D_ALL_DEVICES);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetMinVolume_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetMinVolume_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = CONNECT_TYPE_DISTRIBUTED;
    int32_t ret = audioGroupManager.GetMinVolume(STREAM_WAKEUP);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetMinVolume_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetMinVolume_002, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.GetMinVolume(STREAM_ALL);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: GetMinVolume_003
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, GetMinVolume_003, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    int32_t ret = audioGroupManager.GetMinVolume(STREAM_ULTRASONIC);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: SetMute_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = ConnectType::CONNECT_TYPE_DISTRIBUTED;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_NONE;
    int32_t ret = audioGroupManager.SetMute(AudioStreamType::STREAM_DEFAULT, false, deviceType);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: SetMute_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_002, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = ConnectType::CONNECT_TYPE_LOCAL;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_NONE;
    int32_t ret = audioGroupManager.SetMute(AudioStreamType::STREAM_DEFAULT, false, deviceType);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: SetMute_003
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_003, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = ConnectType::CONNECT_TYPE_LOCAL;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_INVALID;
    int32_t ret = audioGroupManager.SetMute(AudioStreamType::STREAM_DEFAULT, false, deviceType);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: SetMute_004
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_004, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = ConnectType::CONNECT_TYPE_LOCAL;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_INVALID;
    int32_t ret = audioGroupManager.SetMute(AudioStreamType::STREAM_APP, false, deviceType);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: IsStreamMute_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, IsStreamMute_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = ConnectType::CONNECT_TYPE_DISTRIBUTED;
    bool isMute = false;
    int32_t ret = audioGroupManager.IsStreamMute(AudioStreamType::STREAM_CAMCORDER, isMute);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: IsStreamMute_002
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, IsStreamMute_002, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    audioGroupManager.connectType_ = ConnectType::CONNECT_TYPE_LOCAL;
    bool isMute = false;
    int32_t ret = audioGroupManager.IsStreamMute(AudioStreamType::STREAM_APP, isMute);
    EXPECT_EQ(ret, SUCCESS);
}
/**
* @tc.name  : Test Audio API
* @tc.type  : FUNC
* @tc.number: IsAlived_001
* @tc.desc  : Test cross ring cache.
*/
HWTEST(AudioGroupManagerUnitTest, IsAlived_001, TestSize.Level1)
{
    AudioGroupManager audioGroupManager(1);
    bool ret = audioGroupManager.IsAlived();
    EXPECT_EQ(ret, true);
}
} // namespace AudioStandard
} //