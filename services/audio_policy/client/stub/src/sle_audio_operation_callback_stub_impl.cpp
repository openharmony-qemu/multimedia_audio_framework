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

#ifndef LOG_TAG
#define LOG_TAG "SleAudioOperationCallbackStubImpl"
#endif

#include "sle_audio_operation_callback_stub_impl.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
SleAudioOperationCallbackStubImpl::SleAudioOperationCallbackStubImpl()
{
}

SleAudioOperationCallbackStubImpl::~SleAudioOperationCallbackStubImpl()
{
}

int32_t SleAudioOperationCallbackStubImpl::SetSleAudioOperationCallback(
    const std::weak_ptr<SleAudioOperationCallback> &callback)
{
    std::lock_guard<std::mutex> lock(sleAudioOperationCallbackMutex_);
    sleAudioOperationCallback_ = callback;
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR_INVALID_PARAM,
        "sleAudioOperationCallback_ is nullptr");
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR,
        "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    sleAudioOperationCallback->GetSleAudioDeviceList(devices);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR,
        "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    sleAudioOperationCallback->GetSleVirtualAudioDeviceList(devices);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::IsInBandRingOpen(const std::string &device, bool& ret)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, false, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    ret = sleAudioOperationCallback->IsInBandRingOpen(device);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::GetSupportStreamType(const std::string &device, uint32_t& retType)
{
    uint32_t streamType = 0;
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, streamType, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    retType = sleAudioOperationCallback->GetSupportStreamType(device);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::SetActiveSinkDevice(
    const std::string &device, uint32_t streamType, int32_t& ret)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    ret = sleAudioOperationCallback->SetActiveSinkDevice(device, streamType);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::StartPlaying(const std::string &device, uint32_t streamType, int32_t& ret)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    ret = sleAudioOperationCallback->StartPlaying(device, streamType);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::StopPlaying(const std::string &device, uint32_t streamType, int32_t& ret)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    ret = sleAudioOperationCallback->StopPlaying(device, streamType);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::ConnectAllowedProfiles(const std::string &remoteAddr, int32_t& ret)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    ret = sleAudioOperationCallback->ConnectAllowedProfiles(remoteAddr);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::SetDeviceAbsVolume(
    const std::string &remoteAddr, uint32_t volume, uint32_t streamType, int32_t& ret)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    ret = sleAudioOperationCallback->SetDeviceAbsVolume(remoteAddr, volume, streamType);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::SendUserSelection(const std::string &device, uint32_t streamType,
    int32_t &ret)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    ret = sleAudioOperationCallback->SendUserSelection(device, streamType);
    return SUCCESS;
}

int32_t SleAudioOperationCallbackStubImpl::GetRenderPosition(const std::string &device, uint32_t &delayValue)
{
    std::unique_lock lock(sleAudioOperationCallbackMutex_);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback = sleAudioOperationCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR, "sleAudioOperationCallback_ is nullptr");
    lock.unlock();

    return sleAudioOperationCallback->GetRenderPosition(device, delayValue);
}

} // namespace AudioStandard
} // namespace OHOS
