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

#include "audio_spatialization_manager_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void AudioSpatializationManagerUnitTest::SetUpTestCase(void) {}
void AudioSpatializationManagerUnitTest::TearDownTestCase(void) {}
void AudioSpatializationManagerUnitTest::SetUp(void) {}
void AudioSpatializationManagerUnitTest::TearDown(void) {}

void HeadTrackingDataRequestedChangeCallbackTest::OnHeadTrackingDataRequestedChange(bool isRequested)
{
    return;
}

/**
* @tc.name   : Test IsHeadTrackingDataRequested API
* @tc.number : IsHeadTrackingDataRequested_001
* @tc.desc   : Test IsHeadTrackingDataRequested interface.
*/
HWTEST(AudioSpatializationManagerUnitTest, IsHeadTrackingDataRequested_001, TestSize.Level1)
{
    std::string address = "";
    auto ret = AudioSpatializationManager::GetInstance()->IsHeadTrackingDataRequested(address);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name   : Test RegisterHeadTrackingDataRequestedEventListener API
* @tc.number : RegisterHeadTrackingDataRequestedEventListener_001
* @tc.desc   : Test RegisterHeadTrackingDataRequestedEventListener interface.
*/
HWTEST(AudioSpatializationManagerUnitTest, RegisterHeadTrackingDataRequestedEventListener_001, TestSize.Level1)
{
    std::string address = "123";
    std::shared_ptr<HeadTrackingDataRequestedChangeCallback> callback =
        make_shared<HeadTrackingDataRequestedChangeCallbackTest>();
    auto ret = AudioSpatializationManager::GetInstance()->RegisterHeadTrackingDataRequestedEventListener(address,
        callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test UnregisterHeadTrackingDataRequestedEventListener API
* @tc.number : UnregisterHeadTrackingDataRequestedEventListener_001
* @tc.desc   : Test UnregisterHeadTrackingDataRequestedEventListener interface.
*/
HWTEST(AudioSpatializationManagerUnitTest, UnregisterHeadTrackingDataRequestedEventListener_001, TestSize.Level1)
{
    std::string address = "123";
    auto ret = AudioSpatializationManager::GetInstance()->UnregisterHeadTrackingDataRequestedEventListener(address);
    EXPECT_EQ(SUCCESS, ret);
}
} // namespace AudioStandard
} // namespace OHOS
