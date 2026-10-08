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

#ifndef AUDIO_SUITE_MANAGER_TEST_COMMON_H
#define AUDIO_SUITE_MANAGER_TEST_COMMON_H

#include "audio_suite_node.h"
#include <unordered_map>
class AudioNodeTestImpl : public OHOS::AudioStandard::AudioSuite::AudioNode {
public:
    explicit AudioNodeTestImpl(OHOS::AudioStandard::AudioSuite::AudioNodeType nodeType)
        : OHOS::AudioStandard::AudioSuite::AudioNode(nodeType)
    {
    }
    virtual ~AudioNodeTestImpl() = default;
    int32_t DoProcess(uint32_t needDataLength) override
    {
        return 0;
    }
    int32_t Flush() override
    {
        return 0;
    }
    int32_t Connect(const std::shared_ptr<OHOS::AudioStandard::AudioSuite::AudioNode> &preNode) override
    {
        if (!preNode) {
            return -1;
        }
        return 0;
    }
    int32_t DisConnect(const std::shared_ptr<OHOS::AudioStandard::AudioSuite::AudioNode> &preNode) override
    {
        if (!preNode) {
            return -1;
        }
        return 0;
    }
};

class SuiteInputNodeRequestDataCallBackFinishedTestImpl
    : public OHOS::AudioStandard::AudioSuite::InputNodeRequestDataCallBack {
public:
    ~SuiteInputNodeRequestDataCallBackFinishedTestImpl() = default;
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) override

    {
        (void)audioData;
        (void)audioDataSize;
        if (finished != nullptr) {
            *finished = true;
        }
        return 0;
    }
};

class SuiteInputNodeRequestDataCallBackTestImpl
    : public SuiteInputNodeRequestDataCallBackFinishedTestImpl::InputNodeRequestDataCallBack {
public:
    ~SuiteInputNodeRequestDataCallBackTestImpl() = default;
    int32_t OnRequestDataCallBack(void *audioData, int32_t audioDataSize, bool *finished) override
    {
        (void)audioData;
        (void)audioDataSize;
        (void)finished;
        return 0;
    }
};

class AudioSuiteEngineManagerUnitTest : public testing::Test {
public:
    void SetUp()
    {
        if (!OHOS::AudioStandard::AudioSuite::AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
    };
    void TearDown() {};
};

#endif // AUDIO_SUITE_MANAGER_TEST_COMMON_H
