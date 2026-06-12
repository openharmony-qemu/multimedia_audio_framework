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

#include <chrono>
#include "securec.h"
#include "gmock/gmock.h"
#include "audio_renderer_firstFrame_mock_test.h"
#include "audio_renderer_private.h"

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;
using namespace OHOS::AudioStandard::FirstFrameMockUT;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test AudioRendererFirstFrameWritingCallback Mock API
 * @tc.number: Audio_Renderer_FirstFrameWritingCallback_Mock_001
 * @tc.desc  : Test MockAudioRendererFirstFrameWritingCallback::OnFirstFrameWriting is called correctly
 */
HWTEST(AudioRendererFirstFrameMockTest, Audio_Renderer_FirstFrameWritingCallback_Mock_001, TestSize.Level2)
{
    auto mockCallback = std::make_shared<MockAudioRendererFirstFrameWritingCallback>();
    EXPECT_NE(mockCallback, nullptr);

    uint64_t testLatency = 1000;

    EXPECT_CALL(*mockCallback, OnFirstFrameWriting(testLatency))
        .Times(1)
        .WillOnce([testLatency](uint64_t latency) {
            EXPECT_EQ(latency, testLatency);
            g_firstFrameLatency = latency;
            g_firstFrameCalled = true;
        });

    mockCallback->OnFirstFrameWriting(testLatency);
    EXPECT_EQ(g_firstFrameLatency, testLatency);
    EXPECT_TRUE(g_firstFrameCalled);

    g_firstFrameLatency = 0;
    g_firstFrameCalled = false;
}

/**
 * @tc.name  : Test Mock FirstFrameWritingCallback with Latency Value
 * @tc.number: Audio_Renderer_FirstFrameWritingCallback_Mock_002
 * @tc.desc  : Test MockAudioRendererFirstFrameWritingCallback with different latency values
 */
HWTEST(AudioRendererFirstFrameMockTest, Audio_Renderer_FirstFrameWritingCallback_Mock_002, TestSize.Level2)
{
    auto mockCallback = std::make_shared<MockAudioRendererFirstFrameWritingCallback>();
    EXPECT_NE(mockCallback, nullptr);

    uint64_t latency1 = 500;
    uint64_t latency2 = 1500;

    uint64_t actualLatency1 = 0;
    uint64_t actualLatency2 = 0;

    EXPECT_CALL(*mockCallback, OnFirstFrameWriting(latency1))
        .Times(1)
        .WillOnce([&actualLatency1](uint64_t latency) {
            actualLatency1 = latency;
        });

    EXPECT_CALL(*mockCallback, OnFirstFrameWriting(latency2))
        .Times(1)
        .WillOnce([&actualLatency2](uint64_t latency) {
            actualLatency2 = latency;
        });

    mockCallback->OnFirstFrameWriting(latency1);
    mockCallback->OnFirstFrameWriting(latency2);

    EXPECT_EQ(actualLatency1, latency1);
    EXPECT_EQ(actualLatency2, latency2);
}

/**
 * @tc.name  : Test AudioRenderer with Mock FirstFrameWritingCallback End-to-End
 * @tc.number: Audio_Renderer_FirstFrameWritingCallback_E2E_001
 * @tc.desc  : Test FirstFrameWritingCallback triggered in CALLBACK mode end-to-end flow
 */
HWTEST(AudioRendererFirstFrameMockTest, Audio_Renderer_FirstFrameWritingCallback_E2E_001, TestSize.Level2)
{
    AudioRendererOptions rendererOptions;
    AudioRendererFirstFrameMockTest::InitializeRendererOptions(rendererOptions);
    
    std::shared_ptr<AudioRenderer> audioRenderer = AudioRenderer::CreateRenderer(rendererOptions);
    ASSERT_NE(audioRenderer, nullptr);

    int32_t ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(ret, SUCCESS);

    auto mockFirstFrameCallback = std::make_shared<MockAudioRendererFirstFrameWritingCallback>();
    EXPECT_NE(mockFirstFrameCallback, nullptr);

    ret = audioRenderer->SetRendererFirstFrameWritingCallback(mockFirstFrameCallback);
    EXPECT_EQ(ret, SUCCESS);

    auto mockWriteCallback = std::make_shared<MockWriteCallback>(audioRenderer);
    EXPECT_NE(mockWriteCallback, nullptr);

    ret = audioRenderer->SetRendererWriteCallback(mockWriteCallback);
    EXPECT_EQ(ret, SUCCESS);

    g_firstFrameCalled = false;
    g_firstFrameLatency = 0;
    
    EXPECT_CALL(*mockFirstFrameCallback, OnFirstFrameWriting(_))
        .Times(AtLeast(1))
        .WillRepeatedly([](uint64_t latency) {
            g_firstFrameCalled = true;
            g_firstFrameLatency = latency;
            EXPECT_GT(latency, 0u);
            EXPECT_LT(latency, 10000u);
        });

    bool isStarted = audioRenderer->Start();
    EXPECT_TRUE(isStarted);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(g_firstFrameCalled);
    EXPECT_GT(g_firstFrameLatency, 0u);

    audioRenderer->Release();

    g_firstFrameLatency = 0;
    g_firstFrameCalled = false;
}

/**
 * @tc.name  : Test FirstFrameWritingCallback Triggered Only Once
 * @tc.number: Audio_Renderer_FirstFrameWritingCallback_E2E_002
 * @tc.desc  : Test FirstFrameWritingCallback is triggered only once in multiple writes
 */
