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
#ifndef AUXILIARY_SINK_AIUT_H
#define AUXILIARY_SINK_AIUT_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock_i_device_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/auxiliary_sink.h"

namespace OHOS {
namespace AudioStandard {

class AuxiliarySinkAiut : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

    // Mock helpers
    void InjectMockDeviceManager();
    void RestoreDeviceManager();
    void SetupMockCreateCognitionStreamSuccess();
    void SetupMockDestroyCognitionStreamSuccess();
    void SetupMockNotifyCognitionDataSuccess();
    void SetupMockForFullInit();

    // Test helpers
    IAudioSinkAttr CreateDefaultSinkAttr();
    IAudioSinkAttr CreateBtA2dpSinkAttr();
    std::shared_ptr<AuxiliarySink> CreateAndInitSink();
    void DeInitAndResetSink(std::shared_ptr<AuxiliarySink> &sink);

protected:
    std::shared_ptr<MockIDeviceManager> mockDeviceManager_;
    std::shared_ptr<IDeviceManager> originalDeviceManager_;
    int32_t mockMmapFd_ = -1;
    static constexpr int32_t MOCK_SINK_ID = 100;
    static constexpr int32_t MOCK_TOTAL_BUFFER_FRAMES = 1764;
    static constexpr int32_t MOCK_TRANSFER_FRAME_SIZE = 882;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUXILIARY_SINK_AIUT_H
