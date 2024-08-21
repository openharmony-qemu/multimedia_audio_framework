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
#ifndef NAPI_AUDIO_ENUM_H
#define NAPI_AUDIO_ENUM_H

#include <map>
#include <string>
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "audio_info.h"
#include "tone_player.h"

namespace OHOS {
namespace AudioStandard {
const int32_t REFERENCE_CREATION_COUNT = 1;

class NapiAudioEnum {
public:
    NapiAudioEnum();
    ~NapiAudioEnum();
    enum AudioSampleFormatNapi {
        SAMPLE_FORMAT_INVALID = -1,
        SAMPLE_FORMAT_U8 = 0,
        SAMPLE_FORMAT_S16LE = 1,
        SAMPLE_FORMAT_S24LE = 2,
        SAMPLE_FORMAT_S32LE = 3,
        SAMPLE_FORMAT_F32LE = 4
    };

    enum AudioJsVolumeType {
        VOLUMETYPE_DEFAULT = -1,
        VOICE_CALL = 0,
        RINGTONE = 2,
        MEDIA = 3,
        ALARM = 4,
        ACCESSIBILITY = 5,
        VOICE_ASSISTANT = 9,
        ULTRASONIC = 10,
        VOLUMETYPE_MAX,
        ALL = 100
    };

    enum AudioRingMode {
        RINGER_MODE_SILENT = 0,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL
    };

    enum InterruptMode {
        SHARE_MODE = 0,
        INDEPENDENT_MODE = 1
    };

    enum FocusType {
        FOCUS_TYPE_RECORDING
    };

    enum CapturerType {
        TYPE_INVALID = -1,
        TYPE_MIC = 0,
        TYPE_VOICE_RECOGNITION = 1,
        TYPE_WAKEUP = 3,
        TYPE_VOICE_CALL = 4,
        TYPE_PLAYBACK_CAPTURE = 2,
        TYPE_COMMUNICATION = 7,
        TYPE_MESSAGE = 10,
        TYPE_REMOTE_CAST = 11,
        TYPE_VOICE_TRANSCRIPTION = 12
    };

    enum AudioDataCallbackResult {
        CALLBACK_RESULT_INVALID = -1,
        CALLBACK_RESULT_VALID = 0,
    };