HWTEST(AudioRendererFirstFrameMockTest, Audio_Renderer_FirstFrameWritingCallback_E2E_002, TestSize.Level2)
{
    AudioRendererOptions rendererOptions;
    AudioRendererFirstFrameMockTest::InitializeRendererOptions(rendererOptions);
    
    std::shared_ptr<AudioRenderer> audioRenderer = AudioRenderer::CreateRenderer(rendererOptions);
    ASSERT_NE(audioRenderer, nullptr);

    int32_t ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(ret, SUCCESS);

    auto mockFirstFrameCallback = std::make_shared<MockAudioRendererFirstFrameWritingCallback>();
    EXPECT_NE(mockFirstFrameCallback, nullptr);

    ret = audioRenderer->SetRendererFirstFrameWritingCallback(mockFirstFrameCallback);
    EXPECT_EQ(ret, SUCCESS);

    auto mockWriteCallback = std::make_shared<MockWriteCallback>(audioRenderer);
    EXPECT_NE(mockWriteCallback, nullptr);

    ret = audioRenderer->SetRendererWriteCallback(mockWriteCallback);
    EXPECT_EQ(ret, SUCCESS);

    EXPECT_CALL(*mockFirstFrameCallback, OnFirstFrameWriting(_))
        .Times(1);

    bool isStarted = audioRenderer->Start();
    EXPECT_TRUE(isStarted);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    audioRenderer->Release();

    g_firstFrameLatency = 0;
    g_firstFrameCalled = false;
}

/**
 * @tc.name  : Test FirstFrameWritingCallback Latency Parameter Correctness
 * @tc.number: Audio_Renderer_FirstFrameWritingCallback_E2E_003
 * @tc.desc  : Test FirstFrameWritingCallback receives correct latency from system
 */
HWTEST(AudioRendererFirstFrameMockTest, Audio_Renderer_FirstFrameWritingCallback_E2E_003, TestSize.Level2)
{
    AudioRendererOptions rendererOptions;
    AudioRendererFirstFrameMockTest::InitializeRendererOptions(rendererOptions);
    
    std::shared_ptr<AudioRenderer> audioRenderer = AudioRenderer::CreateRenderer(rendererOptions);
    ASSERT_NE(audioRenderer, nullptr);

    int32_t ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(ret, SUCCESS);

    auto mockFirstFrameCallback = std::make_shared<MockAudioRendererFirstFrameWritingCallback>();
    ret = audioRenderer->SetRendererFirstFrameWritingCallback(mockFirstFrameCallback);
    EXPECT_EQ(ret, SUCCESS);

    auto mockWriteCallback = std::make_shared<MockWriteCallback>(audioRenderer);
    ret = audioRenderer->SetRendererWriteCallback(mockWriteCallback);
    EXPECT_EQ(ret, SUCCESS);

    g_firstFrameLatency = 0;
    
    EXPECT_CALL(*mockFirstFrameCallback, OnFirstFrameWriting(_))
        .Times(1)
        .WillOnce([](uint64_t latency) {
            g_firstFrameLatency = latency;
            EXPECT_GE(latency, 20u);
            EXPECT_LE(latency, 500000u);
        });

    bool isStarted = audioRenderer->Start();
    EXPECT_TRUE(isStarted);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_GT(g_firstFrameLatency, 0u);

    audioRenderer->Release();

    g_firstFrameLatency = 0;
    g_firstFrameCalled = false;
}

/**
 * @tc.name  : Test FirstFrameWritingCallback in Static Buffer Mode
 * @tc.number: Audio_Renderer_FirstFrameWritingCallback_StaticBuffer_E2E_001
 * @tc.desc  : Test FirstFrameWritingCallback triggered in STATIC mode with AudioSharedMemory
 */
HWTEST(AudioRendererFirstFrameMockTest, Audio_Renderer_FirstFrameWritingCallback_StaticBuffer_E2E_001, TestSize.Level2)
{
    AudioRendererOptions rendererOptions;
    AudioRendererFirstFrameMockTest::InitializeRendererOptions(rendererOptions);
    
    size_t bufferSize = 1024;
    auto sharedMemory = AudioSharedMemory::CreateFromLocal(bufferSize, "test_static_buffer");
    ASSERT_NE(sharedMemory, nullptr);
    
    uint8_t* bufferBase = sharedMemory->GetBase();
    if (bufferBase != nullptr) {
        memset_s(bufferBase, bufferSize, 0, bufferSize);
    }
    
    class StaticBufferEventCallbackTest : public StaticBufferEventCallback {
    public:
        void OnStaticBufferEvent(StaticBufferEventId eventId) override {}
    };
    auto staticCallback = std::make_shared<StaticBufferEventCallbackTest>();
    ASSERT_NE(staticCallback, nullptr);
    
    std::shared_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(
        rendererOptions, sharedMemory, staticCallback);
    ASSERT_NE(audioRenderer, nullptr);
    
    auto mockFirstFrameCallback = std::make_shared<MockAudioRendererFirstFrameWritingCallback>();
    ASSERT_NE(mockFirstFrameCallback, nullptr);
    
    int32_t ret = audioRenderer->SetRendererFirstFrameWritingCallback(mockFirstFrameCallback);
    EXPECT_EQ(ret, SUCCESS);
    
    EXPECT_CALL(*mockFirstFrameCallback, OnFirstFrameWriting(_))
        .Times(1)
        .WillOnce([](uint64_t latency) {
            g_firstFrameCalled = true;
            g_firstFrameLatency = latency;
            EXPECT_GT(latency, 0u);
            EXPECT_LT(latency, 10000u);
        });
    
    bool isStarted = audioRenderer->Start();
    EXPECT_TRUE(isStarted);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_TRUE(g_firstFrameCalled);
    EXPECT_GT(g_firstFrameLatency, 0u);
    
    audioRenderer->Release();
    g_firstFrameLatency = 0;
    g_firstFrameCalled = false;
}

} // namespace AudioStandard
} // namespace OHOS