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

#ifndef AUDIO_POLICY_SERVICE_EXT_UNIT_TEST_H
#define AUDIO_POLICY_SERVICE_EXT_UNIT_TEST_H

#include "gtest/gtest.h"
#include "i_standard_sle_audio_operation_callback.h"

namespace OHOS {
namespace AudioStandard {

class AudioPolicyServiceExtUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

class MockSleAudioOperationCallback : public IStandardSleAudioOperationCallback {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {}

    void GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {}

    bool IsInBandRingOpen(const std::string &device) override
    {
        return true;
    }

    uint32_t GetSupportStreamType(const std::string &device) override
    {
        return 0;
    }

    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType) override
    {
        return SUCCESS;
    }

    int32_t StartPlaying(const std::string &device, uint32_t streamType) override
    {
        return SUCCESS;
    }

    int32_t StopPlaying(const std::string &device, uint32_t streamType) override
    {
        return SUCCESS;
    }

    int32_t ConnectAllowedProfiles(const std::string &remoteAddr) override
    {
        return SUCCESS;
    }

    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType) override
    {
        return SUCCESS;
    }

    int32_t SendUserSelection(const std::string &device, uint32_t streamType) override
    {
        return SUCCESS;
    }

    int32_t GetRenderPosition(const std::string &device, uint32_t &delayValue) override
    {
        return SUCCESS;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_SERVICE_EXT_UNIT_TEST_H