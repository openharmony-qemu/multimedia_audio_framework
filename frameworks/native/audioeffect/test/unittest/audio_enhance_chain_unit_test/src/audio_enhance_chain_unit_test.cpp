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

#ifndef LOG_TAG
#define LOG_TAG "AudioEnhanceChainUnitTest"
#endif

#include "audio_enhance_chain_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_enhance_chain.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
    const uint32_t LENGTH_NUM = 10;
    std::string scene = "scene";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(scene, algoParam, deviceAttr, true);
}

void AudioEnhanceChainUnitTest::SetUpTestCase(void) {}
void AudioEnhanceChainUnitTest::TearDownTestCase(void) {}
void AudioEnhanceChainUnitTest::SetUp(void) {}
void AudioEnhanceChainUnitTest::TearDown(void) {}

/**
* @tc.name   : Test ReleaseEnhanceChain API
* @tc.number : ReleaseEnhanceChain_001
* @tc.desc   : Test ReleaseEnhanceChain interface.
*/
HWTEST(AudioEnhanceChainUnitTest, ReleaseEnhanceChain_001, TestSize.Level1)
{
    AudioEffectLibrary *audioEffectLibrary = new AudioEffectLibrary();
    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(3);
    audioEnhanceChain->enhanceLibHandles_.push_back(audioEffectLibrary);
    audioEnhanceChain->ReleaseEnhanceChain();
    EXPECT_NE(nullptr, audioEnhanceChain);
}

/**
* @tc.name   : Test ReleaseEnhanceChain API
* @tc.number : ReleaseEnhanceChain_002
* @tc.desc   : Test ReleaseEnhanceChain interface.
*/
HWTEST(AudioEnhanceChainUnitTest, ReleaseEnhanceChain_002, TestSize.Level1)
{
    AudioEffectLibrary *audioEffectLibrary = new AudioEffectLibrary();
    AudioEffectInterface *audioEffectHandle = new AudioEffectInterface();
    audioEnhanceChain->standByEnhanceHandles_.push_back(&audioEffectHandle);
    audioEnhanceChain->enhanceLibHandles_.push_back(audioEffectLibrary);
    audioEnhanceChain->ReleaseEnhanceChain();
    EXPECT_NE(nullptr, audioEnhanceChain);
}

/**
* @tc.name   : Test ReleaseEnhanceChain API
* @tc.number : ReleaseEnhanceChain_003
* @tc.desc   : Test ReleaseEnhanceChain interface.
*/
HWTEST(AudioEnhanceChainUnitTest, ReleaseEnhanceChain_003, TestSize.Level1)
{
    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(3);
    audioEnhanceChain->enhanceLibHandles_ = std::vector<AudioEffectLibrary *>(3);
    audioEnhanceChain->ReleaseEnhanceChain();
    EXPECT_NE(nullptr, audioEnhanceChain);
}

