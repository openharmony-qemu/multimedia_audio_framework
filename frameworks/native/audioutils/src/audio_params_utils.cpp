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
#define LOG_TAG "AudioParamsUtils"
#endif

#include "audio_params_utils.h"

#include <cerrno>
#include <climits>
#include <cstdlib>

#include "audio_common_log.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const char *PARAM_FAST_CONTROL = "persist.multimedia.audioflag.fastcontrolled";
const char *PARAM_FWK_EC_ENABLE = "const.multimedia.audio.fwk_ec.enable";
#ifdef SUPPORT_OLD_ENGINE
const char *PARAM_ENGINE_FLAG = "const.multimedia.audio.proaudioEnable";
#endif
const char *PARAM_HAS_EARPIECE = "const.multimedia.audio.has_earpiece";

bool GetInt32ParameterValue(const char *key, int32_t &value)
{
    char paraValue[30] = {0};
    auto res = GetParameter(key, "-1", paraValue, sizeof(paraValue));
    CHECK_AND_RETURN_RET_LOG(res > 0, false, "GetParameter fail, key:%{public}s res:%{public}d", key, res);
    char *end = nullptr;
    errno = 0;
    long parsedValue = std::strtol(paraValue, &end, 10);
    CHECK_AND_RETURN_RET_LOG(end != paraValue && end[0] == '\0' && errno != ERANGE &&
        parsedValue >= INT32_MIN && parsedValue <= INT32_MAX,
        false, "invalid value for key:%{public}s value:%{public}s", key, paraValue);
    value = static_cast<int32_t>(parsedValue);
    return true;
}
}

bool GetFastControlParam()
{
    int32_t fastControlFlag = 1;
    GetInt32ParameterValue(PARAM_FAST_CONTROL, fastControlFlag);
    AUDIO_INFO_LOG("get params[%{public}s]: %{public}s", PARAM_FAST_CONTROL, fastControlFlag ? "true" : "false");
    return fastControlFlag != 0;
}

bool GetEcEnableParam()
{
    bool ret = system::GetBoolParameter(PARAM_FWK_EC_ENABLE, false);
    AUDIO_INFO_LOG("get params[%{public}s]: %{public}s", PARAM_FWK_EC_ENABLE, ret ? "true" : "false");
    return ret;
}

int32_t GetEngineFlag()
{
#ifdef SUPPORT_OLD_ENGINE
    static int32_t engineFlag = -1;
    if (engineFlag == -1) {
        bool res = GetInt32ParameterValue(PARAM_ENGINE_FLAG, engineFlag);
        AUDIO_INFO_LOG("get params[%{public}s]: %{public}d", PARAM_ENGINE_FLAG, engineFlag);
        CHECK_AND_RETURN_RET_LOG(res, engineFlag, "get %{public}s fail", PARAM_ENGINE_FLAG);
    }
    return engineFlag;
#else
    return 1;
#endif
}

bool GetHasEarpieceParam()
{
    bool ret = system::GetBoolParameter(PARAM_HAS_EARPIECE, false);
    AUDIO_INFO_LOG("get params[%{public}s]: %{public}s", PARAM_HAS_EARPIECE, ret ? "true" : "false");
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
