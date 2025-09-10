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
#include "va_input_stream_stub_impl_test.h"
#include <iostream>
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
    namespace AudioStandard {

        void VAInputStreamStubImplTest::SetUpTestCase(void) {}
        void VAInputStreamStubImplTest::TearDownTestCase(void) {}
        void VAInputStreamStubImplTest::SetUp(void) {}
        void VAInputStreamStubImplTest::TearDown(void) {}

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : Start_001
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, Start_001, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
            auto helloInputStream = std::make_shared<HelloInputStream>();

            vaInputStreamStub.SetVAInputStreamCallback(helloInputStream);
    
            int32_t result = vaInputStreamStub.Start();

            EXPECT_EQ(result, SUCCESS);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : Start_002
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, Start_002, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
             
            int32_t result = vaInputStreamStub.Start();

            EXPECT_EQ(result, ERROR);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : SetVAInputStreamCallback_001
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, SetVAInputStreamCallback_001, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
            auto helloInputStream = std::make_shared<HelloInputStream>();
            
            int32_t result = vaInputStreamStub.SetVAInputStreamCallback(nullptr);
            EXPECT_EQ(result, ERR_INVALID_PARAM);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : SetVAInputStreamCallback_002
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, SetVAInputStreamCallback_002, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
             
            std::shared_ptr<VAInputStreamCallback> nullCallback = nullptr;
            int32_t result = vaInputStreamStub.SetVAInputStreamCallback(nullCallback);
            
            EXPECT_EQ(result, ERR_INVALID_PARAM);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : GetStreamProperty_001
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, GetStreamProperty_001, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
            auto helloInputStream = std::make_shared<HelloInputStream>();

            vaInputStreamStub.SetVAInputStreamCallback (helloInputStream);

            VAAudioStreamProperty streamProp;

            int32_t result = vaInputStreamStub.GetStreamProperty(streamProp);

            EXPECT_EQ(result, SUCCESS);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : GetStreamProperty_002
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, GetStreamProperty_002, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
              
            VAAudioStreamProperty streamProp;

            int32_t result = vaInputStreamStub.GetStreamProperty(streamProp);

            EXPECT_EQ(result, ERROR);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : RequestSharedMem_001
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, RequestSharedMem_001, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
            auto helloInputStream = std::make_shared<HelloInputStream>();

            vaInputStreamStub.SetVAInputStreamCallback(helloInputStream);

            VASharedMemInfo memInfo;

            int32_t result = vaInputStreamStub.RequestSharedMem(memInfo);

            EXPECT_EQ(result, SUCCESS);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl
        * @tc.number : RequestSharedMem_002
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface
        */
        HWTEST(VAInputStreamStubImplTest, RequestSharedMem_002, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
             
            VASharedMemInfo memInfo;

            int32_t result = vaInputStreamStub.RequestSharedMem(memInfo);

            EXPECT_EQ(result, ERROR);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : Stop_001
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, Stop_001, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
            auto helloInputStream = std::make_shared<HelloInputStream>();

            vaInputStreamStub.SetVAInputStreamCallback(helloInputStream);

            int32_t result = vaInputStreamStub.Stop();

            EXPECT_EQ(result, SUCCESS);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : Stop_002
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, Stop_002, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
         
            int32_t result = vaInputStreamStub.Stop();

            EXPECT_EQ(result, ERROR);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : Close_001
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, Close_001, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
            auto helloInputStream = std::make_shared<HelloInputStream>();

            vaInputStreamStub.SetVAInputStreamCallback(helloInputStream);

            int32_t result = vaInputStreamStub.Close();

            EXPECT_EQ(result, SUCCESS);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : Close_002
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, Close_002, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;

            int32_t result = vaInputStreamStub.Close();

            EXPECT_EQ(result, ERROR);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : GetCapturePosition_001
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, GetCapturePosition_001, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
            auto helloInputStream = std::make_shared<HelloInputStream>();

            vaInputStreamStub.SetVAInputStreamCallback(helloInputStream);

            uint64_t attr_1 = 0;
            uint64_t attr_2 = 0;

            int32_t result = vaInputStreamStub.GetCapturePosition(attr_1, attr_2);

            EXPECT_EQ(result, SUCCESS);
        }

        /**
        * @tc.name   : TestVAInputStreamStubImpl.
        * @tc.number : GetCapturePosition_002
        * @tc.desc   : TestVAInputStreamStubImpl OnInterrupt interface.
        */
        HWTEST(VAInputStreamStubImplTest, GetCapturePosition_002, TestSize.Level4)
        {
            VAInputStreamStubImpl vaInputStreamStub;
             
            uint64_t attr_1 = 0;
            uint64_t attr_2 = 0;

            int32_t result = vaInputStreamStub.GetCapturePosition(attr_1, attr_2);
            EXPECT_EQ(result, ERROR);
        }
    }  // namespace AudioStandard
}  // namespace OHOS