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
#ifndef ANI_PARAM_UTILS_H
#define ANI_PARAM_UTILS_H

#include "ani.h"
#include "audio_common_log.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

class AniParamUtils {
public:
    static ani_boolean isUndefined(ani_env *env, ani_object object);
    static ani_status GetInt32(ani_env *env, ani_int arg, int32_t &value);
    static ani_status GetInt32(ani_env *env, ani_object arg, int32_t &value);
    static ani_status SetValueEnum(ani_env *env, ani_class cls, ani_object handle,
        const std::string &key, ani_enum_item value);
    static ani_status SetValueInt(ani_env *env, ani_class cls, ani_object handle,
        const std::string &key, int32_t value);
    static ani_status SetValueNumber(ani_env *env, ani_class cls, ani_object handle,
        const std::string &key, int32_t value);
    static ani_status SetValueBool(ani_env *env, ani_class cls, ani_object handle,
    const std::string &key, bool value);
    static ani_status SetValueString(ani_env *env, ani_class cls, ani_object handle,
        const std::string &key, const std::string &value);
    static ani_status SetValueObject(ani_env *env, ani_class cls, ani_object handle,
        const std::string &key, ani_object value);
    static ani_status CreateVolumeEventHandle(ani_env *env, ani_class &cls, ani_object &result);
    static ani_status SetValueVolumeEvent(ani_env *env, const VolumeEvent &volumeEvent, ani_object &handle);
    static ani_status CreateAudioRendererInfoHandle(ani_env *env, ani_class &cls, ani_object &result);
    static ani_status SetRendererInfo(ani_env *env,
        const AudioRendererInfo &audioRendererInfo, ani_object &handle);
    static ani_status CreateInterruptEventHandle(ani_env *env, ani_class &cls, ani_object &result);
    static ani_status SetInterruptEvent(ani_env *env,
        const InterruptEvent &interruptEvent, ani_object &handle);
    static ani_status CreateAudioDeviceDescriptorHandle(ani_env *env, ani_class &cls, ani_object &result);
    static ani_status MakeAniArrayRetSetMethod(ani_env *env, size_t size, ani_object &aniArray, ani_method &setMethod);
    template<class Container>
    static ani_status SetValueElement(ani_env *env, ani_class cls, ani_object &result,
        const std::string &key, const Container &values);
    static ani_status SetSetDeviceDescriptorMore(ani_env *env, ani_class cls,
        const AudioDeviceDescriptor &deviceInfo, ani_object &result);
    static ani_status SetDeviceDescriptor(ani_env *env, const AudioDeviceDescriptor &deviceInfo, ani_object &result);
    static ani_status SetDeviceDescriptors(ani_env *env, ani_object &aniArray,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescriptors);
    static ani_status SetValueDeviceInfo(ani_env *env, const AudioDeviceDescriptor &deviceInfo, ani_object &handle);
    static ani_status CreateAudioStreamDeviceChangeHandle(ani_env *env, ani_class &cls, ani_object &result);
    static ani_status CreateDeviceBlockStatusInfoHandle(ani_env *env, ani_class &cls, ani_object &result);
    static ani_status SetValueBlockedDeviceAction(ani_env *env, const MicrophoneBlockedInfo &action,
        ani_object &handle);
    static std::string GetStringArgument(ani_env *env, ani_string aniStr);
    static ani_status GetString(ani_env *env, ani_string arg, std::string &str);
    static ani_status GetString(ani_env *env, ani_object arg, std::string &str);
    static ani_status ToAniString(ani_env *env, const std::string &str, ani_string &aniStr);
    static ani_status ToAniInt(ani_env *env, const std::int32_t &int32, ani_int &aniInt);
    static ani_status ToAniLong(ani_env *env, const std::int64_t &int64, ani_long &aniLong);
    static ani_status ToAniDouble(ani_env *env, const double &arg, ani_double &aniDouble);
    static ani_status ToAniDoubleObject(ani_env *env, double src, ani_object &aniObj);
    static ani_status ToAniBooleanObject(ani_env *env, bool src, ani_object &aniObj);
    static ani_status ToAniIntObject(ani_env *env, int32_t src, ani_object &aniObj);
    static ani_status ToAniLongObject(ani_env *env, int64_t src, ani_object &aniObj);
    static ani_status GetOptionalEnumInt32Field(ani_env *env, ani_object src,
        const std::string &fieldName, int32_t &value);
    static ani_status GetOptionalInt32Field(ani_env *env, ani_object src,
        const std::string &fieldName, int32_t &value);
    static ani_status GetRendererInfo(ani_env *env, ani_object arg, AudioRendererInfo *rendererInfo);
    static void CreateAniErrorObject(ani_env *env, ani_object &errorObj, const int32_t errCode,
        const std::string &errMsg);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ANI_PARAM_UTILS_H
