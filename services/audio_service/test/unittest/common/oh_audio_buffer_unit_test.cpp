/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "oh_audio_buffer.h"
#include "audio_service_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class OHAudioBufferUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OHAudioBufferUnitTest::SetUpTestCase(void) {}
void OHAudioBufferUnitTest::TearDownTestCase(void) {}
void OHAudioBufferUnitTest::SetUp(void) {}
void OHAudioBufferUnitTest::TearDown(void) {}

HWTEST(OHAudioBufferUnitTest, SetPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    uint64_t spanSize = 256;
    uint64_t engineSize = 512;
    uint32_t routeFlag = 1;
    
    buffer->SetPendingSpanSize(spanSize, engineSize, routeFlag);
    
    EXPECT_TRUE(buffer->HasPendingSpanSize());
}

HWTEST(OHAudioBufferUnitTest, SetPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    uint64_t spanSize = 0;
    uint64_t engineSize = 0;
    uint32_t routeFlag = 0;
    
    buffer->SetPendingSpanSize(spanSize, engineSize, routeFlag);
    
    EXPECT_FALSE(buffer->HasPendingSpanSize());
}

HWTEST(OHAudioBufferUnitTest, GetAndClearPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    uint64_t spanSize = 480;
    uint64_t engineSize = 960;
    uint32_t routeFlag = 2;
    
    buffer->SetPendingSpanSize(spanSize, engineSize, routeFlag);
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = buffer->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, spanSize);
    EXPECT_EQ(retrievedEngineSize, engineSize);
    EXPECT_EQ(retrievedRouteFlag, routeFlag);
    EXPECT_FALSE(buffer->HasPendingSpanSize());
}

HWTEST(OHAudioBufferUnitTest, GetAndClearPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = buffer->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, 0);
    EXPECT_EQ(retrievedEngineSize, 0);
    EXPECT_EQ(retrievedRouteFlag, 0);
    EXPECT_FALSE(buffer->HasPendingSpanSize());
}

HWTEST(OHAudioBufferUnitTest, HasPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    EXPECT_FALSE(buffer->HasPendingSpanSize());
    
    buffer->SetPendingSpanSize(100, 200, 1);
    EXPECT_TRUE(buffer->HasPendingSpanSize());
}

HWTEST(OHAudioBufferUnitTest, HasPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    buffer->SetPendingSpanSize(100, 200, 1);
    EXPECT_TRUE(buffer->HasPendingSpanSize());
    
    buffer->ClearPendingSpanSize();
    EXPECT_FALSE(buffer->HasPendingSpanSize());
}

HWTEST(OHAudioBufferUnitTest, ClearPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    buffer->SetPendingSpanSize(1000, 2000, 3);
    EXPECT_TRUE(buffer->HasPendingSpanSize());
    
    buffer->ClearPendingSpanSize();
    EXPECT_FALSE(buffer->HasPendingSpanSize());
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = buffer->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, 0);
    EXPECT_EQ(retrievedEngineSize, 0);
    EXPECT_EQ(retrievedRouteFlag, 0);
}

HWTEST(OHAudioBufferUnitTest, ClearPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t spanSizeInFrame = 256;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    buffer->ClearPendingSpanSize();
    EXPECT_FALSE(buffer->HasPendingSpanSize());
}

HWTEST(OHAudioBufferUnitTest, PendingSpanSizeMultipleOperations_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 2048;
    uint32_t spanSizeInFrame = 512;
    uint32_t byteSizePerFrame = 4;
    
    auto buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    ASSERT_NE(buffer, nullptr);
    
    buffer->SetPendingSpanSize(512, 1024, 1);
    EXPECT_TRUE(buffer->HasPendingSpanSize());
    
    uint64_t engineSize1 = 0;
    uint32_t routeFlag1 = 0;
    uint64_t spanSize1 = buffer->GetAndClearPendingSpanSize(engineSize1, routeFlag1);
    EXPECT_EQ(spanSize1, 512);
    EXPECT_EQ(engineSize1, 1024);
    EXPECT_EQ(routeFlag1, 1);
    EXPECT_FALSE(buffer->HasPendingSpanSize());
    
    buffer->SetPendingSpanSize(1024, 2048, 2);
    EXPECT_TRUE(buffer->HasPendingSpanSize());
    
    uint64_t engineSize2 = 0;
    uint32_t routeFlag2 = 0;
    uint64_t spanSize2 = buffer->GetAndClearPendingSpanSize(engineSize2, routeFlag2);
    EXPECT_EQ(spanSize2, 1024);
    EXPECT_EQ(engineSize2, 2048);
    EXPECT_EQ(routeFlag2, 2);
    EXPECT_FALSE(buffer->HasPendingSpanSize());
}

} // namespace AudioStandard
} // namespace OHOS