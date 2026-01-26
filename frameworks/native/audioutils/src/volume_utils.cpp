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
#define LOG_TAG "VolumeUtils"
#endif

#include "audio_common_log.h"
#include "audio_utils.h"
#include "parameters.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static const float VOLUME_DB_MIN = -std::numeric_limits<float>::epsilon();
    static const float VOLUME_DB_MAX = 1.0f + std::numeric_limits<float>::epsilon();
} // namespace

float VolumeUtils::enforcedToneVolume_ = -1.0f;
bool VolumeUtils::isLegacySetVolumeIgnored_ = false;

void VolumeUtils::InitEnforcedToneVolume()
{
    std::string srcVolume = system::GetParameter("const.multimedia.audio.enforced_tone_volume", "-1.0");
    bool isCovertSuccess = StringConverterFloat(srcVolume, enforcedToneVolume_);
    AUDIO_INFO_LOG("enforced tone volume srcVolume: %{public}s, covertSuccess: %{public}d, finalVolume: %{public}f",
        srcVolume.c_str(), isCovertSuccess, enforcedToneVolume_);
}

bool VolumeUtils::IsEnforcedToneVolumeFixed()
{
    AUDIO_DEBUG_LOG("enforced tone volume: %{public}f", enforcedToneVolume_);
    return enforcedToneVolume_ > VOLUME_DB_MIN && enforcedToneVolume_ < VOLUME_DB_MAX;
}

float VolumeUtils::GetEnforcedToneVolumeFixed()
{
    AUDIO_DEBUG_LOG("fixed enforced tone volume: %{public}f", enforcedToneVolume_);
    return enforcedToneVolume_;
}

void VolumeUtils::SetVolumeLegacyIgnored(const bool &isIgnored)
{
    AUDIO_INFO_LOG("set volume ignored for legacy: %{public}d", isIgnored);
    isLegacySetVolumeIgnored_ = isIgnored;
}

bool VolumeUtils::IsLegacySetVolumeIgnored()
{
    return isLegacySetVolumeIgnored_;
}
} // namespace AudioStandard
} // namespace OHOS