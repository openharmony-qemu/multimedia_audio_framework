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

#include "client_type_manager_unit_test.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const int32_t TEST_CLIENT_UID = 1;
static const ClientType TEST_CLIENT_TYPE = CLIENT_TYPE_GAME;
static const std::string TEST_BUNDLE_NAME = "com.testApp";

void ClientTypeManagerUnitTest::SetUpTestCase(void) {}
void ClientTypeManagerUnitTest::TearDownTestCase(void) {}
void ClientTypeManagerUnitTest::SetUp(void) {}
void ClientTypeManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test ClientTypeManager.
 * @tc.number: ClientTypeManagerUnitTest_001.
 * @tc.desc  : Test GetAndSaveClientType API.
 */
HWTEST(ClientTypeManagerUnitTest, ClientTypeManagerUnitTest_001, TestSize.Level4)
{
    auto manager = ClientTypeManager::GetInstance();
    EXPECT_NE(nullptr, manager);
    manager->clientTypeMap_.clear();
    manager->OnClientTypeQueryCompleted(TEST_CLIENT_UID, TEST_CLIENT_TYPE);
    const int32_t checkSize = 1;
    EXPECT_EQ(checkSize, manager->clientTypeMap_.size());

    ClientType type = manager->GetClientTypeByUid(TEST_CLIENT_UID);
    EXPECT_EQ(TEST_CLIENT_TYPE, type);

    const int32_t TEST_INVALID_CLIENT_UID = 2;
    ClientType type2 = manager->GetClientTypeByUid(TEST_INVALID_CLIENT_UID);
    EXPECT_EQ(CLIENT_TYPE_OTHERS, type2);

    manager->GetAndSaveClientType(TEST_CLIENT_UID, TEST_BUNDLE_NAME);
    EXPECT_EQ(checkSize, manager->clientTypeMap_.size());

    const int32_t TEST_CLIENT_UID_2 = 5;
    const int32_t TEST_CLIENT_UID_3 = 6;
    manager->GetAndSaveClientType(TEST_CLIENT_UID_2, TEST_BUNDLE_NAME);
    manager->GetAndSaveClientType(TEST_CLIENT_UID_3, "");
}
} // AudioStandardnamespace
} // OHOSnamespace
