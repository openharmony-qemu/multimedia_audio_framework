/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "PolicyProviderStub"
#endif

#include "policy_provider_stub.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

PolicyProviderWrapper::~PolicyProviderWrapper()
{
    policyWorker_ = nullptr;
}

PolicyProviderWrapper::PolicyProviderWrapper(IPolicyProvider *policyWorker) : policyWorker_(policyWorker)
{
}

int32_t PolicyProviderWrapper::GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->GetProcessDeviceInfo(config, lockFlag, deviceInfo);
}

int32_t PolicyProviderWrapper::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->InitSharedVolume(buffer);
}

int32_t PolicyProviderWrapper::NotifyCapturerAdded(const AudioCapturerInfo &capturerInfo,
    const AudioStreamInfo &streamInfo, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
}

int32_t PolicyProviderWrapper::NotifyWakeUpCapturerRemoved()
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->NotifyWakeUpCapturerRemoved();
}

int32_t PolicyProviderWrapper::IsAbsVolumeSupported(bool &isSupported)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    isSupported = policyWorker_->IsAbsVolumeSupported();
    return SUCCESS;
}

int32_t PolicyProviderWrapper::OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize,
    uint32_t &timeStamp)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
}

int32_t PolicyProviderWrapper::NearlinkGetRenderPosition(uint32_t &delayValue)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->NearlinkGetRenderPosition(delayValue);
}

int32_t PolicyProviderWrapper::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->GetAndSaveClientType(uid, bundleName);
}

int32_t PolicyProviderWrapper::GetMaxRendererInstances(int32_t &maxInstances)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    maxInstances = policyWorker_->GetMaxRendererInstances();
    return SUCCESS;
}

int32_t PolicyProviderWrapper::IsSupportInnerCaptureOffload(bool &isSupported)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    isSupported = policyWorker_->IsSupportInnerCaptureOffload();
    return SUCCESS;
}

int32_t PolicyProviderWrapper::NotifyCapturerRemoved(uint64_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->NotifyCapturerRemoved(sessionId);
}

int32_t PolicyProviderWrapper::LoadModernInnerCapSink(int32_t innerCapId)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->LoadModernInnerCapSink(innerCapId);
#else
    (void)innerCapId;
    return AUDIO_ERR;
#endif
}

int32_t PolicyProviderWrapper::UnloadModernInnerCapSink(int32_t innerCapId)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->UnloadModernInnerCapSink(innerCapId);
#else
    (void)innerCapId;
    return AUDIO_ERR;
#endif
}

int32_t PolicyProviderWrapper::LoadModernOffloadCapSource()
{
#ifdef HAS_FEATURE_INNERCAPTURER
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->LoadModernOffloadCapSource();
#else
    return AUDIO_ERR;
#endif
}

int32_t PolicyProviderWrapper::UnloadModernOffloadCapSource()
{
#ifdef HAS_FEATURE_INNERCAPTURER
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->UnloadModernOffloadCapSource();
#else
    return AUDIO_ERR;
#endif
}

int32_t PolicyProviderWrapper::ClearAudioFocusBySessionID(int32_t sessionID)
{
    CHECK_AND_RETURN_RET_LOG(policyWorker_ != nullptr, AUDIO_INIT_FAIL, "policyWorker_ is null");
    return policyWorker_->ClearAudioFocusBySessionID(sessionID);
}
} // namespace AudioStandard
} // namespace OHOS
