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

#include "audio_setting_provider_unit_test.h"
#include "audio_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSettingProviderUnitTest::SetUp(void) {}

void AudioSettingProviderUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test OnChange API
 * @tc.type  : FUNC
 * @tc.number: OnChange_001
 * @tc.desc  : Test update_= nullptr
 */
HWTEST(AudioSettingProviderUnitTest, OnChange_001, TestSize.Level1)
{
    auto audioSettingObserverTest_ = std::make_shared<AudioSettingObserver>();
    ASSERT_TRUE(audioSettingObserverTest_ != nullptr);

    audioSettingObserverTest_->update_ = nullptr;

    audioSettingObserverTest_->OnChange();
    EXPECT_EQ(audioSettingObserverTest_->update_, nullptr);
}

/**
 * @tc.name  : Test OnChange API
 * @tc.type  : FUNC
 * @tc.number: OnChange_002
 * @tc.desc  : Test Test update_ != nullptr
 */
HWTEST(AudioSettingProviderUnitTest, OnChange_002, TestSize.Level1)
{
    auto audioSettingObserverTest_ = std::make_shared<AudioSettingObserver>();
    ASSERT_TRUE(audioSettingObserverTest_ != nullptr);

    AudioSettingObserver::UpdateFunc func;
    func = [](const std::string &str) {
        return true;
    };

    audioSettingObserverTest_->SetUpdateFunc(func);

    audioSettingObserverTest_->OnChange();
    ASSERT_TRUE(audioSettingObserverTest_->update_ != nullptr);
}

/**
 * @tc.name  : Test GetInstance API
 * @tc.type  : FUNC
 * @tc.number: GetInstance_001
 * @tc.desc  : Test GetInstance interface
 */
HWTEST(AudioSettingProviderUnitTest, GetInstance_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProviderTest1 = nullptr;
    AudioSettingProvider* audioSettingProviderTest2 = nullptr;

    int32_t systemAbilityId = 1;
    audioSettingProviderTest1 = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProviderTest1 != nullptr);

    audioSettingProviderTest2 = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProviderTest2 != nullptr);
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_001
 * @tc.desc  : Test pos < len && isspace(input[pos]
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_001, TestSize.Level1)
{
    size_t pos_ = 0;
    std::string input_ = " =example";
    size_t len_ = input_.length();

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "example");
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_002
 * @tc.desc  : Test pos < len && !isspace
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_002, TestSize.Level1)
{
    size_t pos_ = 0;
    std::string input_ = "=example";
    size_t len_ = input_.length();

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "example");
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_003
 * @tc.desc  : Test pos >= len
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_003, TestSize.Level1)
{
    size_t pos_ = 3;
    size_t len_ = 1;
    std::string input_ = "=example";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_004
 * @tc.desc  : Test pos < len && input[pos] != '"'.
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_004, TestSize.Level1)
{
    size_t pos_ = 0;
    std::string input_ = "=name: 30";
    size_t len_ = input_.length();

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "name: 30");
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_005
 * @tc.desc  : Test Test pos < len && input[pos] = '"'.
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_005, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 4;
    std::string input_ = "\"id\"";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "id");
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_006
 * @tc.desc  : Test ParseFirstOfKey interface.
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_006, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 5;
    std::string input_ = "\"name\": \"John\"";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "name");
    EXPECT_EQ(pos_, 5);
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_007
 * @tc.desc  : Test ParseFirstOfKey interface.
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_007, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 100;
    std::string input_ = "=example";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "example");
    EXPECT_EQ(pos_ > input_.length(), true);
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseFirstOfKey_008
 * @tc.desc  : Test ParseFirstOfKey interface.
 */
HWTEST(AudioSettingProviderUnitTest, ParseFirstOfKey_008, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 5;
    std::string input_ = "=exampleTest";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseFirstOfKey(pos_, len_, input_);
    EXPECT_EQ(ret, "exam");
    EXPECT_EQ(pos_, 5);
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_001
 * @tc.desc  : Test pos < len && isspace(input[pos]
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_001, TestSize.Level1)
{
    size_t pos_ = 1;
    size_t len_ = 8;
    std::string input_ = " example";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "example");
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_002
 * @tc.desc  : Test pos < len && !isspace
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_002, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 8;
    std::string input_ = "example";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "example");
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_003
 * @tc.desc  : Test pos >= len
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_003, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 8;
    std::string input_ = "example";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "example");
}

