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

#ifndef AUDIO_VOLUME_CHANGE_UNIT_TEST_H
#define AUDIO_VOLUME_CHANGE_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioVolumeChangeUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void WaitForCallback();
};
class ApplicationCallback : public VolumeKeyEventCallback {
public:
    explicit ApplicationCallback(const std::string &testCaseName);
    ~ApplicationCallback() = default;

    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;

private:
    std::string testCaseName_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_VOLUME_CHANGE_UNIT_TEST_H