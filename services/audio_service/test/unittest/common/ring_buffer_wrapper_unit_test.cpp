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

#include <thread>
#include <cinttypes>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "ring_buffer_wrapper.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
} // namespace

class RingBufferWrapperUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name  : Test RingBufferWrapper API
 * @tc.type  : FUNC
 * @tc.number: RingBufferWrapper_001
 * @tc.desc  : Test RingBufferWrapper interface.
 */
HWTEST_F(RingBufferWrapperUnitTest, RingBufferWrapper_001, TestSize.Level0)
{
    std::vector<uint8_t> vecU8 = {0};
    BasicBufferDesc illegalBasicBuffer1 = {.buffer = nullptr, .bufLength = 1};
    EXPECT_EQ(illegalBasicBuffer1.IsLegal(), false);

    BasicBufferDesc illegalBasicBuffer2 = {.buffer = vecU8.data(), .bufLength = 0};
    EXPECT_EQ(illegalBasicBuffer2.IsLegal(), false);

    BasicBufferDesc legalBasicBuffer = {.buffer = vecU8.data(), .bufLength = 1};
    EXPECT_EQ(legalBasicBuffer.IsLegal(), true);

    RingBufferWrapper illegalRingBuffer1 = {
        .basicBufferDescs = {{
            illegalBasicBuffer1,
            legalBasicBuffer
        }},
        .dataLength = 1
    };
    EXPECT_EQ(illegalRingBuffer1.IsLegal(), false);

    RingBufferWrapper illegalRingBuffer2 = {
        .basicBufferDescs = {{
            legalBasicBuffer,
            illegalBasicBuffer1
        }},
        .dataLength = 1
    };
    EXPECT_EQ(illegalRingBuffer2.IsLegal(), false);
}

/**
 * @tc.name  : Test RingBufferWrapper API
 * @tc.type  : FUNC
 * @tc.number: RingBufferWrapper_002
 * @tc.desc  : Test RingBufferWrapper interface.
 */
HWTEST_F(RingBufferWrapperUnitTest, RingBufferWrapper_002, TestSize.Level0)
{
    // size 10, value = 0
    std::vector<uint8_t> vecU8(10, 0);
    // [0, 5)
    BasicBufferDesc legalBasicBuffer1 = {.buffer = vecU8.data(), .bufLength = 5};
    EXPECT_EQ(legalBasicBuffer1.IsLegal(), true);

    // [5, 10)
    BasicBufferDesc legalBasicBuffer2 = {.buffer = (vecU8.data() + 5), .bufLength = 5};
    EXPECT_EQ(legalBasicBuffer2.IsLegal(), true);

    // [4, 10)
    BasicBufferDesc legalBasicBuffer3 = {.buffer = (vecU8.data() + 4), .bufLength = 6};
    EXPECT_EQ(legalBasicBuffer3.IsLegal(), true);

    BasicBufferDesc legalBasicBuffer4 = {.buffer = nullptr, .bufLength = 0};
    EXPECT_EQ(legalBasicBuffer4.IsLegal(), true);

    RingBufferWrapper illegalRingBuffer1 = {
        .basicBufferDescs = {{
            legalBasicBuffer4,
            legalBasicBuffer1
        }},
        .dataLength = 0
    };
    EXPECT_EQ(illegalRingBuffer1.IsLegal(), false);

    RingBufferWrapper illegalRingBuffer2 = {
        .basicBufferDescs = {{
            legalBasicBuffer1,
            legalBasicBuffer3
        }},
        .dataLength = 0
    };
    EXPECT_EQ(illegalRingBuffer2.IsLegal(), false);

    RingBufferWrapper illegalRingBuffer3 = {
        .basicBufferDescs = {{
            legalBasicBuffer1,
            legalBasicBuffer2
        }},
        // len of buffer1 is 5, len of buffer2 is 5; datalenth > buffer1 + buffer2
        .dataLength = 11
    };
    EXPECT_EQ(illegalRingBuffer3.IsLegal(), false);
}

/**
 * @tc.name  : Test RingBufferWrapper API
 * @tc.type  : FUNC
 * @tc.number: RingBufferWrapper_003
 * @tc.desc  : Test RingBufferWrapper interface.
 */
HWTEST_F(RingBufferWrapperUnitTest, RingBufferWrapper_003, TestSize.Level0)
{
    // size 10, value = 0
    std::vector<uint8_t> vecU8(10, 0);
    // [0, 5)
    BasicBufferDesc legalBasicBuffer1 = {.buffer = vecU8.data(), .bufLength = 5};
    EXPECT_EQ(legalBasicBuffer1.IsLegal(), true);

    // [5, 10)
    BasicBufferDesc legalBasicBuffer2 = {.buffer = (vecU8.data() + 5), .bufLength = 5};
    EXPECT_EQ(legalBasicBuffer2.IsLegal(), true);

    // [4, 10)
    BasicBufferDesc legalBasicBuffer3 = {.buffer = (vecU8.data() + 4), .bufLength = 6};
    EXPECT_EQ(legalBasicBuffer3.IsLegal(), true);

    BasicBufferDesc legalBasicBuffer4 = {.buffer = nullptr, .bufLength = 0};
    EXPECT_EQ(legalBasicBuffer4.IsLegal(), true);

    RingBufferWrapper legalRingBuffer1 = {
        .basicBufferDescs = {{
            legalBasicBuffer1,
            legalBasicBuffer2
        }},
        .dataLength = 0
    };
    EXPECT_EQ(legalRingBuffer1.IsLegal(), true);

    RingBufferWrapper legalRingBuffer2 = {
        .basicBufferDescs = {{
            legalBasicBuffer1,
            legalBasicBuffer4
        }},
        .dataLength = 0
    };
    EXPECT_EQ(legalRingBuffer2.IsLegal(), true);

    RingBufferWrapper legalRingBuffer3 = {
        .basicBufferDescs = {{
            legalBasicBuffer4,
            legalBasicBuffer4
        }},
        .dataLength = 0
    };
    EXPECT_EQ(legalRingBuffer3.IsLegal(), true);
}

/**
 * @tc.name  : Test RingBufferWrapper API
 * @tc.type  : FUNC
 * @tc.number: RingBufferWrapper_004
 * @tc.desc  : Test RingBufferWrapper interface.
 */
HWTEST_F(RingBufferWrapperUnitTest, RingBufferWrapper_004, TestSize.Level0)
{
    // size 2, value = 0
    std::vector<uint8_t> vecU8(2, 0);
    RingBufferWrapper buffer1 = {
        .basicBufferDescs = {{
            {vecU8.data(), 2},
            {nullptr, 0}
        }},
        .dataLength = 2
    };

    // size 2, value = 1
    std::vector<uint8_t> vecU82(2, 1);
    RingBufferWrapper buffer2 = {
        .basicBufferDescs = {{
            {vecU82.data(), 2},
            {nullptr, 0}
        }},
        .dataLength = 1
    };

    buffer1.CopyInputBufferValueToCurBuffer(buffer2);
    EXPECT_THAT(vecU8, ElementsAre(1, 0));
}
} // namespace AudioStandard
} // namespace OHOS