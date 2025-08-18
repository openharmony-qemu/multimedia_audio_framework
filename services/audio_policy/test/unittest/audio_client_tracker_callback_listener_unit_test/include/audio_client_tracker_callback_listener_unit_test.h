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

#ifndef AUDIO_CLIENT_TRACKER_CALLBACK_LISTENER_UNIT_TEST_H
#define AUDIO_CLIENT_TRACKER_CALLBACK_LISTENER_UNIT_TEST_H

#include "audio_client_tracker_callback_listener.h"
#include "audio_client_tracker_callback_service.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AudioStandard {

using namespace testing::ext;

class AudioClientTrackerCallbackListenerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);

private:
    sptr<IStandardClientTracker> listener_;
    StreamSetStateEventInternal streamSetStateEventInternal_;
    std::shared_ptr<AudioClientTracker> clientTracker_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif //AUDIO_CLIENT_TRACKER_CALLBACK_LISTENER_UNIT_TEST_H