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
#ifndef HPAE_MANAGER_UNIT_TEST_H
#define HPAE_MANAGER_UNIT_TEST_H
#include "gtest/gtest.h"
#include "hpae_manager.h"
#include "hpae_info.h"
#include "hpae_audio_service_callback_unit_test.h"
 
namespace OHOS {
namespace AudioStandard {
class HpaeManagerUnitTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
 
protected:
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_;
 
protected:
    void WaitForMsgProcessing();
    AudioModuleInfo GetSinkAudioModeInfo();
    AudioModuleInfo GetSourceAudioModeInfo();
    HPAE::HpaeStreamInfo GetRenderStreamInfo();
    HPAE::HpaeStreamInfo GetCaptureStreamInfo();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif