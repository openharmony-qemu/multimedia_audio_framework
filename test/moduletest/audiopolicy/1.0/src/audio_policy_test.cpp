/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_system_manager.h"
#include "audio_policy_test.h"

using namespace std;
using namespace OHOS::AudioStandard;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {
void AudioPolicyTest::SetUpTestCase(void)
{
    ASSERT_NE(nullptr, AudioSystemManager::GetInstance());
}

void AudioPolicyTest::TearDownTestCase(void) {}

void AudioPolicyTest::SetUp(void) {}

void AudioPolicyTest::TearDown(void) {}

void AudioRingerModeCallbackTest::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    ringerMode_ = ringerMode;
}

namespace {
const PolicyParam VOLUME_PARAMS[] = {
    {
        .volume = 8,
        .streamType = STREAM_MUSIC
    },
    {
        .volume = 8,
        .streamType = STREAM_RING
    }
};

const PolicyParam MUTE_PARAMS[] = {
    {
        .streamType = STREAM_MUSIC,
        .mute = true
    },
    {
        .streamType = STREAM_MUSIC,
        .mute = false
    }
};

const PolicyParam STREAM_PARAMS[] = {
    {
        .streamType = STREAM_MUSIC,
        .active = true
    },
    {
        .streamType = STREAM_RING,
        .active = false
    }
};

const PolicyParam RINGER_MODE_PARAMS[] = {
    {
        .ringerMode = RINGER_MODE_NORMAL
    },
    {
        .ringerMode = RINGER_MODE_SILENT
    },
    {
        .ringerMode = RINGER_MODE_VIBRATE
    },
};

const PolicyParam MIC_MUTE_PARAMS[] = {
    {
        .mute = true
    },
    {
        .mute = false
    }
};

const PolicyParam VOLUME_RANGE_PARAMS[] = {
    {
        .streamType = STREAM_MUSIC
    },
    {
        .streamType = STREAM_RING
    }
};

const PolicyParam AUDIO_PARAMS[] = {
    {
        .key = "sampling_rate",
        .value = "8000"
    },
    {
        .key = "sampling_rate",
        .value = "44100"
    },
    {
        .key = "sampling_rate",
        .value = "96000"
    }
};

const PolicyParam DEVICES_PARAMS[] = {
    {
        .deviceType = DEVICE_TYPE_MIC,
        .deviceFlag = INPUT_DEVICES_FLAG,
        .deviceRole = INPUT_DEVICE
    },
    {
        .deviceType = DEVICE_TYPE_SPEAKER,
        .deviceFlag = OUTPUT_DEVICES_FLAG,
        .deviceRole = OUTPUT_DEVICE
    }
};

const PolicyParam AUDIO_SCENE_PARAMS[] = {
    {
        .audioScene = AUDIO_SCENE_DEFAULT
    },
    {
        .audioScene = AUDIO_SCENE_PHONE_CHAT
    }
};
} // namespace

/*
 * Set Volume
 *
 */
class AudioPolicySetVolumeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetVolumeTest, SetVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();

    AudioVolumeType volumeType
        = static_cast<AudioVolumeType>(params.streamType);
    float volume = params.volume;
    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetVolume(volumeType, volume));
}

INSTANTIATE_TEST_SUITE_P(
    SetVolume,
    AudioPolicySetVolumeTest,
    ValuesIn(VOLUME_PARAMS));

/*
 * Get Volume
 *
 */
class AudioPolicyGetVolumeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetVolumeTest, GetVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioVolumeType volumeType
        = static_cast<AudioVolumeType>(params.streamType);
    float volume = params.volume;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetVolume(volumeType, volume));
    EXPECT_EQ(volume, AudioSystemManager::GetInstance()->GetVolume(volumeType));
}

INSTANTIATE_TEST_SUITE_P(
    GetVolume,
    AudioPolicyGetVolumeTest,
    ValuesIn(VOLUME_PARAMS));

/*
 * Set Mute
 *
 */
class AudioPolicySetMuteTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetMuteTest, SetMute, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioVolumeType volumeType
        = static_cast<AudioVolumeType>(params.streamType);
    bool mute = params.mute;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetMute(volumeType, mute));
}

INSTANTIATE_TEST_SUITE_P(
    SetMute,
    AudioPolicySetMuteTest,
    ValuesIn(MUTE_PARAMS));

/*
 * Set Ringer Mode
 *
 */
class AudioPolicySetRingerModeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetRingerModeTest, SetRingerMode, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioRingerMode ringerMode = params.ringerMode;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetRingerMode(ringerMode));
}


INSTANTIATE_TEST_SUITE_P(
    SetRingerMode,
    AudioPolicySetRingerModeTest,
    ValuesIn(RINGER_MODE_PARAMS));

/*
 * Get Ringer Mode
 *
 */
class AudioPolicyGetRingerModeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetRingerModeTest, GetRingerMode, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioRingerMode ringerMode = params.ringerMode;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetRingerMode(ringerMode));
    EXPECT_EQ(ringerMode, AudioSystemManager::GetInstance()->GetRingerMode());
}

INSTANTIATE_TEST_SUITE_P(
    GetRingerMode,
    AudioPolicyGetRingerModeTest,
    ValuesIn(RINGER_MODE_PARAMS));

/*
 * Check ringer mode callback
 *
 */
class AudioPolicySetRingerModeCallbackTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetRingerModeCallbackTest, SetRingerModeCallback, TestSize.Level1)
{
    int32_t ret = -1;
    PolicyParam params = GetParam();
    AudioRingerMode ringerMode = params.ringerMode;

    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();

