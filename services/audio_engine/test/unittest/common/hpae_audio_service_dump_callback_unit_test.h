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
#ifndef HPAE_AUDIO_SERVICE_DUMP_CALLBACK_UNIT_TEST_H
#define HPAE_AUDIO_SERVICE_DUMP_CALLBACK_UNIT_TEST_H
#include "audio_service_hpae_dump_callback.h"
 
namespace OHOS {
namespace AudioStandard {
class HpaeAudioServiceDumpCallbackUnitTest : public AudioServiceHpaeDumpCallback {
public:
    void OnDumpSinkInfoCb(std::string& dumpStr, int32_t result) override {}
    void OnDumpSourceInfoCb(std::string &dumpStr, int32_t result) override {}
    void OnDumpAllAvailableDeviceCb(int32_t result) override {}
    void OnDumpSinkInputsInfoCb(std::vector<HpaeInputOutputInfo> &sinkInputs, int32_t result) override
    {
        if (result == 0) {
            sinkInputs_.swap(sinkInputs);
        }
    }
    void OnDumpSourceOutputsInfoCb(std::vector<HpaeInputOutputInfo> &sourceOutputs, int32_t result) override
    {
        if (result == 0) {
            sourceOutputs_.swap(sourceOutputs);
        }
    }
    ~HpaeAudioServiceDumpCallbackUnitTest() override {}
    size_t GetSinkInputsSize()
    {
        return sinkInputs_.size();
    }
    size_t GetSourceOutputsSize()
    {
        return sourceOutputs_.size();
    }
private:
    std::vector<HpaeInputOutputInfo> sinkInputs_;
    std::vector<HpaeInputOutputInfo> sourceOutputs_;
};
}
}
#endif