/**
* @tc.name   : Test SetInputDevice API
* @tc.number : SetInputDevice_001
* @tc.desc   : Test SetInputDevice interface.
*/
HWTEST(AudioEnhanceChainUnitTest, SetInputDevice_001, TestSize.Level1)
{
    std::string inputDevice = "";
    std::string deviceName = "deviceName";
    int result = audioEnhanceChain->SetInputDevice(inputDevice, deviceName);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test SetInputDevice API
* @tc.number : SetInputDevice_002
* @tc.desc   : Test SetInputDevice interface.
*/
HWTEST(AudioEnhanceChainUnitTest, SetInputDevice_002, TestSize.Level1)
{
    std::string inputDevice = "inputDevice";
    std::string deviceName = "deviceName";
    int result = audioEnhanceChain->SetInputDevice(inputDevice, deviceName);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test AddEnhanceHandle API
* @tc.number : AddEnhanceHandle_001
* @tc.desc   : Test AddEnhanceHandle interface.
*/
HWTEST(AudioEnhanceChainUnitTest, AddEnhanceHandle_001, TestSize.Level1)
{
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    AudioEffectLibrary *libHandle = new AudioEffectLibrary();
    std::string enhance = "enhance";
    std::string property = "property";
    audioEnhanceChain->AddEnhanceHandle(handle, libHandle, enhance, property);
    EXPECT_NE(nullptr, audioEnhanceChain);
}

/**
* @tc.name   : Test AddEnhanceHandle API
* @tc.number : AddEnhanceHandle_002
* @tc.desc   : Test AddEnhanceHandle interface.
*/
HWTEST(AudioEnhanceChainUnitTest, AddEnhanceHandle_002, TestSize.Level1)
{
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    AudioEffectLibrary *libHandle = new AudioEffectLibrary();
    std::string enhance = "enhance";
    std::string property = "property";
    audioEnhanceChain->algoSupportedConfig_.sampleRate = 0;
    audioEnhanceChain->AddEnhanceHandle(handle, libHandle, enhance, property);
    EXPECT_NE(nullptr, audioEnhanceChain);
}

/**
* @tc.name   : Test GetAlgoConfig API
* @tc.number : GetAlgoConfig_001
* @tc.desc   : Test GetAlgoConfig interface.
*/
HWTEST(AudioEnhanceChainUnitTest, GetAlgoConfig_001, TestSize.Level1)
{
    AudioBufferConfig micConfig;
    AudioBufferConfig ecConfig;
    AudioBufferConfig micRefConfig;
    audioEnhanceChain->needEcFlag_ = true;
    audioEnhanceChain->needMicRefFlag_ = true;
    audioEnhanceChain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);
    EXPECT_NE(nullptr, audioEnhanceChain);
}

/**
* @tc.name   : Test GetOneFrameInputData API
* @tc.number : GetOneFrameInputData_001
* @tc.desc   : Test GetOneFrameInputData interface.
*/
HWTEST(AudioEnhanceChainUnitTest, GetOneFrameInputData_001, TestSize.Level1)
{
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    int result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test GetOneFrameInputData API
* @tc.number : GetOneFrameInputData_002
* @tc.desc   : Test GetOneFrameInputData interface.
*/
HWTEST(AudioEnhanceChainUnitTest, GetOneFrameInputData_002, TestSize.Level1)
{
    audioEnhanceChain->needEcFlag_ = true;
    audioEnhanceChain->needMicRefFlag_ = true;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->ecBuffer.push_back(1);
    enhanceBuffer->micBufferIn.push_back(1);
    enhanceBuffer->micRefBuffer.push_back(1);
    int result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test GetOneFrameInputData API
* @tc.number : GetOneFrameInputData_003
* @tc.desc   : Test GetOneFrameInputData interface.
*/
HWTEST(AudioEnhanceChainUnitTest, GetOneFrameInputData_003, TestSize.Level1)
{
    audioEnhanceChain->needEcFlag_ = false;
    audioEnhanceChain->needMicRefFlag_ = false;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->ecBuffer.push_back(1);
    enhanceBuffer->micBufferIn.push_back(1);
    enhanceBuffer->micRefBuffer.push_back(1);
    int result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test GetOneFrameInputData API
* @tc.number : GetOneFrameInputData_004
* @tc.desc   : Test GetOneFrameInputData interface.
*/
HWTEST(AudioEnhanceChainUnitTest, GetOneFrameInputData_004, TestSize.Level1)
{
    audioEnhanceChain->needMicRefFlag_ = true;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micRefBuffer.push_back(1);
    int result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test GetOneFrameInputData API
* @tc.number : GetOneFrameInputData_005
* @tc.desc   : Test GetOneFrameInputData interface.
*/
HWTEST(AudioEnhanceChainUnitTest, GetOneFrameInputData_005, TestSize.Level1)
{
    audioEnhanceChain->needMicRefFlag_ = false;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micRefBuffer.push_back(1);
    int result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ApplyEnhanceChain API
* @tc.number : ApplyEnhanceChain_001
* @tc.desc   : Test ApplyEnhanceChain interface.
*/
HWTEST(AudioEnhanceChainUnitTest, ApplyEnhanceChain_001, TestSize.Level1)
{
    audioEnhanceChain->deviceAttr_.needEc = false;
    audioEnhanceChain->deviceAttr_.needMicRef = false;
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    audioEnhanceChain->standByEnhanceHandles_.push_back(handle);
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    uint32_t length = LENGTH_NUM;
    int result = audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test ApplyEnhanceChain API
* @tc.number : ApplyEnhanceChain_002
* @tc.desc   : Test ApplyEnhanceChain interface.
*/
HWTEST(AudioEnhanceChainUnitTest, ApplyEnhanceChain_002, TestSize.Level1)
{
    audioEnhanceChain->deviceAttr_.needEc = false;
    audioEnhanceChain->deviceAttr_.needMicRef = false;
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    audioEnhanceChain->standByEnhanceHandles_.push_back(handle);

    audioEnhanceChain->needEcFlag_ = true;
    audioEnhanceChain->needMicRefFlag_ = true;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->ecBuffer.push_back(1);
    enhanceBuffer->micBufferIn.push_back(1);
    enhanceBuffer->micRefBuffer.push_back(1);
    uint32_t length = LENGTH_NUM;
    int result = audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test SetEnhanceProperty API
* @tc.number : SetEnhanceProperty_001
* @tc.desc   : Test SetEnhanceProperty interface.
*/
HWTEST(AudioEnhanceChainUnitTest, SetEnhanceProperty_001, TestSize.Level1)
{
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    audioEnhanceChain->standByEnhanceHandles_.push_back(handle);
    std::string enhance = "enhance";
    std::string property = "property";
    audioEnhanceChain->enhanceNames_.push_back(enhance);
    int result = audioEnhanceChain->SetEnhanceProperty(enhance, property);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test SetEnhanceProperty API
* @tc.number : SetEnhanceProperty_002
* @tc.desc   : Test SetEnhanceProperty interface.
*/
HWTEST(AudioEnhanceChainUnitTest, SetEnhanceProperty_002, TestSize.Level1)
{
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    audioEnhanceChain->standByEnhanceHandles_.push_back(handle);
    std::string enhance = "";
    std::string property = "property";
    int result = audioEnhanceChain->SetEnhanceProperty(enhance, property);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test SetEnhanceProperty API
* @tc.number : SetEnhanceProperty_003
* @tc.desc   : Test SetEnhanceProperty interface.
*/
HWTEST(AudioEnhanceChainUnitTest, SetEnhanceProperty_003, TestSize.Level1)
{
    std::string enhance = "";
    std::string property = "";
    int result = audioEnhanceChain->SetEnhanceProperty(enhance, property);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test SetPropertyToHandle API
* @tc.number : SetPropertyToHandle_001
* @tc.desc   : Test SetPropertyToHandle interface.
*/
HWTEST(AudioEnhanceChainUnitTest, SetPropertyToHandle_001, TestSize.Level1)
{
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    std::string property = "property";
    int result = audioEnhanceChain->SetPropertyToHandle(handle, property);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test SetPropertyToHandle API
* @tc.number : SetPropertyToHandle_002
* @tc.desc   : Test SetPropertyToHandle interface.
*/
HWTEST(AudioEnhanceChainUnitTest, SetPropertyToHandle_002, TestSize.Level1)
{
    AudioEffectInterface effectInterface = {
        [](AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer) -> int32_t {
            return 0;
        },
        [](AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
            AudioEffectTransInfo *replyInfo) -> int32_t {
            return 0;
        }
    };
    AudioEffectInterface* effectInterfacePtr = &effectInterface;
    AudioEffectHandle handle = &effectInterfacePtr;
    std::string property = "";
    int result = audioEnhanceChain->SetPropertyToHandle(handle, property);
    EXPECT_EQ(SUCCESS, result);
}
} // namespace AudioStandard
} // namespace OHOS