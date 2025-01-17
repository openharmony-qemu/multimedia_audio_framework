/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "AudioEffectHdiParamUnitTest"
#endif

#include "audio_effect_hdi_param_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_effect_hdi_param.h"
#include "audio_errors.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
}

void AudioEffectHdiParamUnitTest::SetUpTestCase(void) {}
void AudioEffectHdiParamUnitTest::TearDownTestCase(void) {}
void AudioEffectHdiParamUnitTest::SetUp(void) {}
void AudioEffectHdiParamUnitTest::TearDown(void) {}

/**
* @tc.name   : Test InitHdi API
* @tc.number : InitHdi_001
* @tc.desc   : Test InitHdi interface.
*/
HWTEST(AudioEffectHdiParamUnitTest, InitHdi_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectHdiParamUnitTest: InitHdi_001 start ");
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam_ = std::make_shared<AudioEffectHdiParam>();
    audioEffectHdiParam_->InitHdi();
}

/**
* @tc.name   : Test UpdateHdiState API
* @tc.number : UpdateHdiState_001
* @tc.desc   : Test UpdateHdiState interface.
*/
HWTEST(AudioEffectHdiParamUnitTest, UpdateHdiState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectHdiParamUnitTest: UpdateHdiState_001 start ");
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam_ = std::make_shared<AudioEffectHdiParam>();
    audioEffectHdiParam_->InitHdi();

    int8_t effectHdiInput_[SEND_HDI_COMMAND_LEN];
    effectHdiInput_[0] = HDI_ROTATION;
    effectHdiInput_[1] = HDI_VOLUME;
    audioEffectHdiParam_->hdiModel_ = nullptr;
    int32_t result = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test UpdateHdiState API
* @tc.number : UpdateHdiState_002
* @tc.desc   : Test UpdateHdiState interface.
*/
HWTEST(AudioEffectHdiParamUnitTest, UpdateHdiState_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioEffectHdiParamUnitTest: UpdateHdiState_002 start ");
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam_ = std::make_shared<AudioEffectHdiParam>();
    audioEffectHdiParam_->InitHdi();

    int8_t effectHdiInput_[SEND_HDI_COMMAND_LEN];
    effectHdiInput_[0] = HDI_ROTATION;
    effectHdiInput_[1] = HDI_VOLUME;
    audioEffectHdiParam_->hdiModel_ = nullptr;
    int32_t result = audioEffectHdiParam_->UpdateHdiState(effectHdiInput_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ERROR, result);
}
} // namespace AudioStandard
} // namespace OHOS