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

#ifndef HPAE_OFFLOAD_RENDERER_MANAGER_FUZZER_H
#define HPAE_OFFLOAD_RENDERER_MANAGER_FUZZER_H

#include "audio_info.h"
#include "audio_stream_info.h"
#include "i_capturer_stream.h"
#include "i_renderer_stream.h"
// Note: IStreamCallback is defined in i_renderer_stream.h
// Note: ICapturerStreamCallback is defined in i_capturer_stream.h
// Note: IEffectInstance is needed for CreateSessionEffect fuzz coverage
#include "i_effect_instance.h"
#include <atomic>

namespace OHOS {
namespace AudioStandard {

// Minimal stub of IEffectInstance to drive CreateSessionEffect success/failure
// paths in HpaeOffloadRendererManager::InsertSessionEffectNode without depending
// on the real algorithm library. Each virtual returns a benign value so the
// node layer does not crash when it inspects the instance pointer.
class StubEffectInstance : public HPAE::IEffectInstance {
public:
    StubEffectInstance() = default;
    ~StubEffectInstance() override = default;

    int32_t Release() override { return 0; }
    const std::string &GetEffectName() const override { return effectName_; }
    const AudioEffectConfig &GetAudioConfig() const override { return config_; }
    int32_t FeedInput(const uint8_t *data, size_t len) override { return 0; }
    int32_t ReadOutput(uint8_t *buf, size_t len, uint32_t *skipCount) override { return 0; }
    size_t GetInputLevel() const override { return 0; }
    void ClearBuffers() override {}
    int32_t StopEffect() override { return 0; }
    int32_t TriggerProcess() override { return 0; }
    int32_t Flush() override { return 0; }
    int32_t SetParameter(int32_t type, float value) override { return 0; }
    uint32_t GetPreheatFrames() override { return 0; }

private:
    std::string effectName_ {"fuzz_stub_effect"};
    AudioEffectConfig config_ {};
};

// Test callback class for renderer operations
class WriteFixedDataCb : public IStreamCallback, public std::enable_shared_from_this<WriteFixedDataCb> {
public:
    int32_t OnStreamData(AudioCallBackStreamInfo& callBackStremInfo) override;
    bool OnQueryUnderrun() override { return false; }
    void OnNotifyHdiData(const std::pair<uint64_t, TimePoint>& hdiPos) override {}

    explicit WriteFixedDataCb(AudioSampleFormat format) : format_(format)
    {}
    virtual ~WriteFixedDataCb()
    {}

private:
    int32_t writeNum_ = 0;
    AudioSampleFormat format_ = SAMPLE_F32LE;
};
 
class ReadDataCb : public ICapturerStreamCallback, public std::enable_shared_from_this<ReadDataCb> {
public:
    explicit ReadDataCb(const std::string &fileName);
    virtual ~ReadDataCb();
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo) override;
private:
    FILE *testFile_ = nullptr;
};

class TestCallbackWithDestruction : public IStreamCallback,
                                    public std::enable_shared_from_this<TestCallbackWithDestruction> {
public:
    virtual ~TestCallbackWithDestruction() = default;
    int32_t OnStreamData(AudioCallBackStreamInfo& callBackStremInfo) override
    {
        callbackInvokedCount_++;
        return SUCCESS;
    }

    bool OnQueryUnderrun() override
    {
        underrunQueryCount_++;
        return false;
    }

    void OnNotifyHdiData(const std::pair<uint64_t, TimePoint>& hdiPos) override
    {
        hdiNotifyCount_++;
    }

    void Reset()
    {
        callbackInvokedCount_ = 0;
        underrunQueryCount_ = 0;
        hdiNotifyCount_ = 0;
    }

    int32_t GetInvokedCount() const
    {
        return callbackInvokedCount_;
    }

    int32_t GetUnderrunQueryCount() const
    {
        return underrunQueryCount_;
    }

    int32_t GetHdiNotifyCount() const
    {
        return hdiNotifyCount_;
    }

private:
    std::atomic<int32_t> callbackInvokedCount_ {0};
    std::atomic<int32_t> underrunQueryCount_ {0};
    std::atomic<int32_t> hdiNotifyCount_ {0};
};

} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_OFFLOAD_RENDERER_MANAGER_FUZZER_H