/**
 * @tc.name  : Test ParseFirstOfKey API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_004
 * @tc.desc  : Test pos < len && input[pos] != '"'.
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_004, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 8;
    std::string input_ = "name: 30";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "name: 30");
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_005
 * @tc.desc  : Test pos < len && input[pos] = ','.
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_005, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 4;
    std::string input_ = "\"id\"";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "\"id\"");
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_006
 * @tc.desc  : Test pos < len && !input[pos] = '"'.
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_006, TestSize.Level1)
{
    size_t pos_ = 1;
    size_t len_ = 5;
    std::string input_ = "\"name\": \"John\"";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "name");
    EXPECT_EQ(pos_, 5);
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_007
 * @tc.desc  : Test pos < len && !input[pos] = '"'.
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_007, TestSize.Level1)
{
    size_t pos_ = 1;
    size_t len_ = 5;
    std::string input_ = "\"name\": \"John\"";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "name");
    EXPECT_EQ(pos_, 5);
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: ParseSecondOfValue_008
 * @tc.desc  : Test ParseSecondOfValue interface.
 */
HWTEST(AudioSettingProviderUnitTest, ParseSecondOfValue_008, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 100;
    std::string input_ = "example";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "example");
    EXPECT_EQ(pos_ > input_.length(), true);
}

/**
 * @tc.name  : Test ParseSecondOfValue API
 * @tc.type  : FUNC
 * @tc.number: PParseSecondOfValue_009
 * @tc.desc  : Test ParseSecondOfValue interface.
 */
HWTEST(AudioSettingProviderUnitTest, PParseSecondOfValue_009, TestSize.Level1)
{
    size_t pos_ = 0;
    size_t len_ = 5;
    std::string input_ = "exampleTest";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string ret = audioSettingProvider_->ParseSecondOfValue(pos_, len_, input_);
    EXPECT_EQ(ret, "examp");
    EXPECT_EQ(pos_, 5);
}

/**
 * @tc.name  : Test IsValidKey API
 * @tc.type  : FUNC
 * @tc.number: IsValidKey_001
 * @tc.desc  : Test ret != ERR_NAME_NOT_FOUND;
 */