    shared_ptr<AudioRingerModeCallbackTest> ringerModeCB = std::make_shared<AudioRingerModeCallbackTest>();
    ret = audioSystemMgr->SetRingerModeCallback(1, ringerModeCB);
    EXPECT_EQ(SUCCESS, ret);

    audioSystemMgr->SetRingerMode(ringerMode);
    sleep(1);
    EXPECT_EQ(ringerModeCB->ringerMode_, ringerMode);

    ret = audioSystemMgr->UnsetRingerModeCallback(1);
    EXPECT_EQ(SUCCESS, ret);
}

INSTANTIATE_TEST_SUITE_P(
    SetRingerModeCallback,
    AudioPolicySetRingerModeCallbackTest,
    ValuesIn(RINGER_MODE_PARAMS));

/*
 * Set microphone mute
 *
 */
class AudioPolicySetMicrophoneMuteTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetMicrophoneMuteTest, SetMicrophoneMute, TestSize.Level1)
{
    PolicyParam params = GetParam();
    bool mute = params.mute;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetMicrophoneMute(mute));
}

INSTANTIATE_TEST_SUITE_P(
    SetMicrophoneMute,
    AudioPolicySetMicrophoneMuteTest,
    ValuesIn(MIC_MUTE_PARAMS));

/*
 * Is Microphone Mute
 *
 */
class AudioPolicyGetMicrophoneMuteTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetMicrophoneMuteTest, IsMicrophoneMute, TestSize.Level1)
{
    PolicyParam params = GetParam();
    bool mute = params.mute;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetMicrophoneMute(mute));
    EXPECT_EQ(mute, AudioSystemManager::GetInstance()->IsMicrophoneMute());
}

INSTANTIATE_TEST_SUITE_P(
    IsMicrophoneMute,
    AudioPolicyGetMicrophoneMuteTest,
    ValuesIn(MIC_MUTE_PARAMS));

/*
 * Check volume range
 *
 */
class AudioPolicyVolumeRangeTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyVolumeRangeTest, GetMaxVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioVolumeType volumeType
        = static_cast<AudioVolumeType>(params.streamType);
    EXPECT_EQ(15, AudioSystemManager::GetInstance()->GetMaxVolume(volumeType));
}

HWTEST_P(AudioPolicyVolumeRangeTest, GetMinVolume, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioVolumeType volumeType
        = static_cast<AudioVolumeType>(params.streamType);
    EXPECT_EQ(0, AudioSystemManager::GetInstance()->GetMinVolume(volumeType));
}

INSTANTIATE_TEST_SUITE_P(
    GetMaxVolume,
    AudioPolicyVolumeRangeTest,
    ValuesIn(VOLUME_RANGE_PARAMS));

INSTANTIATE_TEST_SUITE_P(
    GetMinVolume,
    AudioPolicyVolumeRangeTest,
    ValuesIn(VOLUME_RANGE_PARAMS));

/*
 * Check volume range
 *
 */
class AudioPolicyAudioParameterTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyAudioParameterTest, SetAudioParameter, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::GetInstance()->SetAudioParameter(params.key, params.value);
    EXPECT_EQ(params.value, AudioSystemManager::GetInstance()->GetAudioParameter(params.key));
}

INSTANTIATE_TEST_SUITE_P(
    SetAudioParameter,
    AudioPolicyAudioParameterTest,
    ValuesIn(AUDIO_PARAMS));

HWTEST_P(AudioPolicyAudioParameterTest, GetAudioParameter, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioSystemManager::GetInstance()->SetAudioParameter(params.key, params.value);
    EXPECT_EQ(params.value, AudioSystemManager::GetInstance()->GetAudioParameter(params.key));
}

INSTANTIATE_TEST_SUITE_P(
    GetAudioParameter,
    AudioPolicyAudioParameterTest,
    ValuesIn(AUDIO_PARAMS));

/*
 * Check set audio scene
 *
 */
class AudioPolicySetAudioSceneTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicySetAudioSceneTest, SetAudioScene, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioScene scene = params.audioScene;
    int32_t ret = AudioSystemManager::GetInstance()->SetAudioScene(scene);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(SUCCESS, ret);
}

INSTANTIATE_TEST_SUITE_P(
    SetAudioScene,
    AudioPolicySetAudioSceneTest,
    ValuesIn(AUDIO_SCENE_PARAMS));

/*
 * Check get audio scene
 *
 */
class AudioPolicyGetAudioSceneTest : public AudioPolicyTest {};

HWTEST_P(AudioPolicyGetAudioSceneTest, GetAudioScene, TestSize.Level1)
{
    PolicyParam params = GetParam();
    AudioScene scene = params.audioScene;
    EXPECT_EQ(AudioSystemManager::GetInstance()->GetAudioScene(), AUDIO_SCENE_DEFAULT);

    int32_t ret = AudioSystemManager::GetInstance()->SetAudioScene(scene);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_EQ(AudioSystemManager::GetInstance()->GetAudioScene(), scene);

    ret = AudioSystemManager::GetInstance()->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(SUCCESS, ret);
}

INSTANTIATE_TEST_SUITE_P(
    GetAudioScene,
    AudioPolicyGetAudioSceneTest,
    ValuesIn(AUDIO_SCENE_PARAMS));
} // namespace V1_0
} // namespace AudioStandard
} // namespace OHOS
