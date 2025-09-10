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
#include <gtest/gtest.h>
#include "offline_stream_in_server.h"
#include "audio_shared_memory.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
class OfflineStreamInServerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioSharedMemoryTest : public AudioSharedMemory {
public:
    uint8_t *GetBase() override { return nullptr; };
    size_t GetSize() override { return 0; };
    int GetFd() override { return 0; };
    std::string GetName() override { return "abc"; };
    bool Marshalling(Parcel &parcel) const override { return true; };
};

/**
* @tc.name  : Test OfflineStreamInServer::PrepareOfflineEffectChain()
* @tc.number: OfflineStreamInServer_001
* @tc.desc  : Test OfflineStreamInServer interface.
*/
HWTEST(OfflineStreamInServerUnitTest, OfflineStreamInServer_001, TestSize.Level1)
{
    auto offlineStreamInServer = std::make_shared<OfflineStreamInServer>();
    EXPECT_NE(offlineStreamInServer, nullptr);

    std::shared_ptr<AudioSharedMemory> inBuffer = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(inBuffer, nullptr);
    std::shared_ptr<AudioSharedMemory> outBuffer = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(outBuffer, nullptr);

    offlineStreamInServer->serverBufferIn_ = nullptr;
    offlineStreamInServer->serverBufferOut_ = nullptr;

    std::string chainName = "abc";
    offlineStreamInServer->CreateOfflineEffectChain(chainName);
    auto ret = offlineStreamInServer->PrepareOfflineEffectChain(inBuffer, outBuffer);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test OfflineStreamInServer::PrepareOfflineEffectChain()
* @tc.number: OfflineStreamInServer_002
* @tc.desc  : Test OfflineStreamInServer interface.
*/
HWTEST(OfflineStreamInServerUnitTest, OfflineStreamInServer_002, TestSize.Level1)
{
    auto offlineStreamInServer = std::make_shared<OfflineStreamInServer>();
    EXPECT_NE(offlineStreamInServer, nullptr);

    std::shared_ptr<AudioSharedMemory> inBuffer = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(inBuffer, nullptr);
    std::shared_ptr<AudioSharedMemory> outBuffer = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(outBuffer, nullptr);

    offlineStreamInServer->serverBufferIn_ = inBuffer;
    offlineStreamInServer->serverBufferOut_ = nullptr;

    std::string chainName = "abc";
    offlineStreamInServer->CreateOfflineEffectChain(chainName);
    auto ret = offlineStreamInServer->PrepareOfflineEffectChain(inBuffer, outBuffer);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : Test OfflineStreamInServer::PrepareOfflineEffectChain()
* @tc.number: OfflineStreamInServer_003
* @tc.desc  : Test OfflineStreamInServer interface.
*/
HWTEST(OfflineStreamInServerUnitTest, OfflineStreamInServer_003, TestSize.Level1)
{
    auto offlineStreamInServer = std::make_shared<OfflineStreamInServer>();
    EXPECT_NE(offlineStreamInServer, nullptr);

    std::shared_ptr<AudioSharedMemory> inBuffer = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(inBuffer, nullptr);
    std::shared_ptr<AudioSharedMemory> outBuffer = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(outBuffer, nullptr);

    offlineStreamInServer->serverBufferIn_ = inBuffer;
    offlineStreamInServer->serverBufferOut_ = outBuffer;

    std::string chainName = "abc";
    offlineStreamInServer->CreateOfflineEffectChain(chainName);
    auto ret = offlineStreamInServer->PrepareOfflineEffectChain(inBuffer, outBuffer);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test OfflineStreamInServer::PrepareOfflineEffectChain()
* @tc.number: OfflineStreamInServer_004
* @tc.desc  : Test OfflineStreamInServer interface.
*/
HWTEST(OfflineStreamInServerUnitTest, OfflineStreamInServer_004, TestSize.Level1)
{
    auto offlineStreamInServer = std::make_shared<OfflineStreamInServer>();
    EXPECT_NE(offlineStreamInServer, nullptr);

    std::shared_ptr<AudioSharedMemory> inBuffer = nullptr;
    std::shared_ptr<AudioSharedMemory> outBuffer = nullptr;

    auto ret = offlineStreamInServer->PrepareOfflineEffectChain(inBuffer, outBuffer); // effectChain is nullptr
    EXPECT_NE(ret, 0);
}
} // namespace AudioStandard
} // namespace OHOS