HWTEST(AudioSettingProviderUnitTest, IsValidKey_001, TestSize.Level1)
{
    std::string key = "test";

    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(false);
    bool ret = audioSettingProvider_->IsValidKey(key);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsValidKey API
 * @tc.type  : FUNC
 * @tc.number: IsValidKey_002
 * @tc.desc  : Test ret != ERR_NAME_NOT_FOUND;
 */
HWTEST(AudioSettingProviderUnitTest, IsValidKey_002, TestSize.Level1)
{
    std::string key = "";
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    bool ret = audioSettingProvider_->IsValidKey(key);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test ExecRegisterCb API
 * @tc.type  : FUNC
 * @tc.number: ExecRegisterCb_001
 * @tc.desc  : Test observer == nullptr
 */
HWTEST(AudioSettingProviderUnitTest, ExecRegisterCb_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->ExecRegisterCb(nullptr);
}

/**
 * @tc.name  : Test UnregisterObserver API
 * @tc.type  : FUNC
 * @tc.number: UnregisterObserver_001
 * @tc.desc  : Test UnregisterObserver
 */
HWTEST(AudioSettingProviderUnitTest, UnregisterObserver_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(false);

    sptr<AudioSettingObserver> observerPtr = new AudioSettingObserver();
    ErrCode ret = audioSettingProvider_->UnregisterObserver(observerPtr, "TEST");
    EXPECT_EQ(ret, ERR_NO_INIT);
}

/**
 * @tc.name  : Test UnregisterObserver API
 * @tc.type  : FUNC
 * @tc.number: UnregisterObserver_002
 * @tc.desc  : Test UnregisterObserver
 */
HWTEST(AudioSettingProviderUnitTest, UnregisterObserver_002, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    sptr<AudioSettingObserver> observerPtr = new AudioSettingObserver();
    ErrCode ret = audioSettingProvider_->UnregisterObserver(observerPtr, "TEST");
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name  : Test PutStringValue API
 * @tc.type  : FUNC
 * @tc.number: PutStringValue_001
 * @tc.desc  : Test PutStringValue
 */
HWTEST(AudioSettingProviderUnitTest, PutStringValue_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    
    ErrCode ret = audioSettingProvider_->PutStringValue("key", "value", "tableType", false, 0);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name  : Test CreateDataShareHelper API
 * @tc.type  : FUNC
 * @tc.number: CreateDataShareHelper_001
 * @tc.desc  : Test CreateDataShareHelper
 */
HWTEST(AudioSettingProviderUnitTest, CreateDataShareHelper_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    
    auto helper = audioSettingProvider_->CreateDataShareHelper("system", -1);
    ASSERT_TRUE(helper != nullptr);
}

/**
 * @tc.name  : Test CreateDataShareHelper API
 * @tc.type  : FUNC
 * @tc.number: CreateDataShareHelper_002
 * @tc.desc  : Test CreateDataShareHelper
 */
HWTEST(AudioSettingProviderUnitTest, CreateDataShareHelper_002, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    
    auto helper = audioSettingProvider_->CreateDataShareHelper("secure", -1);
    ASSERT_TRUE(helper != nullptr);
}

/**
 * @tc.name  : Test AssembleUri API
 * @tc.type  : FUNC
 * @tc.number: AssembleUri_001
 * @tc.desc  : Test AssembleUri
 */
HWTEST(AudioSettingProviderUnitTest, AssembleUri_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    
    Uri uri = audioSettingProvider_->AssembleUri("TEST", "secure", -1);
    ASSERT_TRUE(uri.uriString_.length() > 0);
}

/**
 * @tc.name  : Test AssembleUri API
 * @tc.type  : FUNC
 * @tc.number: AssembleUri_002
 * @tc.desc  : Test AssembleUri
 */
HWTEST(AudioSettingProviderUnitTest, AssembleUri_002, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    
    Uri uri = audioSettingProvider_->AssembleUri("TEST", "system", -1);
    ASSERT_TRUE(uri.uriString_.length() > 0);
}

/**
 * @tc.name  : Test CreateDataShareHelper API
 * @tc.type  : FUNC
 * @tc.number: CreateDataShareHelper_003
 * @tc.desc  : Test CreateDataShareHelper
 */
HWTEST(AudioSettingProviderUnitTest, CreateDataShareHelper_003, TestSize.Level1)
{
    #define SUPPORT_USER_ACCOUNT
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(true);
    
    auto helper = audioSettingProvider_->CreateDataShareHelper("test", MIN_USER_ACCOUNT);
    ASSERT_TRUE(helper != nullptr);
}

/**
 * @tc.name  : Test GetBoolValue API
 * @tc.type  : FUNC
 * @tc.number: GetBoolValue_001
 * @tc.desc  : Test GetBoolValue
 */
HWTEST(AudioSettingProviderUnitTest, GetBoolValue_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(false);
    
    std::string key = "test";
    bool value = true;
    ErrCode ret = audioSettingProvider_->GetBoolValue(key, value, "system", -1);
    ASSERT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name  : Test GetMapValue API
 * @tc.type  : FUNC
 * @tc.number: GetMapValue_001
 * @tc.desc  : Test GetMapValue
 */
HWTEST(AudioSettingProviderUnitTest, GetMapValue_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    audioSettingProvider_->SetDataShareReady(false);

    std::string key = "test";
    std::vector<std::map<std::string, std::string>> value;
    ErrCode ret = audioSettingProvider_->GetMapValue(key, value, "system");
    ASSERT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_000
 * @tc.desc  : Test ParseJsonArray pos >= len
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_000, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 0);
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_001
 * @tc.desc  : Test ParseJsonArray pos < len && isspace(input[pos] is false
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_001, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = ":tests";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 0);
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_002
 * @tc.desc  : Test ParseJsonArray pos < len && isspace(input[pos] is true
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_002, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = " :test";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 0);
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_003
 * @tc.desc  : Test ParseJsonArray input[pos++] == '[' && pos >= len
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_003, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 0);
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_004
 * @tc.desc  : Test ParseJsonArray input[pos] == ']'
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_004, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[ ]";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 0);
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_005
 * @tc.desc  : Test ParseJsonArray input[pos++]!= '{'
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_005, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[ ";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 0);
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_006
 * @tc.desc  : Test ParseJsonArray input[pos++]!= '{'
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_006, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_007
 * @tc.desc  : Test ParseJsonArray input[pos] == '}'
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_007, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_008
 * @tc.desc  : Test ParseJsonArray key != "uid" && input.find(',', pos) == std::string::npos
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_008, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{test}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_009
 * @tc.desc  : Test ParseJsonArray key != "uid" && input.find(',', pos) != std::string::npos
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_009, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{test,}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_010
 * @tc.desc  : Test ParseJsonArray key == "uid" && input.find(',', pos) == std::string::npos
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_010, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_011
 * @tc.desc  : Test ParseJsonArray key == "uid" && input.find(',', pos) != std::string::npos
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_011, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\",}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_012
 * @tc.desc  : Test ParseJsonArray !key.empty()
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_012, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\":}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_013
 * @tc.desc  : Test ParseJsonArray !key.empty()
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_013, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\":}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0][""], "");
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_014
 * @tc.desc  : Test ParseJsonArray !key.empty() && !value.empty()
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_014, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\":\"123\"}]";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);

    for (const auto& mapItem : result) {
        for (const auto& [key, value] : mapItem) {
            EXPECT_EQ(key, "123");
            EXPECT_EQ(value, "1");
        }
    }
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_015
 * @tc.desc  : Test ParseJsonArray !input[pos] == ','
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_015, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\":\"123\"},]";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    for (const auto& mapItem : result) {
        for (const auto& [key, value] : mapItem) {
            EXPECT_EQ(key, "123");
            EXPECT_EQ(value, "1");
        }
    }
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_016
 * @tc.desc  : Test ParseJsonArray input[pos] != ']'
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_016, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\":\"123\"}";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    for (const auto& mapItem : result) {
        for (const auto& [key, value] : mapItem) {
            EXPECT_EQ(key, "123");
            EXPECT_EQ(value, "1");
        }
    }
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_017
 * @tc.desc  : Test ParseJsonArray
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_017, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\":\"123\"},{\" uid\":\"456\"}]";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    for (const auto& mapItem : result) {
        for (const auto& [key, value] : mapItem) {
            if (key == "123") {
                EXPECT_EQ(key, "123");
                EXPECT_EQ(value, "1");
            }
            if (key == "456") {
                EXPECT_EQ(key, "456");
                EXPECT_EQ(value, "1");
            }
        }
    }
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_018
 * @tc.desc  : Test ParseJsonArray
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_018, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[{\" uid\":\"123\",\" name\":\"test\"}]";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    for (const auto& mapItem : result) {
        for (const auto& [key, value] : mapItem) {
            if (key == "123") {
                EXPECT_EQ(key, "123");
                EXPECT_EQ(value, "1");
            }
            if (key == "") {
                EXPECT_EQ(key, "");
                EXPECT_EQ(value, "");
            }
        }
    }
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_019
 * @tc.desc  : Test ParseJsonArray
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_019, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string json = "[ { \" uid\" : \"123\" } ]";
    auto result = audioSettingProvider_->ParseJsonArray(json);

    ASSERT_EQ(result.size(), 1);
    for (const auto& mapItem : result) {
        for (const auto& [key, value] : mapItem) {
            EXPECT_EQ(key, "123");
            EXPECT_EQ(value, "1");
        }
    }
}

/**
 * @tc.name  : Test ParseJsonArray API
 * @tc.type  : FUNC
 * @tc.number: ParseJsonArray_020
 * @tc.desc  : Test ParseJsonArray
 */
HWTEST(AudioSettingProviderUnitTest, ParseJsonArray_020, TestSize.Level1)
{
    AudioSettingProvider* audioSettingProvider_ = nullptr;
    int32_t systemAbilityId = 1;
    audioSettingProvider_ = &AudioSettingProvider::GetInstance(systemAbilityId);
    ASSERT_TRUE(audioSettingProvider_ != nullptr);

    std::string invalidJson = "[{\" uid\":\"123\"";
    auto result = audioSettingProvider_->ParseJsonArray(invalidJson);

    EXPECT_FALSE(result.empty());
}
} // namespace AudioStandard
} // namespace OHOS
