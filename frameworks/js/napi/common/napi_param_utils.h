/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef NAPI_PARAM_UTILS_H
#define NAPI_PARAM_UTILS_H

#include <cstdint>
#include <map>
#include <list>
#if defined(ANDROID_PLATFORM) || defined(IOS_PLATFORM)
#include <securec.h>
#else
#include "ability.h"
#endif
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_base_context.h"
#include "audio_common_log.h"
#include "audio_capturer.h"
#include "audio_system_manager.h"
#include "audio_stream_manager.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
const int ARGS_ZERO = 0;
const int ARGS_ONE = 1;
const int ARGS_TWO = 2;
const int ARGS_THREE = 3;

const int PARAM0 = 0;
const int PARAM1 = 1;
const int PARAM2 = 2;

/* check condition related to argc/argv, return and logging. */
#define NAPI_CHECK_ARGS_RETURN_VOID(context, condition, message, code)               \
    do {                                                               \
        if (!(condition)) {                                            \
            (context)->status = napi_invalid_arg;                         \
            (context)->errMessage = std::string(message);                      \
            (context)->errCode = code;                      \
            AUDIO_ERR_LOG("test (" #condition ") failed: " message);           \
            return;                                                    \
        }                                                              \
    } while (0)

#define NAPI_CHECK_STATUS_RETURN_VOID(context, message, code)                        \
    do {                                                               \
        if ((context)->status != napi_ok) {                               \
            (context)->errMessage = std::string(message);                      \
            (context)->errCode = code;                      \
            AUDIO_ERR_LOG("test (context->status == napi_ok) failed: " message);  \
            return;                                                    \
        }                                                              \
    } while (0)

class NapiParamUtils {
public:
    static napi_status GetParam(const napi_env &env, napi_callback_info info, size_t &argc, napi_value *args);
    static napi_status GetValueInt32(const napi_env &env, int32_t &value, napi_value in);
    static napi_status SetValueInt32(const napi_env &env, const int32_t &value, napi_value &result);
    static napi_status GetValueInt32(const napi_env &env, const std::string &fieldStr, int32_t &value, napi_value in);
    static napi_status SetValueInt32(const napi_env &env, const std::string &fieldStr,
        const int32_t value, napi_value &result);

    static napi_status GetValueUInt32(const napi_env &env, uint32_t &value, napi_value in);
    static napi_status SetValueUInt32(const napi_env &env, const uint32_t &value, napi_value &result);

    static napi_status GetValueDouble(const napi_env &env, double &value, napi_value in);
    static napi_status SetValueDouble(const napi_env &env, const double &value, napi_value &result);
    static napi_status GetValueDouble(const napi_env &env, const std::string &fieldStr, double &value, napi_value in);
    static napi_status SetValueDouble(const napi_env &env, const std::string &fieldStr,
        const double value, napi_value &result);

    static std::string GetStringArgument(napi_env env, napi_value value);
    static std::string GetPropertyString(napi_env env, napi_value value, const std::string &fieldStr);
    static napi_status SetValueString(const napi_env &env, const std::string &stringValue, napi_value &result);
    static napi_status SetValueString(const napi_env &env, const std::string &fieldStr, const std::string &stringValue,
        napi_value &result);

    static napi_status GetValueBoolean(const napi_env &env, bool &boolValue, napi_value in);
    static napi_status SetValueBoolean(const napi_env &env, const bool boolValue, napi_value &result);
    static napi_status GetValueBoolean(const napi_env &env, const std::string &fieldStr,
        bool &boolValue, napi_value in);
    static napi_status SetValueBoolean(const napi_env &env, const std::string &fieldStr,
        const bool boolValue, napi_value &result);

    static napi_status GetValueInt64(const napi_env &env, int64_t &value, napi_value in);
    static napi_status SetValueInt64(const napi_env &env, const int64_t &value, napi_value &result);
    static napi_status GetValueInt64(const napi_env &env, const std::string &fieldStr, int64_t &value, napi_value in);
    static napi_status SetValueInt64(const napi_env &env, const std::string &fieldStr,
        const int64_t value, napi_value &result);
    static napi_status GetArrayBuffer(const napi_env &env, void* &data, size_t &length, napi_value in);
    static napi_status CreateArrayBuffer(const napi_env &env, const std::string &fieldStr, size_t bufferLen,
        uint8_t *bufferData, napi_value &result);
    static napi_status CreateArrayBuffer(const napi_env &env, const size_t bufferLen,
        const uint8_t *bufferData, napi_value &result);

    static napi_value GetUndefinedValue(napi_env env);

    /* NapiAudioRenderer Get&&Set object */
    static void ConvertDeviceInfoToAudioDeviceDescriptor(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
        const AudioDeviceDescriptor &deviceInfo);
    static napi_status GetRendererOptions(const napi_env &env, AudioRendererOptions *opts, napi_value in);
    static napi_status GetRendererInfo(const napi_env &env, AudioRendererInfo *rendererInfo, napi_value in);
    static napi_status SetRendererInfo(const napi_env &env, const AudioRendererInfo &rendererInfo, napi_value &result);
    static napi_status GetStreamInfo(const napi_env &env, AudioStreamInfo *streamInfo, napi_value in);
    static napi_status SetStreamInfo(const napi_env &env, const AudioStreamInfo &streamInfo, napi_value &result);
    static napi_status SetTimeStampInfo(const napi_env &env, const Timestamp &timestamp, napi_value &result);
    static napi_status SetValueInt32Element(const napi_env &env, const std::string &fieldStr,
        const std::vector<int32_t> &values, napi_value &result);
    static napi_status SetDeviceDescriptor(const napi_env &env, const AudioDeviceDescriptor &deviceInfo,
        napi_value &result);
    static napi_status SetDeviceDescriptors(const napi_env &env,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescriptors, napi_value &result);
    static napi_status SetAudioSpatialEnabledStateForDevice(const napi_env &env,
    const AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice, napi_value &result);
    static napi_status SetValueDeviceInfo(const napi_env &env, const AudioDeviceDescriptor &deviceInfo,
        napi_value &result);
    static napi_status SetInterruptEvent(const napi_env &env, const InterruptEvent &interruptEvent,
        napi_value &result);
    static napi_status SetNativeAudioRendererDataInfo(const napi_env &env,
        const AudioRendererDataInfo &audioRendererDataInfo, napi_value &result);

    /* NapiAudioCapturer Get&&Set object */
    static napi_status GetCapturerInfo(const napi_env &env, AudioCapturerInfo *capturerInfo, napi_value in);
    static napi_status SetCapturerInfo(const napi_env &env, const AudioCapturerInfo &capturerInfo, napi_value &result);
    static napi_status GetCaptureFilterOptionsVector(const napi_env &env,
        CaptureFilterOptions *filterOptions, napi_value in);
    static napi_status GetPlaybackCaptureConfig(const napi_env &env,
        AudioPlaybackCaptureConfig* captureConfig, napi_value in);
    static napi_status GetCapturerOptions(const napi_env &env, AudioCapturerOptions *opts, napi_value in);
    static napi_status SetAudioCapturerChangeInfoDescriptors(const napi_env &env,
        const AudioCapturerChangeInfo &changeInfo, napi_value &result);
    static napi_status SetMicrophoneDescriptor(const napi_env &env, const sptr<MicrophoneDescriptor> &micDesc,
        napi_value &result);
    static napi_status SetMicrophoneDescriptors(const napi_env &env,
        const std::vector<sptr<MicrophoneDescriptor>> &micDescs, napi_value &result);

    /* NapiAudioManager Get&&Set object */
    static napi_status SetValueMicStateChange(const napi_env &env, const MicStateChangeEvent &micStateChangeEvent,
        napi_value &result);
    static napi_status SetVolumeGroupInfos(const napi_env &env,
        const std::vector<sptr<VolumeGroupInfo>> &volumeGroupInfos, napi_value &result);
    static napi_status SetValueVolumeEvent(const napi_env& env, const VolumeEvent &volumeEvent,
        napi_value &result);
    static napi_status SetValueStreamVolumeEvent(const napi_env& env,
        const StreamVolumeEvent &volumeEvent, napi_value &result);
    static napi_status SetValueStreamUsageArray(const napi_env& env,
        const std::vector<StreamUsage> streamUsageArray, napi_value &result);
    static napi_status SetValueAudioVolumeTypeArray(const napi_env& env,
        const std::vector<AudioVolumeType> volumeTypeArray, napi_value &result);
    static napi_status GetAudioDeviceDescriptor(const napi_env &env,
        std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool &argTransFlag, napi_value in);
    static napi_status GetAudioDeviceDescriptorVector(const napi_env &env,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescriptorsVector,
        bool &argTransFlag, napi_value in);
    static napi_status GetAudioCapturerFilter(const napi_env &env, sptr<AudioCapturerFilter> &audioCapturerFilter,
        napi_value in);
    static napi_status GetAudioCapturerInfo(const napi_env &env, AudioCapturerInfo *capturerInfo, napi_value in);
    static napi_status GetAudioRendererFilter(const napi_env &env, sptr<AudioRendererFilter> &audioRendererFilter,
        bool &argTransFlag, napi_value in);
    static napi_status GetAudioDeviceUsage(const napi_env &env, AudioDeviceUsage &audioDevUsage, napi_value in);
    static napi_status SetValueDeviceChangeAction(const napi_env& env, const DeviceChangeAction &action,
        napi_value &result);
    static napi_status SetValueBlockedDeviceAction(const napi_env& env, const MicrophoneBlockedInfo &action,
        napi_value &result);
    static napi_status SetRendererChangeInfos(const napi_env &env,
        const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &changeInfos, napi_value &result);
    static napi_status SetCapturerChangeInfos(const napi_env &env,
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &changeInfos, napi_value &result);
    static napi_status SetEffectInfo(const napi_env &env,
        const AudioSceneEffectInfo &audioSceneEffectInfo, napi_value &result);
    static napi_status GetAudioInterrupt(const napi_env &env, AudioInterrupt &audioInterrupt, napi_value in);
    static napi_status SetValueInterruptAction(const napi_env &env, const InterruptAction &interruptAction,
        napi_value &result);
    static napi_status GetSpatialDeviceState(napi_env env, AudioSpatialDeviceState *spatialDeviceState,
        napi_value in);
    static napi_status GetExtraParametersSubKV(napi_env env, std::vector<std::pair<std::string, std::string>> &subKV,
        napi_value in);
    static int32_t UniqueEffectPropertyData(AudioEffectPropertyArrayV3 &propertyArray);
    static napi_status SetEffectProperty(const napi_env &env,
        const AudioEffectPropertyArrayV3 &propertyArray, napi_value &result);
    static napi_status GetEffectPropertyArray(napi_env env,
        AudioEffectPropertyArrayV3 &propertyArray, napi_value in);
    static napi_status SetEffectProperty(const napi_env &env,
        const AudioEffectPropertyArray &effectArray, napi_value &result);
    static napi_status SetEnhanceProperty(const napi_env &env,
        const AudioEnhancePropertyArray &enhanceArray, napi_value &result);
    static napi_status GetEffectPropertyArray(napi_env env, AudioEffectPropertyArray &effectArray, napi_value in);
    static napi_status GetEnhancePropertyArray(napi_env env, AudioEnhancePropertyArray &enhanceArray, napi_value in);
    static napi_status GetExtraParametersVector(const napi_env &env, std::vector<std::string> &subKeys, napi_value in);
    static napi_status SetExtraAudioParametersInfo(const napi_env &env,
        const std::vector<std::pair<std::string, std::string>> &extraParameters, napi_value &result);
    static napi_status GetAudioSessionStrategy(const napi_env &env,
        AudioSessionStrategy &audioSessionStrategy, napi_value in);
    static napi_status SetAudioSessionDeactiveEvent(const napi_env &env,
        const AudioSessionDeactiveEvent &deactiveEvent, napi_value &result);
    static napi_status GetAudioCapturerChangeInfo(const napi_env &env, AudioCapturerChangeInfo &capturerInfo,
        napi_value in);
    static bool CheckArgType(napi_env env, napi_value arg, napi_valuetype expectedType);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_PARAM_UTILS_H
