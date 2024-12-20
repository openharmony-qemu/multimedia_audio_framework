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
#include "fast_audio_stream.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class FastSystemStreamUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test GetVolume API
 * @tc.type  : FUNC
 * @tc.number: GetVolume_001
 * @tc.desc  : Test GetVolume interface.
 */
HWTEST(FastSystemStreamUnitTest, GetVolume_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolume_001 start");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    float result = fastAudioStream->GetVolume();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolume_001 result:%{public}f", result);
    EXPECT_GT(result, 0);
}

/**
 * @tc.name  : Test SetVolume API
 * @tc.type  : FUNC
 * @tc.number: SetVolume_001
 * @tc.desc  : Test SetVolume interface.
 */
HWTEST(FastSystemStreamUnitTest, SetVolume_001, TestSize.Level1)
{
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    float volume = 0.5f;
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    int32_t result = fastAudioStream->SetVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetVolume_001 result:%{public}d", result);
    EXPECT_NE(result, ERROR);
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API
 * @tc.type  : FUNC
 * @tc.number: SetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test SetSilentModeAndMixWithOthers interface.
 */
HWTEST(FastSystemStreamUnitTest, SetSilentModeAndMixWithOthers_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);

    bool on = false;
    fastAudioStream->silentModeAndMixWithOthers_ = false;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -1");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -2");

    on = true;
    fastAudioStream->silentModeAndMixWithOthers_ = false;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -3");
    fastAudioStream->silentModeAndMixWithOthers_ = true;
    fastAudioStream->SetSilentModeAndMixWithOthers(on);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSilentModeAndMixWithOthers_001 -4");
}

/**
 * @tc.name  : Test GetSwitchInfo API
 * @tc.type  : FUNC
 * @tc.number: GetSwitchInfo_001
 * @tc.desc  : Test GetSwitchInfo interface.
 */
HWTEST(FastSystemStreamUnitTest, GetSwitchInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSwitchInfo_001 start");
    int32_t appUid = static_cast<int32_t>(getuid());
    IAudioStream::SwitchInfo info;
    std::shared_ptr<FastAudioStream> fastAudioStream;
    fastAudioStream = std::make_shared<FastAudioStream>(STREAM_MUSIC, AUDIO_MODE_PLAYBACK, appUid);
    fastAudioStream->GetSwitchInfo(info);
}
} // namespace AudioStandard
} // namespace OHOS