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
#ifndef PRO_AUDIO_SERVICE_ADAPTER_UNIT_TEST_H
#define PRO_AUDIO_SERVICE_ADAPTER_UNIT_TEST_H
#include <functional>
#include "gtest/gtest.h"
#include "pro_audio_service_adapter_impl.h"
namespace OHOS {
namespace AudioStandard {
class ProAudioServiceCallbackTest : public AudioServiceAdapterCallback {
public:
    ProAudioServiceCallbackTest() {}
    ~ProAudioServiceCallbackTest()
    {
        AUDIO_WARNING_LOG("Destructor ProAudioServiceCallbackTest");
    }
    void OnAudioStreamRemoved(const uint64_t sessionId)
    {}
    void OnSetVolumeDbCb()
    {}
};

class ProAudioServiceAdapterUnitTest : public testing::Test {
public:
    ProAudioServiceAdapterUnitTest();
    ~ProAudioServiceAdapterUnitTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
    AudioModuleInfo InitSinkAudioModeInfo();
    AudioModuleInfo InitSourceAudioModeInfo();
private:
    void Init();
protected:
   std::shared_ptr<AudioServiceAdapter> impl_;
   int32_t engineFlag_;
};
}  // namespace AudioStandard
}  // namespace OHOS

#endif
