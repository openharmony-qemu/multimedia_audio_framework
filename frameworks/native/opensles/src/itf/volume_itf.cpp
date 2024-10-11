/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <common.h>

using namespace OHOS::AudioStandard;

static SLresult SetVolumeLevel(SLVolumeItf self, SLmillibel level)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IVolume *thiz = (IVolume *)self;
    AudioPlayerAdapter::GetInstance()->SetVolumeLevelAdapter(thiz->mId, level);
    return SL_RESULT_SUCCESS;
}

static SLresult GetVolumeLevel(SLVolumeItf self, SLmillibel *level)
{
    if (self == nullptr || level == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IVolume *thiz = (IVolume *)self;
    AudioPlayerAdapter::GetInstance()->GetVolumeLevelAdapter(thiz->mId, level);
    return SL_RESULT_SUCCESS;
}

static SLresult GetMaxVolumeLevel(SLVolumeItf self, SLmillibel *maxLevel)
{
    if (self == nullptr || maxLevel == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IVolume *thiz = (IVolume *)self;
    AudioPlayerAdapter::GetInstance()->GetMaxVolumeLevelAdapter(thiz->mId, maxLevel);
    return SL_RESULT_SUCCESS;
}

static SLresult SetMute(SLVolumeItf self, SLboolean state)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetMute(SLVolumeItf self, SLboolean *state)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult EnableStereoPosition(SLVolumeItf self, SLboolean enable)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult IsEnabledStereoPosition(SLVolumeItf self, SLboolean *pEnable)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetStereoPosition(SLVolumeItf self, SLpermille stereoPosition)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetStereoPosition(SLVolumeItf self, SLpermille *pStereoPosition)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLVolumeItf_ VolumeItf = {
    SetVolumeLevel,
    GetVolumeLevel,
    GetMaxVolumeLevel,
    SetMute,
    GetMute,
    EnableStereoPosition,
    IsEnabledStereoPosition,
    SetStereoPosition,
    GetStereoPosition
};

void IVolumeInit(void *self, SLuint32 id)
{
    IVolume *thiz = (IVolume *) self;
    thiz->mItf = &VolumeItf;
    thiz->mId = id;
}
