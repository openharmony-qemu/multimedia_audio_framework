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

class OHAudioBufferBaseUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OHAudioBufferBaseUnitTest::SetUpTestCase(void) {}
void OHAudioBufferBaseUnitTest::TearDownTestCase(void) {}
void OHAudioBufferBaseUnitTest::SetUp(void) {}
void OHAudioBufferBaseUnitTest::TearDown(void) {}

HWTEST(OHAudioBufferBaseUnitTest, SetPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    uint64_t spanSize = 256;
    uint64_t engineSize = 512;
    uint32_t routeFlag = 1;
    
    bufferBase->SetPendingSpanSize(spanSize, engineSize, routeFlag);
    
    EXPECT_TRUE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, SetPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    uint64_t spanSize = 0;
    uint64_t engineSize = 0;
    uint32_t routeFlag = 0;
    
    bufferBase->SetPendingSpanSize(spanSize, engineSize, routeFlag);
    
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, GetAndClearPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    uint64_t spanSize = 480;
    uint64_t engineSize = 960;
    uint32_t routeFlag = 2;
    
    bufferBase->SetPendingSpanSize(spanSize, engineSize, routeFlag);
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = bufferBase->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, spanSize);
    EXPECT_EQ(retrievedEngineSize, engineSize);
    EXPECT_EQ(retrievedRouteFlag, routeFlag);
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, GetAndClearPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = bufferBase->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, 0);
    EXPECT_EQ(retrievedEngineSize, 0);
    EXPECT_EQ(retrievedRouteFlag, 0);
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, HasPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
    
    bufferBase->SetPendingSpanSize(100, 200, 1);
    EXPECT_TRUE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, HasPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    bufferBase->SetPendingSpanSize(100, 200, 1);
    EXPECT_TRUE(bufferBase->HasPendingSpanSize());
    
    bufferBase->ClearPendingSpanSize();
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, ClearPendingSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    bufferBase->SetPendingSpanSize(1000, 2000, 3);
    EXPECT_TRUE(bufferBase->HasPendingSpanSize());
    
    bufferBase->ClearPendingSpanSize();
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = bufferBase->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, 0);
    EXPECT_EQ(retrievedEngineSize, 0);
    EXPECT_EQ(retrievedRouteFlag, 0);
}

HWTEST(OHAudioBufferBaseUnitTest, ClearPendingSpanSize_002, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    bufferBase->ClearPendingSpanSize();
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, PendingSpanSizeMultipleOperations_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 2048;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    bufferBase->SetPendingSpanSize(512, 1024, 1);
    EXPECT_TRUE(bufferBase->HasPendingSpanSize());
    
    uint64_t engineSize1 = 0;
    uint32_t routeFlag1 = 0;
    uint64_t spanSize1 = bufferBase->GetAndClearPendingSpanSize(engineSize1, routeFlag1);
    EXPECT_EQ(spanSize1, 512);
    EXPECT_EQ(engineSize1, 1024);
    EXPECT_EQ(routeFlag1, 1);
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
    
    bufferBase->SetPendingSpanSize(1024, 2048, 2);
    EXPECT_TRUE(bufferBase->HasPendingSpanSize());
    
    uint64_t engineSize2 = 0;
    uint32_t routeFlag2 = 0;
    uint64_t spanSize2 = bufferBase->GetAndClearPendingSpanSize(engineSize2, routeFlag2);
    EXPECT_EQ(spanSize2, 1024);
    EXPECT_EQ(engineSize2, 2048);
    EXPECT_EQ(routeFlag2, 2);
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, WakeFutexIfNeed_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    bufferBase->WakeFutex(IS_READY);
    bufferBase->WakeFutex(IS_NOT_READY);
    bufferBase->WakeFutex(IS_PRE_EXIT);
}

HWTEST(OHAudioBufferBaseUnitTest, InitBasicBufferInfo_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    EXPECT_FALSE(bufferBase->HasPendingSpanSize());
}

HWTEST(OHAudioBufferBaseUnitTest, LargeSpanSize_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 4096;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    uint64_t largeSpanSize = 8192;
    uint64_t largeEngineSize = 16384;
    uint32_t routeFlag = 5;
    
    bufferBase->SetPendingSpanSize(largeSpanSize, largeEngineSize, routeFlag);
    EXPECT_TRUE(bufferBase->HasPendingSpanSize());
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = bufferBase->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, largeSpanSize);
    EXPECT_EQ(retrievedEngineSize, largeEngineSize);
    EXPECT_EQ(retrievedRouteFlag, routeFlag);
}

HWTEST(OHAudioBufferBaseUnitTest, MaxRouteFlag_001, TestSize.Level1)
{
    uint32_t totalSizeInFrame = 1024;
    uint32_t byteSizePerFrame = 4;
    
    auto bufferBase = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);
    ASSERT_NE(bufferBase, nullptr);
    
    uint64_t spanSize = 256;
    uint64_t engineSize = 512;
    uint32_t maxRouteFlag = UINT32_MAX;
    
    bufferBase->SetPendingSpanSize(spanSize, engineSize, maxRouteFlag);
    
    uint64_t retrievedEngineSize = 0;
    uint32_t retrievedRouteFlag = 0;
    uint64_t retrievedSpanSize = bufferBase->GetAndClearPendingSpanSize(retrievedEngineSize, retrievedRouteFlag);
    
    EXPECT_EQ(retrievedSpanSize, spanSize);
    EXPECT_EQ(retrievedEngineSize, engineSize);
    EXPECT_EQ(retrievedRouteFlag, maxRouteFlag);
}

} // namespace AudioStandard
} // namespace OHOS