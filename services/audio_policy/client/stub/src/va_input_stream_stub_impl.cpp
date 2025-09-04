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
#define LOG_TAG "VAInputStreamStubImpl"
#endif

#include "va_input_stream_stub_impl.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
VAInputStreamStubImpl::VAInputStreamStubImpl()
{}

VAInputStreamStubImpl::~VAInputStreamStubImpl()
{}

int32_t VAInputStreamStubImpl::SetVAInputStreamCallback(
    const std::shared_ptr<VAInputStreamCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM);
    vaInputStreamCallback_ = callback;
    return SUCCESS;
}

int32_t VAInputStreamStubImpl::GetStreamProperty(VAAudioStreamProperty& streamProp)
{
    CHECK_AND_RETURN_RET_LOG(vaInputStreamCallback_ != nullptr, ERROR);
    vaInputStreamCallback_->GetStreamProperty(streamProp);
    return SUCCESS;
}

int32_t VAInputStreamStubImpl::RequestSharedMem(const VASharedMemInfo& memInfo)
{
    CHECK_AND_RETURN_RET_LOG(vaInputStreamCallback_ != nullptr, ERROR);
    vaInputStreamCallback_->RequestSharedMem(memInfo);
    return SUCCESS;
}

int32_t VAInputStreamStubImpl::Start()
{
    CHECK_AND_RETURN_RET_LOG(vaInputStreamCallback_ != nullptr, ERROR);
    vaInputStreamCallback_->Start();
    return SUCCESS;
}

int32_t VAInputStreamStubImpl::Stop()
{
    CHECK_AND_RETURN_RET_LOG(vaInputStreamCallback_ != nullptr, ERROR);
    vaInputStreamCallback_->Stop();
    return SUCCESS;
}

int32_t VAInputStreamStubImpl::Close()
{
    CHECK_AND_RETURN_RET_LOG(vaInputStreamCallback_ != nullptr, ERROR);
    vaInputStreamCallback_->Close();
    return SUCCESS;
}


int32_t VAInputStreamStubImpl::GetCapturePosition(uint64_t &attr_1, uint64_t &attr_2)
{
    CHECK_AND_RETURN_RET_LOG(vaInputStreamCallback_ != nullptr, ERROR);
    vaInputStreamCallback_->GetCapturePosition(attr_1, attr_2);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS