/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_INTERPHONE_UNIT_TEST_H
#define AUDIO_INTERPHONE_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

struct InterphoneAdvancedParam {
    StreamUsage streamUsage;
    SourceType sourceType;
    uint32_t outputRouteFlag;
    uint32_t inputRouteFlag;
    AudioMode mode;
    bool isActive;
};

class AudioInterphoneAdvancedUnitTest : public testing::TestWithParam<InterphoneAdvancedParam> {
public:
    AudioInterphoneAdvancedUnitTest() {}
    virtual ~AudioInterphoneAdvancedUnitTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override;
    void TearDown(void) override;
};

class AudioInterphoneBranchUnitTest : public testing::Test {
public:
    AudioInterphoneBranchUnitTest() {}
    virtual ~AudioInterphoneBranchUnitTest() {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override;
    void TearDown(void) override;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INTERPHONE_UNIT_TEST_H