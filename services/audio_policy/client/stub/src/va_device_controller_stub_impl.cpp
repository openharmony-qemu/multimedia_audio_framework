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
#define LOG_TAG "VADeviceControllerStubImpl"
#endif

#include "va_device_controller_stub_impl.h"
#include "va_input_stream_stub_impl.h"

#include "audio_errors.h"
#include "audio_policy_log.h"


namespace OHOS {
namespace AudioStandard {
VADeviceControllerStubImpl::VADeviceControllerStubImpl()
{}

VADeviceControllerStubImpl::~VADeviceControllerStubImpl()
{}

int32_t VADeviceControllerStubImpl::SetVADeviceControllerCallback(
    const std::shared_ptr<VADeviceControllerCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR);
    std::lock_guard<std::mutex> lock(vaDeviceControllerMutex_);
    vaDeviceControllerCallback_ = callback;

    return SUCCESS;
}

int32_t VADeviceControllerStubImpl::OpenInputStream(const VAAudioStreamProperty &prop,
                                                    const VAInputStreamAttribute &attr,
                                                    sptr<IRemoteObject> &inputStream)
{
    std::lock_guard<std::mutex> lock(vaDeviceControllerMutex_);
    CHECK_AND_RETURN_RET_LOG(vaDeviceControllerCallback_ != nullptr, ERROR);
    lock.unlock();
    
    std::shared_ptr<VAInputStreamCallback> inputStreamCallback;
    vaDeviceControllerCallback_->OpenInputStream(prop, attr, inputStreamCallback);
    CHECK_AND_RETURN_RET_LOG(inputStreamCallback != nullptr, ERROR);
    auto vaInputStreamStubImpl = sptr<VAInputStreamStubImpl>::MakeSptr();
    CHECK_AND_RETURN_RET_LOG(vaInputStreamStubImpl != nullptr, ERROR);
    vaInputStreamStubImpl->SetVAInputStreamCallback(inputStreamCallback);
    inputStream = vaInputStreamStubImpl->AsObject();
    if (inputStream == nullptr) {
        AUDIO_ERR_LOG("weird, inputStream is nullptr");
        return ERROR;
    }
    return SUCCESS;
}

int32_t VADeviceControllerStubImpl::GetParameters(const std::string& key, std::string& value)
{
    std::lock_guard<std::mutex> lock(vaDeviceControllerMutex_);
    CHECK_AND_RETURN_RET_LOG(vaDeviceControllerCallback_ != nullptr, ERROR);

    vaDeviceControllerCallback_->GetParameters(key, value);

    return SUCCESS;
}

int32_t VADeviceControllerStubImpl::SetParameters(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(vaDeviceControllerMutex_);
    CHECK_AND_RETURN_RET_LOG(vaDeviceControllerCallback_ != nullptr, ERROR);
    
    vaDeviceControllerCallback_->SetParameters(key, value);

    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS