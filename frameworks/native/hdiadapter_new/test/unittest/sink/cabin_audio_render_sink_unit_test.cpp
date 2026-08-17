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


#include <gtest/gtest.h>
#include "cabin_audio_render_sink.h"
#include "audio_errors.h"
#include "parameter.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class CabinAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override
    {
        sink_ = std::make_unique<CabinAudioRenderSink>();
    }
    void TearDown() override
    {
        sink_ = nullptr;
    }

    std::unique_ptr<CabinAudioRenderSink> sink_;
};

// 注：Init/Start/Stop/RenderFrame/Pause/DeInit 现走真实 HDI(CreateRender/RenderFrame)，
// 单测环境无 HDI 设备，相关用例已移除，待具备 HDI mock 后补充。

/**
* @tc.name   : FormatConversion_Test
* @tc.type   : FUNC
* @tc.desc   : Validate internal helper functions for format and bit depth conversion
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, FormatConversion_Test, TestSize.Level1)
{
    // AUDIO_FORMAT_TYPE_3DA_16BIT/3DA_32BIT 枚举待 HDI 侧 audio_types.h 合入，实现暂用数值占位
    // (1023410177)；S24LE/F32LE(3DA_32BIT) 分支暂时屏蔽，非 S16LE 走 default 同样返回占位值。
    const AudioFormat format3da16bit = static_cast<AudioFormat>(1023410177);
    EXPECT_EQ(sink_->ConvertToHdiFormat(SAMPLE_S16LE), format3da16bit);
    EXPECT_EQ(sink_->ConvertToHdiFormat(SAMPLE_F32LE), format3da16bit);

    EXPECT_EQ(sink_->PcmFormatToBit(SAMPLE_S16LE), PCM_16_BIT);
    EXPECT_EQ(sink_->PcmFormatToBit(SAMPLE_F32LE), PCM_16_BIT);
}

/**
* @tc.name   : GetPresentationPosition_001
* @tc.type   : FUNC
* @tc.desc   : Test GetPresentationPosition to ensure it returns 0 as currently implemented
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, GetPresentationPosition_001, TestSize.Level1)
{
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;

    EXPECT_EQ(sink_->GetPresentationPosition(frames, timeSec, timeNanoSec), 0);
}

/**
* @tc.name   : SetMuteAndBalance_001
* @tc.type   : FUNC
* @tc.desc   : Test mute and Balance APIs (testing coverage for unsupported/empty function)
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, SetMuteAndBalance_001, TestSize.Level1)
{
    // Test SetSinkMuteForSwitchDevice
    EXPECT_EQ(sink_->SetSinkMuteForSwitchDevice(true), ERR_NOT_SUPPORTED);

    // Test SetAudioBalanceValue and SetAudioMonoState(empty, just call for coverage)
    sink_->SetAudioBalanceValue(1.0f);
    sink_->SetAudioMonoState(true);
    int64_t volumeData = 0;
    EXPECT_EQ(sink_->GetVolumeDataCount(volumeData), ERR_NOT_SUPPORTED);
    sink_->SetSpeed(1.0f);
    EXPECT_EQ(sink_->Reset(), SUCCESS);
    sink_->GetMaxAmplitude();
    EXPECT_EQ(sink_->Flush(), SUCCESS);
}

/**
* @tc.name   : UpdateAppsUid_001
* @tc.type   : FUNC
* @tc.desc   : Test UpdateAppsUid with array and vector inputs
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, UpdateAppsUid_001, TestSize.Level1)
{
    int32_t appsUid[2] = {1001, 1002};
    size_t size = 2;

    int32_t retArray = sink_->UpdateAppsUid(appsUid, size);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(retArray, ERR_INVALID_HANDLE);
#else
    EXPECT_EQ(retArray, SUCCESS);
#endif

    std::vector<int32_t> appsUidVec = {1003, 1004};
    int32_t retVec = sink_->UpdateAppsUid(appsUidVec);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(retVec, ERR_INVALID_HANDLE);
#else
    EXPECT_EQ(retVec, SUCCESS);
#endif
}

/**
* @tc.name   : UpdateAppsUid_002
* @tc.type   : FUNC
* @tc.desc   : Test UpdateAppsUid with empty inputs
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, UpdateAppsUid_002, TestSize.Level1)
{
    int32_t appsUid[1] = {0};
    size_t size = 1;

    int32_t retArray = sink_->UpdateAppsUid(appsUid, size);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(retArray, ERR_INVALID_HANDLE);
#else
    EXPECT_EQ(retArray, SUCCESS);
#endif

    std::vector<int32_t> appsUidVec = {};
    int32_t retVec = sink_->UpdateAppsUid(appsUidVec);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(retVec, ERR_INVALID_HANDLE);
#else
    EXPECT_EQ(retVec, SUCCESS);
#endif
}

/**
* @tc.name   : DumpInfo_001
* @tc.type   : FUNC
* @tc.desc   : Test DumpInfo to ensure string formatting is correct
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, DumpInfo_001, TestSize.Level1)
{
    std::string dumpString = "";
    sink_->started_ = true;
    sink_->DumpInfo(dumpString);

    EXPECT_NE(dumpString.find("3dadirectSink"), std::string::npos);
    EXPECT_NE(dumpString.find("started: true"), std::string::npos);
}
}
}
