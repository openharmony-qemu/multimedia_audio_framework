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
#define LOG_TAG "DataShareObserverCallBack"
#endif

#include "data_share_observer_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

DataShareObserverCallBack::DataShareObserverCallBack()
    : audioPolicyService_(AudioPolicyService::GetAudioPolicyService())
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
}

void DataShareObserverCallBack::OnChange()
{
    std::string devicesName = "";
    int32_t ret = audioPolicyService_.GetDeviceNameFromDataShareHelper(devicesName);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Local UpdateDisplayName init device failed");
    audioPolicyService_.SetDisplayName(devicesName, true);
}
} // namespace AudioStandard
} // namespace OHOS
