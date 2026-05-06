/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_INTERPHONE_TEST_H
#define AUDIO_INTERPHONE_TEST_H

#include <audio_system_manager.h>
#include <gtest/gtest.h>
#include <audio_policy_log.h>

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {
using namespace std;
using namespace testing;

struct InterphoneParam {
    float volume;
    StreamUsage streamUsage;
    SourceType sourceType;
    uint32_t outputRouteFlag;
    uint32_t inputRouteFlag;
    AudioStreamType expectedStreamType;
    AudioPipeType expectedPipeType;
    bool mute;
    bool isActive;
    AudioMode mode;
};

struct InterphoneAdvancedParam {
    StreamUsage streamUsage;
    SourceType sourceType;
    uint32_t outputRouteFlag;
    uint32_t inputRouteFlag;
    AudioMode mode;
};

class AudioInterphoneTest : public TestWithParam<InterphoneParam> {
public:
    AudioInterphoneTest() {}

    virtual ~AudioInterphoneTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override;
    void TearDown(void) override;
};

class AudioInterphoneAdvancedTest : public Test {
public:
    AudioInterphoneAdvancedTest() {}

    virtual ~AudioInterphoneAdvancedTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override;
    void TearDown(void) override;
};

class AudioInterphoneBranchTest : public Test {
public:
    AudioInterphoneBranchTest() {}

    virtual ~AudioInterphoneBranchTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override;
    void TearDown(void) override;
};
} // namespace V1_0
} // namespace AudioStandard
} // namespace OHOS
#endif  // AUDIO_INTERPHONE_TEST_H