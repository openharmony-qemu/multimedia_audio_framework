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
#include "va_device_controller_stub_impl_test.h"
#include <iostream>
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void VADeviceControllerStubImplTest::SetUpTestCase(void) {}
void VADeviceControllerStubImplTest::TearDownTestCase(void) {}
void VADeviceControllerStubImplTest::SetUp(void) {}
void VADeviceControllerStubImplTest::TearDown(void) {}

class HelloController : public VADeviceControllerCallback {
public:
    HelloController() = default;
    virtual ~HelloController() = default;

    int32_t OpenInputStream(const VAAudioStreamProperty &prop, const VAInputStreamAttribute &attr,
        std::shared_ptr<VAInputStreamCallback> &inputStream) override
    {
        std::shared_ptr<HelloInputStream> helloInputStream = std::make_shared<HelloInputStream>();
        if (helloInputStream == nullptr) {
            return 1;
        } else {
            inputStream = helloInputStream;
        }
        return 0;
    }

    int32_t GetParameters(const std::string& key, std::string& value) override
    {
        return 0;
    }

    int32_t SetParameters(const std::string& key, const std::string& value) override
    {
        return 0;
    }
};
/**
* @tc.name   : Test VADeviceControllerStubImpl
* @tc.number : SetVADeviceControllerCallback_001
* @tc.desc   : Test VADeviceControllerStubImpl OnInterrupt interface
*/
HWTEST(VADeviceControllerStubImplTest, SetVADeviceControllerCallback_001, TestSize.Level4) {
    VADeviceControllerStubImpl vaDeviceControllerStub;
    auto helloController = std::make_shared<HelloController>();

    EXPECT_EQ(vaDeviceControllerStub.SetVADeviceControllerCallback(helloController), SUCCESS);
}

/**
* @tc.name   : Test VADeviceControllerStubImpl
* @tc.number : SetVADeviceControllerCallback_002
* @tc.desc   : Test VADeviceControllerStubImpl OnInterrupt interface
*/
HWTEST(VADeviceControllerStubImplTest, SetVADeviceControllerCallback_002, TestSize.Level4) {
    VADeviceControllerStubImpl vaDeviceControllerStub;
    auto helloController = std::make_shared<HelloController>();

    EXPECT_EQ(vaDeviceControllerStub.SetVADeviceControllerCallback(nullptr), ERROR);
}

/**
* @tc.name   : Test VADeviceControllerStubImpl
* @tc.number : GetParameters_001
* @tc.desc   : Test VADeviceControllerStubImpl OnInterrupt interface
*/
HWTEST(VADeviceControllerStubImplTest, GetParameters_001, TestSize.Level4)
{
    VADeviceControllerStubImpl vaDeviceControllerStub;
    auto hellocontroller = std::make_shared<HelloController>();

    vaDeviceControllerStub.SetVADeviceControllerCallback(hellocontroller);

    std::string key = "test_key";
    std::string value;
    int32_t result = vaDeviceControllerStub.GetParameters(key, value);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name   : Test VADeviceControllerStubImpl
* @tc.number : GetParameters_002
* @tc.desc   : Test VADeviceControllerStubImpl OnInterrupt interface
*/
HWTEST(VADeviceControllerStubImplTest, GetParameters_002, TestSize.Level4) {
    VADeviceControllerStubImpl vaDeviceControllerStub;
    auto helloController = std::make_shared<HelloController>();
    vaDeviceControllerStub.SetVADeviceControllerCallback(helloController);

    std::string value;
                
    EXPECT_EQ(vaDeviceControllerStub.GetParameters("key1", value), SUCCESS);
    EXPECT_EQ(vaDeviceControllerStub.GetParameters("key2", value), SUCCESS);
}


/**
* @tc.name   : Test VADeviceControllerStubImpl
* @tc.number : SetParameters_001
* @tc.desc   : Test VADeviceControllerStubImpl OnInterrupt interface
*/
HWTEST(VADeviceControllerStubImplTest, SetParameters_001, TestSize.Level4) {
    VADeviceControllerStubImpl vaDeviceControllerStub;
    auto helloController = std::make_shared<HelloController>();
    vaDeviceControllerStub.SetVADeviceControllerCallback(helloController);

    std::string key = "test_key";
    std::string value = "test_value";

    EXPECT_EQ(vaDeviceControllerStub.SetParameters(key, value), SUCCESS);
}

/**
* @tc.name   : Test VADeviceControllerStubImpl
* @tc.number : SetParameters_002
* @tc.desc   : Test VADeviceControllerStubImpl OnInterrupt interface
*/
HWTEST(VADeviceControllerStubImplTest, SetParameters_002, TestSize.Level4) {
    VADeviceControllerStubImpl vaDeviceControllerStub;
    auto helloController = std::make_shared<HelloController>();
    vaDeviceControllerStub.SetVADeviceControllerCallback(helloController);

    EXPECT_EQ(vaDeviceControllerStub.SetParameters("key1", "value1"), SUCCESS);
    EXPECT_EQ(vaDeviceControllerStub.SetParameters("key2", "value2"), SUCCESS);
}


/**
* @tc.name   : Test VADeviceControllerStubImpl
* @tc.number : VADeviceControllerStubImpl_001
* @tc.desc   : Test VADeviceControllerStubImpl OnInterrupt interface.
*/
HWTEST(VADeviceControllerStubImplTest, OpenInputStream_001, TestSize.Level4) {
    VADeviceControllerStubImpl vaDeviceControllerStub;
    auto helloController = std::make_shared<HelloController>();
    vaDeviceControllerStub.SetVADeviceControllerCallback(helloController);

    VAAudioStreamProperty prop;
    VAInputStreamAttribute attr;
    sptr<IRemoteObject> inputStream;

    EXPECT_EQ(vaDeviceControllerStub.OpenInputStream(prop, attr, inputStream), SUCCESS);
}

}  //namespace AudioStandard
}  //namespace OHOS