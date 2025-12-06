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

#ifndef AUDIO_POLICY_SERVICE_FOURTH_UNIT_TEST_H
#define AUDIO_POLICY_SERVICE_FOURTH_UNIT_TEST_H

#include <gtest/gtest.h>

#include "audio_policy_service.h"
#include "audio_policy_server.h"
#include "message_parcel.h"
#include "token_setproc.h"

namespace OHOS {
namespace AudioStandard {

struct StreamPropTestInfo {
    AudioSampleFormat format_ = INVALID_WIDTH;
    uint32_t sampleRate_ = 0;
    AudioChannelLayout channelLayout_ = CH_LAYOUT_UNKNOWN;
    AudioChannel channels_ = CHANNEL_UNKNOW;
};

class GetDynamicInfoTestData {
public:
    GetDynamicInfoTestData(AudioStreamInfo streamInfo, AudioSampleFormat format, uint32_t sampleRate,
        AudioChannelLayout channelLayout, AudioChannel channels);

    bool Check(std::shared_ptr<PipeStreamPropInfo> streamPropInfo);
    AudioStreamInfo streamInfo_;
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo_ = nullptr;
};

class AudioPolicyServiceFourthUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_SERVICE_FOURTH_UNIT_TEST_H