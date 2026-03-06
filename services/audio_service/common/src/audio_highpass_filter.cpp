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
#ifndef LOG_TAG
#define LOG_TAG "AudioHighPassFilter"
#endif

#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <cinttypes>
#include "audio_highpass_filter.h"
#include "audio_service_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
#if (defined(__aarch64__) || defined(__x86_64__))
const char *HIGHPASS_FILTER_LIB_NAME = "/system/lib64/libhighpass_filter.z.so";
#else
const char *HIGHPASS_FILTER_LIB_NAME = "/system/lib/libhighpass_filter.z.so";
#endif
const char *HIGHPASS_FILTER_FUNC_NAME = "AudioHighpassFilterClassCreate";

AudioHighPassFilter::AudioHighPassFilter() : handle_(nullptr), filter_(nullptr)
{
    CHECK_AND_RETURN_LOG(access(HIGHPASS_FILTER_LIB_NAME, R_OK) == 0, "so file not exist");
    handle_ = ::dlopen(HIGHPASS_FILTER_LIB_NAME, RTLD_NOW);
    CHECK_AND_RETURN_LOG(handle_ != nullptr, "dlopen failed check so file exists");
    AudioHighpassFilterClassCreateFunc *createFilterFunc =
        reinterpret_cast<AudioHighpassFilterClassCreateFunc *>(dlsym(handle_, HIGHPASS_FILTER_FUNC_NAME));
    CHECK_AND_RETURN_LOG(createFilterFunc != nullptr, "dlsym failed.check so has this function");
    int32_t ret = createFilterFunc(&filter_);
    if (ret != 0 || filter_ == nullptr) {
        AUDIO_ERR_LOG("create HighPassFilter instance failed, ret=%{public}d", ret);
        dlclose(handle_);
        handle_ = nullptr;
    }
}

AudioHighPassFilter::~AudioHighPassFilter()
{
    if (filter_) {
        delete filter_;
        filter_ = nullptr;
    }
    if (handle_) {
#ifndef TEST_COVERAGE
        dlclose(handle_);
#endif
        handle_ = nullptr;
    }
}

int32_t AudioHighPassFilter::InitFilter(const int32_t &channels)
{
    CHECK_AND_RETURN_RET_LOG(filter_ != nullptr, ERR_INVALID_HANDLE, "filter is nullptr");
    CHECK_AND_RETURN_RET_LOG(channels > 0, ERR_INVALID_PARAM, "input channel number is invalid");
    int32_t ret = filter_->Init(channels);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_INVALID_HANDLE, "init filter failed, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t AudioHighPassFilter::ApplyFilter(const std::vector<float> &samples, std::vector<float> &result)
{
    CHECK_AND_RETURN_RET_LOG(filter_ != nullptr, ERR_INVALID_HANDLE, "filter is nullptr");
    CHECK_AND_RETURN_RET_LOG(!samples.empty(), ERR_INVALID_PARAM, "input samples is empty");
    CHECK_AND_RETURN_RET_LOG(samples.size() == result.size(), ERR_INVALID_PARAM,
        "samples size(%{public}zu) not match result size(%{public}zu)", samples.size(), result.size());
    int32_t ret = filter_->Apply(samples, result);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "apply filter failed, ret: %{public}d", ret);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS