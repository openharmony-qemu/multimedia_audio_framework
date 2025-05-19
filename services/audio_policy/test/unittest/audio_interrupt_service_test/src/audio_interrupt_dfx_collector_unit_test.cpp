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

#include "audio_interrupt_dfx_collector_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterruptDfxCollectorUnitTest::SetUpTestCase(void) {}
void AudioInterruptDfxCollectorUnitTest::TearDownTestCase(void) {}
void AudioInterruptDfxCollectorUnitTest::SetUp(void) {}
void AudioInterruptDfxCollectorUnitTest::TearDown(void) {}

void InterruptDfxBuilderUnitTest::SetUpTestCase(void) {}
void InterruptDfxBuilderUnitTest::TearDownTestCase(void) {}
void InterruptDfxBuilderUnitTest::SetUp(void) {}
void InterruptDfxBuilderUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioInterruptDfxCollector.
* @tc.number: FlushDfxMsg_001
* @tc.desc  : Test AudioInterruptDfxCollector::FlushDfxMsg
*/
HWTEST(AudioInterruptDfxCollectorUnitTest, FlushDfxMsg_001, TestSize.Level1)
{
    AudioInterruptDfxCollector dfxCollector;
    uint32_t index = 0;
    uint32_t appUid = -1;
    std::list<InterruptDfxInfo> dfxInfoList;

    dfxCollector.dfxInfos_[index] = dfxInfoList;
    dfxCollector.FlushDfxMsg(index, appUid);
    EXPECT_NE(dfxCollector.dfxInfos_.size(), 0);

    appUid = 1;
    dfxCollector.FlushDfxMsg(index, appUid);
    EXPECT_EQ(dfxCollector.dfxInfos_.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptDfxCollector.
* @tc.number: GetDfxIndexes_001
* @tc.desc  : Test AudioInterruptDfxCollector::GetDfxIndexes
*/
HWTEST(AudioInterruptDfxCollectorUnitTest, GetDfxIndexes_001, TestSize.Level1)
{
    AudioInterruptDfxCollector dfxCollector;
    uint32_t index = 0;

    dfxCollector.GetDfxIndexes(index);
    EXPECT_NE(dfxCollector.dfxIdx2InfoIdx_.size(), 0);
}

/**
* @tc.name  : Test InterruptDfxBuilder.
* @tc.number: WriteActionMsg_001
* @tc.desc  : Test InterruptDfxBuilder::WriteActionMsg
*/
HWTEST(AudioInterruptDfxCollectorUnitTest, WriteActionMsg_001, TestSize.Level1)
{
    InterruptDfxBuilder dfxBuilder;
    uint8_t infoIndex = 0;
    uint8_t effectIdx = 0;
    InterruptStage stage = INTERRUPT_STAGE_STOP;

    auto &ret = dfxBuilder.WriteActionMsg(infoIndex, effectIdx, stage);
    EXPECT_EQ(&ret, &dfxBuilder);
}

/**
* @tc.name  : Test InterruptDfxBuilder.
* @tc.number: WriteInfoMsg_001
* @tc.desc  : Test InterruptDfxBuilder::WriteInfoMsg
*/
HWTEST(AudioInterruptDfxCollectorUnitTest, WriteInfoMsg_001, TestSize.Level1)
{
    InterruptDfxBuilder dfxBuilder;
    AudioInterrupt audioInterrupt;
    AudioSessionStrategy strategy;
    InterruptRole interruptType = INTERRUPT_ROLE_DEFAULT;

    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    auto &ret = dfxBuilder.WriteInfoMsg(audioInterrupt, strategy, interruptType);
    EXPECT_EQ(&ret, &dfxBuilder);
}

/**
* @tc.name  : Test InterruptDfxBuilder.
* @tc.number: WriteEffectMsg_001
* @tc.desc  : Test InterruptDfxBuilder::WriteEffectMsg
*/
HWTEST(AudioInterruptDfxCollectorUnitTest, WriteEffectMsg_001, TestSize.Level1)
{
    InterruptDfxBuilder dfxBuilder;
    uint8_t appstate = 1;
    std::string bundleName = "com.ohos.test";
    AudioInterrupt audioInterrupt;
    InterruptHint hintType = INTERRUPT_HINT_NONE;

    auto &ret = dfxBuilder.WriteEffectMsg(appstate, bundleName, audioInterrupt, hintType);
    EXPECT_EQ(&ret, &dfxBuilder);
}

/**
* @tc.name  : Test InterruptDfxBuilder.
* @tc.number: GetResult_001
* @tc.desc  : Test InterruptDfxBuilder::GetResult
*/
HWTEST(AudioInterruptDfxCollectorUnitTest, GetResult_001, TestSize.Level1)
{
    InterruptDfxBuilder dfxBuilder;

    auto ret = dfxBuilder.GetResult();
    EXPECT_EQ(ret.interruptEffectVec.size(), 0);
}
} // AudioStandard
} // OHOS