    static napi_value Init(napi_env env, napi_value exports);
    static bool IsLegalInputArgumentInterruptMode(int32_t interruptMode);
    static bool IsLegalInputArgumentAudioEffectMode(int32_t audioEffectMode);
    static bool IsLegalInputArgumentChannelBlendMode(int32_t blendMode);
    static bool IsLegalCapturerType(int32_t type);
    static bool IsLegalInputArgumentVolType(int32_t inputType);
    static bool IsLegalInputArgumentRingMode(int32_t ringMode);
    static bool IsLegalInputArgumentVolumeAdjustType(int32_t adjustType);
    static bool IsLegalInputArgumentDeviceType(int32_t deviceType);
    static bool IsLegalInputArgumentDeviceFlag(int32_t deviceFlag);
    static bool IsLegalInputArgumentActiveDeviceType(int32_t activeDeviceFlag);
    static int32_t GetJsAudioVolumeType(AudioStreamType volumeType);
    static bool IsLegalInputArgumentCommunicationDeviceType(int32_t communicationDeviceType);
    static bool IsValidSourceType(int32_t intValue);
    static bool IsLegalDeviceUsage(int32_t usage);
    static bool IsLegalInputArgumentStreamUsage(int32_t streamUsage);
    static bool IsLegalOutputDeviceType(int32_t deviceType);
    static AudioVolumeType GetNativeAudioVolumeType(int32_t volumeType);
    static AudioRingerMode GetNativeAudioRingerMode(int32_t ringMode);
    static AudioRingMode GetJsAudioRingMode(int32_t ringerMode);
    static AudioStandard::FocusType GetNativeFocusType(int32_t focusType);
    static AudioStandard::InterruptMode GetNativeInterruptMode(int32_t interruptMode);
    static bool IsLegalInputArgumentSpatializationSceneType(int32_t spatializationSceneType);

private:
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_status InitAudioEnum(napi_env env, napi_value exports);
    static napi_status InitAudioExternEnum(napi_env env, napi_value exports);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static NapiAudioEnum* SetValue(napi_env env, napi_callback_info info, napi_value *args, napi_value &result);
    static NapiAudioEnum* GetValue(napi_env env, napi_callback_info info);
    static napi_value GetAudioSampleFormat(napi_env env, napi_callback_info info);
    static napi_value SetAudioSampleFormat(napi_env env, napi_callback_info info);
    static napi_value GetAudioChannel(napi_env env, napi_callback_info info);
    static napi_value SetAudioChannel(napi_env env, napi_callback_info info);
    static napi_value GetAudioSamplingRate(napi_env env, napi_callback_info info);
    static napi_value SetAudioSamplingRate(napi_env env, napi_callback_info info);
    static napi_value GetAudioEncodingType(napi_env env, napi_callback_info info);
    static napi_value SetAudioEncodingType(napi_env env, napi_callback_info info);
    static napi_value GetContentType(napi_env env, napi_callback_info info);
    static napi_value SetContentType(napi_env env, napi_callback_info info);
    static napi_value GetStreamUsage(napi_env env, napi_callback_info info);
    static napi_value SetStreamUsage(napi_env env, napi_callback_info info);
    static napi_value GetDeviceRole(napi_env env, napi_callback_info info);
    static napi_value SetDeviceRole(napi_env env, napi_callback_info info);
    static napi_value GetDeviceType(napi_env env, napi_callback_info info);
    static napi_value SetDeviceType(napi_env env, napi_callback_info info);

    static napi_value CreateEnumObject(const napi_env &env, const std::map<std::string, int32_t> &map,
        napi_ref &ref);
    static napi_value CreateEnumInt64Object(const napi_env &env, const std::map<std::string, uint64_t> &map,
        napi_ref &ref);
    static napi_value CreateLocalNetworkIdObject(napi_env env);
    static napi_value CreateDefaultVolumeGroupIdObject(napi_env env);
    static napi_value CreateDefaultInterruptIdObject(napi_env env);

    static napi_ref sConstructor_;
    static napi_ref audioChannel_;
    static napi_ref samplingRate_;
    static napi_ref encodingType_;
    static napi_ref contentType_;
    static napi_ref streamUsage_;
    static napi_ref deviceRole_;
    static napi_ref deviceType_;
    static napi_ref sourceType_;
    static napi_ref volumeAdjustType_;
    static napi_ref channelBlendMode_;
    static napi_ref audioRendererRate_;
    static napi_ref interruptEventType_;
    static napi_ref interruptForceType_;
    static napi_ref interruptHintType_;
    static napi_ref audioState_;
    static napi_ref sampleFormat_;
    static napi_ref audioEffectMode_;
    static napi_ref audioPrivacyType_;
    static napi_ref audioVolumeTypeRef_;
    static napi_ref deviceFlagRef_;
    static napi_ref activeDeviceTypeRef_;
    static napi_ref audioRingModeRef_;
    static napi_ref deviceChangeType_;
    static napi_ref interruptActionType_;
    static napi_ref audioScene_;
    static napi_ref interruptMode_;
    static napi_ref focusType_;
    static napi_ref connectTypeRef_;
    static napi_ref audioErrors_;
    static napi_ref communicationDeviceType_;
    static napi_ref interruptRequestType_;
    static napi_ref interruptRequestResultType_;
    static napi_ref toneType_;
    static napi_ref audioDviceUsage_;
    static napi_ref audioSpatialDeivceType_;
    static napi_ref audioChannelLayout_;
    static napi_ref audioStreamDeviceChangeReason_;
    static napi_ref spatializationSceneType_;
    static napi_ref asrNoiseSuppressionMode_;
    static napi_ref asrAecMode_;
    static napi_ref asrWhisperDetectionMode_;
    static napi_ref asrVoiceControlMode_;
    static napi_ref asrVoiceMuteMode_;
    static napi_ref policyType_;
    static napi_ref audioDataCallbackResult_;
    static napi_ref concurrencyMode_;
    static napi_ref reason_;

    static const std::map<std::string, int32_t> audioChannelMap;
    static const std::map<std::string, int32_t> samplingRateMap;
    static const std::map<std::string, int32_t> encodingTypeMap;
    static const std::map<std::string, int32_t> contentTypeMap;
    static const std::map<std::string, int32_t> streamUsageMap;
    static const std::map<std::string, int32_t> deviceRoleMap;
    static const std::map<std::string, int32_t> deviceTypeMap;
    static const std::map<std::string, int32_t> sourceTypeMap;
    static const std::map<std::string, int32_t> volumeAdjustTypeMap;
    static const std::map<std::string, int32_t> channelBlendModeMap;
    static const std::map<std::string, int32_t> rendererRateMap;
    static const std::map<std::string, int32_t> interruptEventTypeMap;
    static const std::map<std::string, int32_t> interruptForceTypeMap;
    static const std::map<std::string, int32_t> interruptHintTypeMap;
    static const std::map<std::string, int32_t> audioSampleFormatMap;
    static const std::map<std::string, int32_t> audioStateMap;
    static const std::map<std::string, int32_t> audioPrivacyTypeMap;
    static const std::map<std::string, int32_t> effectModeMap;
    static const std::map<std::string, int32_t> deviceChangeTypeMap;
    static const std::map<std::string, int32_t> audioSceneMap;
    static const std::map<std::string, int32_t> interruptActionTypeMap;
    static const std::map<std::string, int32_t> audioVolumeTypeMap;
    static const std::map<std::string, int32_t> activeDeviceTypeMap;
    static const std::map<std::string, int32_t> interruptModeMap;
    static const std::map<std::string, int32_t> focusTypeMap;
    static const std::map<std::string, int32_t> audioErrorsMap;
    static const std::map<std::string, int32_t> communicationDeviceTypeMap;
    static const std::map<std::string, int32_t> interruptRequestTypeMap;
    static const std::map<std::string, int32_t> interruptRequestResultTypeMap;
    static const std::map<std::string, int32_t> deviceFlagMap;
    static const std::map<std::string, int32_t> connectTypeMap;
    static const std::map<std::string, int32_t> audioRingModeMap;
    static const std::map<std::string, int32_t> toneTypeMap;
    static const std::map<std::string, int32_t> audioDeviceUsageMap;
    static const std::map<std::string, int32_t> audioSpatialDeivceTypeMap;
    static const std::map<std::string, uint64_t> audioChannelLayoutMap;
    static const std::map<std::string, int32_t> audioDeviceChangeReasonMap;
    static const std::map<std::string, int32_t> spatializationSceneTypeMap;
    static const std::map<std::string, int32_t> asrNoiseSuppressionModeMap;
    static const std::map<std::string, int32_t> asrAecModeMap;
    static const std::map<std::string, int32_t> asrWhisperDetectionModeMap;
    static const std::map<std::string, int32_t> asrVoiceControlModeMap;
    static const std::map<std::string, int32_t> asrVoiceMuteModeMap;
    static const std::map<std::string, int32_t> policyTypeMap;
    static const std::map<std::string, int32_t> audioDataCallbackResultMap;
    static const std::map<std::string, int32_t> concurrencyModeMap;
    static const std::map<std::string, int32_t> reasonMap;
    static std::unique_ptr<AudioParameters> sAudioParameters_;

    std::unique_ptr<AudioParameters> audioParameters_;
    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // OHOS_NAPI_AUDIO_ENUM_H
