From e0d146599eaa0640e740b1033d82a6622ce3d608 Mon Sep 17 00:00:00 2001
From: d00845858 <dushuaifei@huawei.com>
Date: Sat, 27 Jun 2026 17:46:14 +0800
Subject: [PATCH] capture noise reduction
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit

TicketNo: DTS0000000000000
Description: capture noise reduction
Team:gitcode
Feature or Bugfix:
Binary Source: sync from gitcode
PrivateCode(Yes/No):No
------ DO NOT MODIFY, AUTO-GENERATED! ------
Gitcode-Issue: 13000
Time: "2026-06-20T11:47:08.729477351Z"
PR-Num: 15703
Gitcode-PR: https://gitcode.com/openharmony/multimedia_audio_framework/merge_requests/15703
UserId:
☹️❌⛔code conflicts, please fix it and re-publish.
conflict files lists:
interfaces/inner_api/native/audiocommon/include/audio_noise_reduction_types.h
frameworks/native/audiocapturer/include/audio_capturer_private.h
frameworks/native/audiocapturer/src/audio_capturer.cpp
frameworks/native/ohaudio/OHAudioCapturer.cpp
frameworks/native/ohaudio/OHAudioCapturer.h
interfaces/inner_api/native/audiocommon/include/audio_noise_reduction_types.h
frameworks/js/napi/common/napi_audio_enum.cpp
interfaces/kits/c/audio_capturer/native_audiocapturer.h
cherry picked from commit a573a0512369225ceedd079c37de7a0fe060392e

Change-Id: I17819560071313bc29730c5366a977d6ff6c1234
---
 .../audiocapturer/napi_audio_capturer.cpp     |  91 +++++++++++++++
 .../napi/audiocapturer/napi_audio_capturer.h  |   3 +
 frameworks/js/napi/common/napi_audio_enum.cpp |  16 +++
 frameworks/js/napi/common/napi_audio_enum.h   |   7 ++
 .../src/audio_debug_manager.cpp               |   1 +
 .../include/audio_capturer_private.h          |   9 +-
 .../include/shared_audio_capturer_wrapper.h   |  16 +++
 .../audiocapturer/src/audio_capturer.cpp      |  86 ++++++++++++++
 .../unittest/include/audio_capturer_mock.h    |   3 +
 .../native/audioutils/include/audio_utils.h   |   1 +
 .../include/common/hdi_adapter_info.h         |   1 +
 .../manager/hdi_adapter_manager.cpp           |   3 +
 .../source/audio_capture_source.cpp           |  18 ++-
 .../source/fast_audio_capture_source.cpp      |   1 +
 .../native/hdiadapter_new/util/id_handler.cpp |   2 +
 frameworks/native/ohaudio/OHAudioCapturer.cpp | 109 ++++++++++++++++++
 frameworks/native/ohaudio/OHAudioCapturer.h   |   8 ++
 .../audiocapturer/taihe_audio_capturer.cpp    |  61 ++++++++++
 .../audiocapturer/taihe_audio_capturer.h      |   3 +
 frameworks/taihe/common/taihe_audio_enum.cpp  |  38 ++++++
 frameworks/taihe/common/taihe_audio_enum.h    |   4 +
 .../taihe/idl/ohos.multimedia.audio.taihe     |  10 ++
 .../audiocapturer/include/audio_capturer.h    |  19 +++
 .../audiocommon/include/audio_pipe_types.h    |   1 +
 .../audiocommon/include/audio_stream_info.h   |   1 +
 .../c/audio_capturer/native_audiocapturer.h   |  46 ++++++++
 .../kits/c/common/native_audio_common.h       |  28 +++++
 .../common/include/audio_module_info.h        |   1 +
 .../domain/device/src/audio_device_common.cpp |   3 +-
 .../pipe/src/audio_concurrency_manager.cpp    |   2 +
 .../src/audio_definition_policy_utils.cpp     |   1 +
 .../domain/pipe/src/audio_pipe_manager.cpp    |   3 +-
 .../domain/pipe/src/audio_pipe_selector.cpp   |   2 +
 .../volume/src/audio_adapter_manager.cpp      |   4 +
 .../parser/include/audio_concurrency_parser.h |   1 +
 .../datashare/src/audio_policy_utils.cpp      |   5 +-
 .../service_main/src/audio_core_service.cpp   |   6 +
 .../src/audio_core_service_private.cpp        |   6 +-
 .../audio_service/server/src/audio_server.cpp |  17 ++-
 39 files changed, 628 insertions(+), 9 deletions(-)

diff --git a/frameworks/js/napi/audiocapturer/napi_audio_capturer.cpp b/frameworks/js/napi/audiocapturer/napi_audio_capturer.cpp
index 22ffd81281b..0c0ecdfab56 100644
--- a/frameworks/js/napi/audiocapturer/napi_audio_capturer.cpp
+++ b/frameworks/js/napi/audiocapturer/napi_audio_capturer.cpp
@@ -273,6 +273,9 @@ napi_status NapiAudioCapturer::InitAudioCapturer(napi_env env, napi_value &const
         DECLARE_NAPI_FUNCTION("getAudioTimestampInfoSync", GetAudioTimestampInfoSync),
         DECLARE_NAPI_FUNCTION("setWillMuteWhenInterrupted", SetWillMuteWhenInterrupted),
         DECLARE_NAPI_FUNCTION("setMuteHint", SetMuteHint),
+        DECLARE_NAPI_FUNCTION("setNoiseReductionMode", SetNoiseReductionMode),
+        DECLARE_NAPI_FUNCTION("getNoiseReductionMode", GetNoiseReductionMode),
+        DECLARE_NAPI_FUNCTION("getSupportedNoiseReductionModes", GetSupportedNoiseReductionModes),
         DECLARE_NAPI_FUNCTION("requestPlaybackCaptureStart", RequestPlaybackCaptureStart),
         DECLARE_NAPI_FUNCTION("on", On),
         DECLARE_NAPI_FUNCTION("off", Off),
@@ -543,6 +546,7 @@ napi_value NapiAudioCapturer::CreateAudioCapturer(napi_env env, napi_callback_in
     return NapiAsyncWork::Enqueue(env, context, "CreateAudioCapturer", nullptr, complete);
 }
 
+
 napi_value NapiAudioCapturer::CreateMicInAudioCapturer(napi_env env, napi_callback_info info)
 {
     auto context = std::make_shared<AudioCapturerAsyncContext>();
@@ -1247,6 +1251,93 @@ napi_value NapiAudioCapturer::SetMuteHint(napi_env env, napi_callback_info info)
     return NapiAsyncWork::Enqueue(env, context, "SetMuteHint", executor, complete);
 }
 
+napi_value NapiAudioCapturer::SetNoiseReductionMode(napi_env env, napi_callback_info info)
+{
+    napi_value result = nullptr;
+    size_t argc = ARGS_ONE;
+    napi_value args[ARGS_ONE] = {};
+
+    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, args);
+    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
+        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");
+
+    napi_valuetype valueType = napi_undefined;
+    napi_typeof(env, args[PARAM0], &valueType);
+    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
+        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of noiseReductionMode must be number"),
+        "invalid valueType");
+
+    int32_t noiseReductionMode = 0;
+    NapiParamUtils::GetValueInt32(env, noiseReductionMode, args[PARAM0]);
+    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentNoiseReductionMode(noiseReductionMode),
+        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
+        "parameter verification failed: The param of noiseReductionMode must be enum NoiseReductionMode"),
+        "get noiseReductionMode failed");
+
+    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
+    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");
+
+    int32_t ret = napiAudioCapturer->audioCapturer_->SetNoiseReductionMode(
+        static_cast<NoiseReductionMode>(noiseReductionMode));
+    if (ret == ERR_INVALID_PARAM || ret == ERROR_INVALID_PARAM) {
+        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM);
+    }
+    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
+        "SetNoiseReductionMode failed");
+    return NapiParamUtils::GetUndefinedValue(env);
+}
+
+napi_value NapiAudioCapturer::GetNoiseReductionMode(napi_env env, napi_callback_info info)
+{
+    napi_value result = nullptr;
+    size_t argc = PARAM0;
+
+    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
+    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
+        NAPI_ERR_INPUT_INVALID), "invalid arguments");
+    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
+    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");
+
+    NoiseReductionMode noiseReductionMode = NOISE_REDUCTION_MODE_FIDELITY;
+    int32_t ret = napiAudioCapturer->audioCapturer_->GetNoiseReductionMode(noiseReductionMode);
+    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
+        "GetNoiseReductionMode failed");
+
+    NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(noiseReductionMode), result);
+    return result;
+}
+
+napi_value NapiAudioCapturer::GetSupportedNoiseReductionModes(napi_env env, napi_callback_info info)
+{
+    napi_value result = nullptr;
+    size_t argc = PARAM0;
+
+    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
+    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
+        NAPI_ERR_INPUT_INVALID), "invalid arguments");
+    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
+    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");
+
+    std::vector<NoiseReductionMode> noiseReductionModes;
+    int32_t ret = napiAudioCapturer->audioCapturer_->GetSupportedNoiseReductionModes(noiseReductionModes);
+    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
+        "GetSupportedNoiseReductionModes failed");
+
+    napi_status status = napi_create_array_with_length(env, noiseReductionModes.size(), &result);
+    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
+        "napi_create_array_with_length failed");
+    for (size_t i = 0; i < noiseReductionModes.size(); ++i) {
+        napi_value jsMode = nullptr;
+        status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(noiseReductionModes[i]), jsMode);
+        CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
+            "SetValueInt32 failed");
+        status = napi_set_element(env, result, i, jsMode);
+        CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
+            "napi_set_element failed");
+    }
+    return result;
+}
+
 napi_value NapiAudioCapturer::RequestPlaybackCaptureStart(napi_env env, napi_callback_info info)
 {
     size_t argc = ARGS_TWO;
diff --git a/frameworks/js/napi/audiocapturer/napi_audio_capturer.h b/frameworks/js/napi/audiocapturer/napi_audio_capturer.h
index 804d6059578..931d18fcf03 100644
--- a/frameworks/js/napi/audiocapturer/napi_audio_capturer.h
+++ b/frameworks/js/napi/audiocapturer/napi_audio_capturer.h
@@ -106,6 +106,9 @@ private:
     static napi_value GetOverflowCountSync(napi_env env, napi_callback_info info);
     static napi_value SetWillMuteWhenInterrupted(napi_env env, napi_callback_info info);
     static napi_value SetMuteHint(napi_env env, napi_callback_info info);
+    static napi_value SetNoiseReductionMode(napi_env env, napi_callback_info info);
+    static napi_value GetNoiseReductionMode(napi_env env, napi_callback_info info);
+    static napi_value GetSupportedNoiseReductionModes(napi_env env, napi_callback_info info);
     static napi_value RequestPlaybackCaptureStart(napi_env env, napi_callback_info info);
     static napi_value SetIndependentAudioSessionStrategy(napi_env env, napi_callback_info info);
     static napi_value RegisterCallback(napi_env env, napi_value jsThis,
diff --git a/frameworks/js/napi/common/napi_audio_enum.cpp b/frameworks/js/napi/common/napi_audio_enum.cpp
index 9ab33ab8f03..1991040882c 100755
--- a/frameworks/js/napi/common/napi_audio_enum.cpp
+++ b/frameworks/js/napi/common/napi_audio_enum.cpp
@@ -1231,6 +1231,22 @@ bool NapiAudioEnum::IsLegalInputArgumentAudioEffectMode(int32_t audioEffectMode)
     return result;
 }
 
+bool NapiAudioEnum::IsLegalInputArgumentNoiseReductionMode(int32_t noiseReductionMode)
+{
+    bool result = false;
+    switch (noiseReductionMode) {
+        case NOISE_REDUCTION_MODE_FIDELITY:
+        case NOISE_REDUCTION_MODE_PURE_VOCALS:
+        case NOISE_REDUCTION_MODE_STANDARD:
+            result = true;
+            break;
+        default:
+            result = false;
+            break;
+    }
+    return result;
+}
+
 bool NapiAudioEnum::IsLegalInputArgumentChannelBlendMode(int32_t blendMode)
 {
     bool result = false;
diff --git a/frameworks/js/napi/common/napi_audio_enum.h b/frameworks/js/napi/common/napi_audio_enum.h
index a0b23a282e5..a7a7392f064 100644
--- a/frameworks/js/napi/common/napi_audio_enum.h
+++ b/frameworks/js/napi/common/napi_audio_enum.h
@@ -139,6 +139,12 @@ public:
         LATENCY_TYPE_HARDWARE = 2
     };
 
+    enum AudioNoiseReductionMode {
+        NOISE_REDUCTION_MODE_FIDELITY = 0,
+        NOISE_REDUCTION_MODE_PURE_VOCALS = 1,
+        NOISE_REDUCTION_MODE_STANDARD = 2
+    };
+
     static napi_value Init(napi_env env, napi_value exports);
     static bool IsLegalInputArgumentInterruptMode(int32_t interruptMode);
     static bool IsLegalInputArgumentAudioEffectMode(int32_t audioEffectMode);
@@ -179,6 +185,7 @@ public:
     static bool IsLegalInputArgumentSessionScene(int32_t scene);
     static bool IsLegalRenderTarget(int32_t target);
     static bool IsLegalAudioLatencyType(int32_t latencyType);
+    static bool IsLegalInputArgumentNoiseReductionMode(int32_t noiseReductionMode);
     static LatencyFlag ConvertLatencyTypeToFlag(int32_t latencyType);
     static bool IsLegalBluetoothAndNearlinkPreferredRecordCategory(uint32_t category);
 
diff --git a/frameworks/native/audio_debug_manager/src/audio_debug_manager.cpp b/frameworks/native/audio_debug_manager/src/audio_debug_manager.cpp
index 497ad058162..78f95787e13 100644
--- a/frameworks/native/audio_debug_manager/src/audio_debug_manager.cpp
+++ b/frameworks/native/audio_debug_manager/src/audio_debug_manager.cpp
@@ -449,6 +449,7 @@ const RouteFlagEntry ROUTE_FLAG_ENTRIES[] = {
     {AUDIO_INPUT_FLAG_INTERPHONE, "INPUT_FLAG_INTERPHONE"},
     {AUDIO_INPUT_FLAG_LIVE, "INPUT_FLAG_LIVE"},
     {AUDIO_INPUT_FLAG_CAMCORDER, "INPUT_FLAG_CAMCORDER"},
+    {AUDIO_INPUT_FLAG_VOICE_MESSAGE, "INPUT_FLAG_VOICE_MESSAGE"},
 };
 
 std::string AudioRouteFlagToString(uint32_t routeFlag)
diff --git a/frameworks/native/audiocapturer/include/audio_capturer_private.h b/frameworks/native/audiocapturer/include/audio_capturer_private.h
index fe3992c1432..0e0666e3f31 100644
--- a/frameworks/native/audiocapturer/include/audio_capturer_private.h
+++ b/frameworks/native/audiocapturer/include/audio_capturer_private.h
@@ -25,6 +25,7 @@
 #include "audio_capturer_proxy_obj.h"
 #include "audio_task_loop.h"
 #include "audio_errors.h"
+#include "audio_noise_reduction_types.h"
 
 namespace OHOS {
 namespace AudioStandard {
@@ -131,6 +132,9 @@ public:
     int32_t StartPlaybackCapture() override;
     int32_t SetIndependentAudioSessionStrategy(const AudioSessionStrategy &strategy, const uint32_t behavior) override;
     int32_t SetInMainThreadState(bool isInMainThread) override;
+    int32_t SetNoiseReductionMode(NoiseReductionMode mode) override;
+    int32_t GetNoiseReductionMode(NoiseReductionMode &mode) override;
+    int32_t GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode>& modes) override;
 
     std::shared_ptr<IAudioStream> audioStream_;
     AudioCapturerInfo capturerInfo_ = {};
@@ -276,7 +280,10 @@ private:
     std::mutex inSwitchingMtx_;
     bool inSwitchingFlag_ = false;
     bool isFirstTimeSetMainThreadState_ = true;
-
+    NoiseReductionMode noiseReductionMode_ = NOISE_REDUCTION_MODE_FIDELITY;
+    bool hasFetchedSupportedModes_ = false;
+    std::vector<NoiseReductionMode> supportedNoiseReductionModes_ = {NOISE_REDUCTION_MODE_FIDELITY};
+    std::recursive_mutex noiseReductionModesMtx_;
 };
 
 class AudioCapturerInterruptCallbackImpl : public AudioInterruptCallback {
diff --git a/frameworks/native/audiocapturer/include/shared_audio_capturer_wrapper.h b/frameworks/native/audiocapturer/include/shared_audio_capturer_wrapper.h
index 5d3e7261d81..b2a3ec79964 100644
--- a/frameworks/native/audiocapturer/include/shared_audio_capturer_wrapper.h
+++ b/frameworks/native/audiocapturer/include/shared_audio_capturer_wrapper.h
@@ -19,6 +19,7 @@
 #include "audio_capturer.h"
 #include "audio_errors.h"
 #include "audio_capturer_log.h"
+#include "audio_noise_reduction_types.h"
 
 namespace OHOS {
 namespace AudioStandard {
@@ -376,6 +377,21 @@ public:
         return sharedAudioCapturer_->SelectInputDevice(desc);
     }
 
+    int32_t SetNoiseReductionMode(NoiseReductionMode mode) override
+    {
+        return sharedAudioCapturer_->SetNoiseReductionMode(mode);
+    }
+
+    int32_t GetNoiseReductionMode(NoiseReductionMode &mode) override
+    {
+        return sharedAudioCapturer_->GetNoiseReductionMode(mode);
+    }
+
+    int32_t GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode>& modes) override
+    {
+        return sharedAudioCapturer_->GetSupportedNoiseReductionModes(modes);
+    }
+
     ~SharedCapturerWrapper() override = default;
 
     SharedCapturerWrapper(const SharedCapturerWrapper&) = delete;
diff --git a/frameworks/native/audiocapturer/src/audio_capturer.cpp b/frameworks/native/audiocapturer/src/audio_capturer.cpp
index 1955358ee68..f5d4a88e72a 100644
--- a/frameworks/native/audiocapturer/src/audio_capturer.cpp
+++ b/frameworks/native/audiocapturer/src/audio_capturer.cpp
@@ -21,6 +21,8 @@
 
 #include <cinttypes>
 #include <cstring>
+#include <sstream>
+#include <algorithm>
 
 #include "audio_capturer_private.h"
 #include "audio_errors.h"
@@ -32,6 +34,7 @@
 #include "audio_info.h"
 #include "app_bundle_manager.h"
 #include "audio_system_client_engine_manager.h"
+#include "audio_system_manager.h"
 #include "audio_capturer_types.h"
 #include "audio_debug_manager.h"
 #include "audio_debug_info.h"
@@ -3422,5 +3425,88 @@ void AudioCapturerPrivate::SetKaraokeReverb(bool enabled) const
     AudioPolicyManager::GetInstance().SetKaraokeParameters(DEVICE_TYPE_SPEAKER, cmdStr);
 }
 
+int32_t AudioCapturerPrivate::SetNoiseReductionMode(NoiseReductionMode mode)
+{
+    AUDIO_INFO_LOG("SetNoiseReductionMode NoiseReductionMode: %{public}d", mode);
+
+    {
+        std::lock_guard<std::recursive_mutex> lock(noiseReductionModesMtx_);
+        if (!hasFetchedSupportedModes_) {
+            std::vector<NoiseReductionMode> tempModes;
+            int32_t ret = GetSupportedNoiseReductionModes(tempModes);
+            if (ret != SUCCESS) {
+                AUDIO_ERR_LOG("SetNoiseReductionMode: failed to get supported modes");
+                return ret;
+            }
+        }
+
+        auto it = std::find(supportedNoiseReductionModes_.begin(),
+                            supportedNoiseReductionModes_.end(), mode);
+        if (it == supportedNoiseReductionModes_.end()) {
+            AUDIO_ERR_LOG("SetNoiseReductionMode: mode %{public}d is not supported", mode);
+            return ERR_INVALID_PARAM;
+        }
+    }
+
+    std::string key = "record_denoise_mode";
+    std::string value = std::to_string(static_cast<int32_t>(mode));
+    AudioSystemManager::GetInstance()->SetAudioParameter(key, value);
+
+    noiseReductionMode_ = mode;
+    return SUCCESS;
+}
+
+int32_t AudioCapturerPrivate::GetNoiseReductionMode(NoiseReductionMode &mode)
+{
+    AUDIO_INFO_LOG("GetNoiseReductionMode NoiseReductionMode: %{public}d", noiseReductionMode_);
+    mode = noiseReductionMode_;
+    return SUCCESS;
+}
+
+namespace {
+NoiseReductionMode StringToNoiseReductionMode(const std::string &modeStr)
+{
+    if (modeStr == "fidelity") {
+        return NOISE_REDUCTION_MODE_FIDELITY;
+    } else if (modeStr == "pure_voice") {
+        return NOISE_REDUCTION_MODE_PURE_VOCALS;
+    } else if (modeStr == "normal") {
+        return NOISE_REDUCTION_MODE_STANDARD;
+    }
+    return NOISE_REDUCTION_MODE_MAX;
+}
+}
+
+int32_t AudioCapturerPrivate::GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode>& modes)
+{
+    AUDIO_INFO_LOG("GetSupportedNoiseReductionModes");
+    modes.clear();
+    modes.push_back(NOISE_REDUCTION_MODE_FIDELITY);
+    std::string key = "record_denoise_mode_support";
+    std::string value = AudioSystemManager::GetInstance()->GetAudioParameter(key);
+    if (value.empty()) {
+        AUDIO_WARNING_LOG("GetSupportedNoiseReductionModes: failed to get from system, use default");
+        return SUCCESS;
+    }
+
+    std::stringstream ss(value);
+    std::string modeStr;
+    while (std::getline(ss, modeStr, ',')) {
+        if (!modeStr.empty()) {
+            NoiseReductionMode mode = StringToNoiseReductionMode(modeStr);
+            if (mode != NOISE_REDUCTION_MODE_MAX) {
+                modes.push_back(mode);
+            }
+        }
+    }
+
+    {
+        std::lock_guard<std::recursive_mutex> lock(noiseReductionModesMtx_);
+        supportedNoiseReductionModes_ = modes;
+        hasFetchedSupportedModes_ = true;
+    }
+    return SUCCESS;
+}
+
 }  // namespace AudioStandard
 }  // namespace OHOS
diff --git a/frameworks/native/audioloopback/test/unittest/include/audio_capturer_mock.h b/frameworks/native/audioloopback/test/unittest/include/audio_capturer_mock.h
index 1b7e37544df..dd822ebb341 100644
--- a/frameworks/native/audioloopback/test/unittest/include/audio_capturer_mock.h
+++ b/frameworks/native/audioloopback/test/unittest/include/audio_capturer_mock.h
@@ -99,6 +99,9 @@ public:
     MOCK_METHOD(int32_t, ReleaseWithError, (), (override));
     MOCK_METHOD(void, SetSensitiveRecordPermitCallback,
         (const std::shared_ptr<AudioCapturerSensitiveRecordPermitCallback> &callback), (override));
+    MOCK_METHOD(int32_t, SetNoiseReductionMode, (NoiseReductionMode mode), (override));
+    MOCK_METHOD(int32_t, GetNoiseReductionMode, (NoiseReductionMode &mode), (override));
+    MOCK_METHOD(int32_t, GetSupportedNoiseReductionModes, (std::vector<NoiseReductionMode> &modes), (override));
 };
 
 } // namespace AudioStandard
diff --git a/frameworks/native/audioutils/include/audio_utils.h b/frameworks/native/audioutils/include/audio_utils.h
index 628382c8660..474ca7ddd0a 100644
--- a/frameworks/native/audioutils/include/audio_utils.h
+++ b/frameworks/native/audioutils/include/audio_utils.h
@@ -645,6 +645,7 @@ enum HdiCaptureOffset : uint32_t {
     HDI_CAPTURE_OFFSET_LIVE = 21,
     HDI_CAPTURE_OFFSET_ONBOARD_MIC = 22,
     HDI_CAPTURE_OFFSET_CAMCORDER = 23,
+    HDI_CAPTURE_OFFSET_VOICE_MESSAGE = 24,
 };
 
 enum HdiRenderOffset : uint32_t {
diff --git a/frameworks/native/hdiadapter_new/include/common/hdi_adapter_info.h b/frameworks/native/hdiadapter_new/include/common/hdi_adapter_info.h
index 44bcd190892..b664fc491a1 100644
--- a/frameworks/native/hdiadapter_new/include/common/hdi_adapter_info.h
+++ b/frameworks/native/hdiadapter_new/include/common/hdi_adapter_info.h
@@ -73,6 +73,7 @@ enum HdiIdType : uint32_t {
 #define HDI_ID_INFO_PRIMARY_EXTRA_CAPTURER_BT_NEARLINK "Built_in_bt"
 #define HDI_ID_INFO_ONBOARD_MIC "onboard_mic"
 #define HDI_ID_INFO_CAMCORDER "camcorder"
+#define HDI_ID_INFO_VOICE_MESSAGE "voice_message"
 
 #define HDI_ID_INFO_PRIMARY_EXTRA_TYPEC "Typec_Speaker"
 #define HDI_ID_INFO_PRIMARY_EXTRA_WIRED "Wired_Speaker"
diff --git a/frameworks/native/hdiadapter_new/manager/hdi_adapter_manager.cpp b/frameworks/native/hdiadapter_new/manager/hdi_adapter_manager.cpp
index 3fb7ce2abad..b614312e0ba 100644
--- a/frameworks/native/hdiadapter_new/manager/hdi_adapter_manager.cpp
+++ b/frameworks/native/hdiadapter_new/manager/hdi_adapter_manager.cpp
@@ -242,6 +242,9 @@ std::optional<std::string> HdiAdapterManager::GetAudioParameterByKey(HdiDeviceMa
         return deviceManager->GetAudioParameter(PRIMARY_ADAPTER_NAME, AudioParamKey::MMI,
             key.substr(strlen(MMI_PREFIX), key.size() - strlen(MMI_PREFIX)));
     }
+    if (key == "record_denoise_mode" || key == "record_denoise_mode_support") {
+        return deviceManager->GetAudioParameter(PRIMARY_ADAPTER_NAME, AudioParamKey::NONE, key);
+    }
     return std::nullopt;
 }
 
diff --git a/frameworks/native/hdiadapter_new/source/audio_capture_source.cpp b/frameworks/native/hdiadapter_new/source/audio_capture_source.cpp
index 074bd201f14..93a8586a10e 100644
--- a/frameworks/native/hdiadapter_new/source/audio_capture_source.cpp
+++ b/frameworks/native/hdiadapter_new/source/audio_capture_source.cpp
@@ -57,6 +57,9 @@ AudioInputType hdiAudioInterphoneInputType = static_cast<AudioInputType>(AUDIO_I
 static constexpr uint32_t AUDIO_INPUT_ONBOARD_MIC_TYPE = 1 << 17;
 AudioInputType hdiAudioOnBoardMicInputType = static_cast<AudioInputType>(AUDIO_INPUT_ONBOARD_MIC_TYPE);
 
+static constexpr uint32_t AUDIO_INPUT_VOICE_MESSAGE_TYPE = 1 << 18;
+AudioInputType hdiAudioVoiceMessageInputType = static_cast<AudioInputType>(AUDIO_INPUT_VOICE_MESSAGE_TYPE);
+
 static constexpr uint32_t MAX_CAMCORDER_METADATA_SIZE = 1024 * 1024;
 static const std::unordered_map<AudioChannelLayout, AudioChannel> MAP_LAYOUT_TO_CHANNEL = {
     {AudioChannelLayout::CH_LAYOUT_MONO, AudioChannel::MONO},
@@ -115,6 +118,8 @@ const std::unordered_map<uint32_t, AudioInputType> INDEPENDENT_PIPE_AUDIO_FLAG_T
     {AUDIO_INPUT_FLAG_RAW_AI, AUDIO_INPUT_RAW_AI_TYPE},
     {AUDIO_INPUT_FLAG_CAMCORDER, AUDIO_INPUT_CAMCORDER_TYPE},
     {AUDIO_INPUT_FLAG_INTERPHONE, hdiAudioInterphoneInputType},
+    {AUDIO_INPUT_ONBOARD_MIC_TYPE, hdiAudioOnBoardMicInputType},
+    {AUDIO_INPUT_VOICE_MESSAGE_TYPE, hdiAudioVoiceMessageInputType},
 };
 
 static AudioInputType ConvertAudioFlagToHdiAudioInputType(uint32_t routeFlag)
@@ -956,7 +961,8 @@ const std::unordered_map<std::string, AudioInputType> AudioCaptureSource::audioI
     {"AUDIO_INPUT_RAW_AI_TYPE", AUDIO_INPUT_RAW_AI_TYPE},
     {"AUDIO_INPUT_CAMCORDER_TYPE", AUDIO_INPUT_CAMCORDER_TYPE},
     {"AUDIO_INPUT_ONBOARD_MIC_TYPE", hdiAudioOnBoardMicInputType},
-    {"AUDIO_INPUT_INTERPHONE_TYPE", hdiAudioInterphoneInputType}
+    {"AUDIO_INPUT_INTERPHONE_TYPE", hdiAudioInterphoneInputType},
+    {"AUDIO_INPUT_VOICE_MESSAGE_TYPE", hdiAudioVoiceMessageInputType}
 };
 
 AudioInputType AudioCaptureSource::MappingAudioInputType(std::string hdiSourceType)
@@ -1028,6 +1034,9 @@ enum AudioInputType AudioCaptureSource::ConvertToHDIAudioInputType(int32_t sourc
         case SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT: // This configuration uses the old channel.
             hdiAudioInputType = AUDIO_INPUT_VOICE_TRANSCRIPTION;
             break;
+        case SOURCE_TYPE_VOICE_MESSAGE:
+            hdiAudioInputType = hdiAudioVoiceMessageInputType;
+            break;
         case SOURCE_TYPE_INTERPHONE:
             hdiAudioInputType = hdiAudioInterphoneInputType;
             break;
@@ -1163,6 +1172,9 @@ uint32_t AudioCaptureSource::GenerateUniqueIDByHdiSource(AudioInputType hdiSourc
     if (hdiSource == static_cast<AudioInputType>(AUDIO_INPUT_ONBOARD_MIC_TYPE)) {
         return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_ONBOARD_MIC);
     }
+    if (hdiSource == static_cast<AudioInputType>(AUDIO_INPUT_VOICE_MESSAGE_TYPE)) {
+        return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_VOICE_MESSAGE);
+    }
     switch (hdiSource) {
         case AUDIO_INPUT_VOICE_TRANSCRIPTION:
             return GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_VOICE_TRANSCRIPTION);
@@ -1835,7 +1847,6 @@ enum AudioFlag AudioCaptureSource::ConvertToAudioInputRouteFlag(int32_t sourceTy
             break;
         case SOURCE_TYPE_MIC:
         case SOURCE_TYPE_CAMCORDER:
-        case SOURCE_TYPE_VOICE_MESSAGE:
         case SOURCE_TYPE_VOICE_CALL:
         case SOURCE_TYPE_VOICE_DOWNLINK:
             audioInputRouteFlag = AUDIO_INPUT_FLAG_NORMAL;
@@ -1861,6 +1872,9 @@ enum AudioFlag AudioCaptureSource::ConvertToAudioInputRouteFlag(int32_t sourceTy
         case SOURCE_TYPE_ONBOARD_MIC:
             audioInputRouteFlag = AUDIO_INPUT_FLAG_ONBOARD_MIC;
             break;
+        case SOURCE_TYPE_VOICE_MESSAGE:
+            audioInputRouteFlag = AUDIO_INPUT_FLAG_VOICE_MESSAGE;
+            break;
         default:
             audioInputRouteFlag = AUDIO_FLAG_NONE;
             break;
diff --git a/frameworks/native/hdiadapter_new/source/fast_audio_capture_source.cpp b/frameworks/native/hdiadapter_new/source/fast_audio_capture_source.cpp
index 7b0e92bb868..4e3db6d476a 100644
--- a/frameworks/native/hdiadapter_new/source/fast_audio_capture_source.cpp
+++ b/frameworks/native/hdiadapter_new/source/fast_audio_capture_source.cpp
@@ -401,6 +401,7 @@ enum AudioInputType FastAudioCaptureSource::ConvertToHDIAudioInputType(int32_t s
         case SOURCE_TYPE_ULTRASONIC:
         case SOURCE_TYPE_UNPROCESSED:
         case SOURCE_TYPE_ONBOARD_MIC:
+        case SOURCE_TYPE_VOICE_MESSAGE:
         case SOURCE_TYPE_LIVE:
             hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
             break;
diff --git a/frameworks/native/hdiadapter_new/util/id_handler.cpp b/frameworks/native/hdiadapter_new/util/id_handler.cpp
index 644352f022c..cbdb7f9c292 100644
--- a/frameworks/native/hdiadapter_new/util/id_handler.cpp
+++ b/frameworks/native/hdiadapter_new/util/id_handler.cpp
@@ -142,6 +142,8 @@ uint32_t IdHandler::GetIdForSolePipeSource(SourceType sourceType, uint32_t route
         return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_ONBOARD_MIC);
     } else if (routeFlag == AUDIO_INPUT_FLAG_CAMCORDER) {
         return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_CAMCORDER);
+    } else if (routeFlag == AUDIO_INPUT_FLAG_VOICE_MESSAGE) {
+        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_VOICE_MESSAGE);
     }
     return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT);
 }
diff --git a/frameworks/native/ohaudio/OHAudioCapturer.cpp b/frameworks/native/ohaudio/OHAudioCapturer.cpp
index 8279507fa54..cad3d5b279a 100644
--- a/frameworks/native/ohaudio/OHAudioCapturer.cpp
+++ b/frameworks/native/ohaudio/OHAudioCapturer.cpp
@@ -23,6 +23,7 @@
 #include "audio_utils.h"
 #include "OHAudioDeviceDescriptor.h"
 #include "OHAudioCommon.h"
+#include "audio_noise_reduction_types.h"
 #include "stream_dfx_manager.h"
 #include <unistd.h>
 static const int32_t START_RESULT_SUCCESS = 0;
@@ -36,6 +37,11 @@ static OHOS::AudioStandard::OHAudioCapturer *convertCapturer(OH_AudioCapturer* c
     return (OHOS::AudioStandard::OHAudioCapturer*) capturer;
 }
 
+static bool IsValidNoiseReductionMode(OH_AudioNoiseReductionMode mode)
+{
+    return mode >= AUDIO_NOISE_REDUCTION_MODE_FIDELITY &&
+        mode <= AUDIO_NOISE_REDUCTION_MODE_STANDARD;
+}
 OH_AudioStream_Result OH_AudioCapturer_Release(OH_AudioCapturer* capturer)
 {
     AUDIO_INFO_LOG("in");
@@ -422,6 +428,73 @@ OH_AudioStream_Result OH_AudioCapturer_GetOverflowCount(OH_AudioCapturer* captur
     return AUDIOSTREAM_SUCCESS;
 }
 
+OH_AudioStream_Result OH_AudioCapturer_SetNoiseReductionMode(OH_AudioCapturer* capturer,
+    OH_AudioNoiseReductionMode noiseReductionMode)
+{
+    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
+    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
+        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
+            OHOS::AudioStandard::RECORD_CONFIG_INVALID_PARAM, "convert capturer failed", true),
+        "convert capturer failed");
+    CHECK_AND_RETURN_RET_LOG(IsValidNoiseReductionMode(noiseReductionMode), AUDIOSTREAM_ERROR_INVALID_PARAM,
+        "noiseReductionMode is invalid");
+
+    auto mode = static_cast<OHOS::AudioStandard::NoiseReductionMode>(noiseReductionMode);
+    int32_t ret = audioCapturer->SetNoiseReductionMode(mode);
+    if (ret == OHOS::AudioStandard::ERR_INVALID_PARAM || ret == OHOS::AudioStandard::ERROR_INVALID_PARAM) {
+        return AUDIOSTREAM_ERROR_INVALID_PARAM;
+    }
+    return ret == OHOS::AudioStandard::SUCCESS ? AUDIOSTREAM_SUCCESS : AUDIOSTREAM_ERROR_ILLEGAL_STATE;
+}
+
+OH_AudioStream_Result OH_AudioCapturer_GetNoiseReductionMode(
+    OH_AudioCapturer* capturer, OH_AudioNoiseReductionMode* noiseReductionMode)
+{
+    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
+    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
+        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
+            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
+        "convert capturer failed");
+    CHECK_AND_RETURN_RET_LOG(noiseReductionMode != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
+        "noiseReductionMode is nullptr");
+
+    OHOS::AudioStandard::NoiseReductionMode mode = OHOS::AudioStandard::NOISE_REDUCTION_MODE_FIDELITY;
+    int32_t ret = audioCapturer->GetNoiseReductionMode(mode);
+    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, AUDIOSTREAM_ERROR_ILLEGAL_STATE,
+        "GetNoiseReductionMode failed");
+
+    *noiseReductionMode = static_cast<OH_AudioNoiseReductionMode>(mode);
+    return AUDIOSTREAM_SUCCESS;
+}
+
+OH_AudioStream_Result OH_AudioCapturer_GetSupportedNoiseReductionModes(
+    OH_AudioCapturer* capturer, OH_AudioNoiseReductionMode** noiseReductionModes)
+{
+    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
+    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
+        OHOS::AudioStandard::StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
+            OHOS::AudioStandard::RECORD_QUERY_ILLEGAL_STATE, "convert capturer failed", true),
+        "convert capturer failed");
+    CHECK_AND_RETURN_RET_LOG(noiseReductionModes != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM,
+    "noiseReductionModes is nullptr");
+    std::vector<OHOS::AudioStandard::NoiseReductionMode> nativeModes;
+    int32_t ret = audioCapturer->GetSupportedNoiseReductionModes(nativeModes);
+    CHECK_AND_RETURN_RET_LOG(ret == OHOS::AudioStandard::SUCCESS, AUDIOSTREAM_ERROR_ILLEGAL_STATE,
+        "GetSupportedNoiseReductionModes failed");
+ 
+    *noiseReductionModes = static_cast<OH_AudioNoiseReductionMode*>(malloc(
+        (nativeModes.size() + 1) * sizeof(OH_AudioNoiseReductionMode)));
+    if (*noiseReductionModes == nullptr) {
+        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
+    }
+ 
+    for (size_t i = 0; i < nativeModes.size(); ++i) {
+        (*noiseReductionModes)[i] = static_cast<OH_AudioNoiseReductionMode>(nativeModes[i]);
+    }
+ 
+    return AUDIOSTREAM_SUCCESS;
+}
+
 namespace OHOS {
 namespace AudioStandard {
 OHAudioCapturer::OHAudioCapturer()
@@ -1102,6 +1175,42 @@ int32_t OHAudioCapturer::SetIndependentAudioSessionStrategy(
     return audioCapturer_->SetIndependentAudioSessionStrategy(strategy, behavior);
 }
 
+int32_t OHAudioCapturer::SetNoiseReductionMode(NoiseReductionMode mode)
+{
+    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
+    return audioCapturer_->SetNoiseReductionMode(mode);
+}
+
+int32_t OHAudioCapturer::GetNoiseReductionMode(NoiseReductionMode &mode)
+{
+    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
+    return audioCapturer_->GetNoiseReductionMode(mode);
+}
+
+int32_t OHAudioCapturer::GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode>& modes)
+{
+    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
+    return audioCapturer_->GetSupportedNoiseReductionModes(modes);
+}
+
+int32_t OHAudioCapturer::GetSupportedNoiseReductionModes(OH_AudioNoiseReductionMode **modes)
+{
+    CHECK_AND_RETURN_RET_LOG(modes != nullptr, ERR_INVALID_PARAM, "modes is nullptr");
+    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
+
+    std::vector<NoiseReductionMode> nativeModes;
+    int32_t ret = audioCapturer_->GetSupportedNoiseReductionModes(nativeModes);
+    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSupportedNoiseReductionModes failed");
+
+    supportedNoiseReductionModes_.clear();
+    supportedNoiseReductionModes_.reserve(nativeModes.size());
+    for (const auto &mode : nativeModes) {
+        supportedNoiseReductionModes_.emplace_back(static_cast<OH_AudioNoiseReductionMode>(mode));
+    }
+    *modes = supportedNoiseReductionModes_.data();
+    return SUCCESS;
+}
+
 std::shared_ptr<AudioCapturer> OHAudioCapturer::GetAudioCapturer()
 {
     return audioCapturer_;
diff --git a/frameworks/native/ohaudio/OHAudioCapturer.h b/frameworks/native/ohaudio/OHAudioCapturer.h
index 7e5d40b9b44..cb39b94b2fd 100644
--- a/frameworks/native/ohaudio/OHAudioCapturer.h
+++ b/frameworks/native/ohaudio/OHAudioCapturer.h
@@ -16,11 +16,14 @@
 #ifndef OH_AUDIO_CAPTURER_H
 #define OH_AUDIO_CAPTURER_H
 
+#include <vector>
+
 #include "native_audiocapturer.h"
 #include "native_audio_session_base.h"
 #include "audio_capturer.h"
 #include "audio_common_log.h"
 #include "OHAudioDeviceDescriptor.h"
+#include "audio_noise_reduction_types.h"
 
 namespace OHOS {
 namespace AudioStandard {
@@ -239,6 +242,10 @@ public:
         OH_AudioCapturer_OnPlaybackCaptureStartCallback callback, void* userData);
     bool IsModernInnerCapturer();
     int32_t SetIndependentAudioSessionStrategy(const AudioSessionStrategy &strategy, const uint32_t behavior);
+    int32_t SetNoiseReductionMode(NoiseReductionMode mode);
+    int32_t GetNoiseReductionMode(NoiseReductionMode &mode);
+    int32_t GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode>& modes);
+    int32_t GetSupportedNoiseReductionModes(OH_AudioNoiseReductionMode **modes);
     std::shared_ptr<AudioCapturer> GetAudioCapturer();
 
 private:
@@ -251,6 +258,7 @@ private:
     std::shared_ptr<OHAudioCapturerFastStatusChangeCallback> audioCapturerFastStatusChangeCallback_;
     std::shared_ptr<OHAudioCapturerSensitiveRecordPermitCallback> audioCapturerSensitiveRecordPermitCallback_;
     std::shared_ptr<OHAudioCapturerOnPlaybackCaptureStartCallback> capturerOnPlaybackCaptureStartCallback_ = nullptr;
+    std::vector<OH_AudioNoiseReductionMode> supportedNoiseReductionModes_;
     bool isModernInnerCapturer_ = false;
 };
 }  // namespace AudioStandard
diff --git a/frameworks/taihe/audiocapturer/taihe_audio_capturer.cpp b/frameworks/taihe/audiocapturer/taihe_audio_capturer.cpp
index aedbd82468f..6ca0b64caa2 100644
--- a/frameworks/taihe/audiocapturer/taihe_audio_capturer.cpp
+++ b/frameworks/taihe/audiocapturer/taihe_audio_capturer.cpp
@@ -615,6 +615,67 @@ void AudioCapturerImpl::SetMuteHintSync(bool mute)
     }
 }
 
+void AudioCapturerImpl::SetNoiseReductionMode(ohos::multimedia::audio::NoiseReductionMode noiseReductionMode)
+{
+    int32_t modeValue = noiseReductionMode.get_value();
+    if (!TaiheAudioEnum::IsLegalInputArgumentNoiseReductionMode(modeValue)) {
+        AUDIO_ERR_LOG("get noiseReductionMode failed: %{public}d", modeValue);
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
+            "parameter verification failed: The param of noiseReductionMode must be enum NoiseReductionMode");
+        return;
+    }
+    if (audioCapturer_ == nullptr) {
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
+        return;
+    }
+
+    int32_t ret = audioCapturer_->SetNoiseReductionMode(
+        static_cast<OHOS::AudioStandard::NoiseReductionMode>(modeValue));
+    if (ret == OHOS::AudioStandard::ERROR_INVALID_PARAM || ret == OHOS::AudioStandard::ERR_INVALID_PARAM) {
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "SetNoiseReductionMode failed");
+    } else if (ret != OHOS::AudioStandard::SUCCESS) {
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "SetNoiseReductionMode failed");
+    }
+}
+
+ohos::multimedia::audio::NoiseReductionMode AudioCapturerImpl::GetNoiseReductionMode()
+{
+    if (audioCapturer_ == nullptr) {
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
+        return ohos::multimedia::audio::NoiseReductionMode::key_t::FIDELITY;
+    }
+
+    OHOS::AudioStandard::NoiseReductionMode noiseReductionMode =
+        OHOS::AudioStandard::NOISE_REDUCTION_MODE_FIDELITY;
+    int32_t ret = audioCapturer_->GetNoiseReductionMode(noiseReductionMode);
+    if (ret != OHOS::AudioStandard::SUCCESS) {
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "GetNoiseReductionMode failed");
+        return ohos::multimedia::audio::NoiseReductionMode::key_t::FIDELITY;
+    }
+    return TaiheAudioEnum::ToTaiheNoiseReductionMode(noiseReductionMode);
+}
+
+taihe::array<ohos::multimedia::audio::NoiseReductionMode> AudioCapturerImpl::GetSupportedNoiseReductionModes()
+{
+    std::vector<ohos::multimedia::audio::NoiseReductionMode> result;
+    if (audioCapturer_ == nullptr) {
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioCapturer_ is nullptr");
+        return taihe::array<ohos::multimedia::audio::NoiseReductionMode>(result);
+    }
+
+    std::vector<OHOS::AudioStandard::NoiseReductionMode> nativeModes;
+    int32_t ret = audioCapturer_->GetSupportedNoiseReductionModes(nativeModes);
+    if (ret != OHOS::AudioStandard::SUCCESS) {
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_ILLEGAL_STATE, "GetSupportedNoiseReductionModes failed");
+        return taihe::array<ohos::multimedia::audio::NoiseReductionMode>(result);
+    }
+
+    for (const auto &mode : nativeModes) {
+        result.emplace_back(TaiheAudioEnum::ToTaiheNoiseReductionMode(mode));
+    }
+    return taihe::array<ohos::multimedia::audio::NoiseReductionMode>(result);
+}
+
 void AudioCapturerImpl::RequestPlaybackCaptureStart(callback_view<void(PlaybackCaptureStartState)> callback)
 {
     if (audioCapturer_ == nullptr) {
diff --git a/frameworks/taihe/audiocapturer/taihe_audio_capturer.h b/frameworks/taihe/audiocapturer/taihe_audio_capturer.h
index a92656539d0..5f362765af6 100644
--- a/frameworks/taihe/audiocapturer/taihe_audio_capturer.h
+++ b/frameworks/taihe/audiocapturer/taihe_audio_capturer.h
@@ -63,6 +63,9 @@ public:
     int64_t GetOverflowCountSync();
     void SetWillMuteWhenInterruptedSync(bool muteWhenInterrupted);
     void SetMuteHintSync(bool mute);
+    void SetNoiseReductionMode(ohos::multimedia::audio::NoiseReductionMode noiseReductionMode);
+    ohos::multimedia::audio::NoiseReductionMode GetNoiseReductionMode();
+    taihe::array<ohos::multimedia::audio::NoiseReductionMode> GetSupportedNoiseReductionModes();
     void RequestPlaybackCaptureStart(callback_view<void(PlaybackCaptureStartState)> callback);
     taihe::array<AudioDeviceDescriptor> GetCurrentInputDevices();
     AudioCapturerChangeInfo GetCurrentAudioCapturerChangeInfo();
diff --git a/frameworks/taihe/common/taihe_audio_enum.cpp b/frameworks/taihe/common/taihe_audio_enum.cpp
index 52016a17a18..08d29123634 100644
--- a/frameworks/taihe/common/taihe_audio_enum.cpp
+++ b/frameworks/taihe/common/taihe_audio_enum.cpp
@@ -241,6 +241,16 @@ static const std::map<OHOS::AudioStandard::AudioEffectMode, AudioEffectMode> AUD
     {OHOS::AudioStandard::AudioEffectMode::EFFECT_DEFAULT, AudioEffectMode::key_t::EFFECT_DEFAULT},
 };
 
+static const std::map<OHOS::AudioStandard::NoiseReductionMode, ohos::multimedia::audio::NoiseReductionMode>
+    NOISE_REDUCTION_MODE_TAIHE_MAP = {
+    {OHOS::AudioStandard::NOISE_REDUCTION_MODE_FIDELITY,
+        ohos::multimedia::audio::NoiseReductionMode::key_t::FIDELITY},
+    {OHOS::AudioStandard::NOISE_REDUCTION_MODE_PURE_VOCALS,
+        ohos::multimedia::audio::NoiseReductionMode::key_t::PURE_VOCALS},
+    {OHOS::AudioStandard::NOISE_REDUCTION_MODE_STANDARD,
+        ohos::multimedia::audio::NoiseReductionMode::key_t::STANDARD},
+};
+
 static const std::map<OHOS::AudioStandard::AudioStreamDeviceChangeReason, AudioStreamDeviceChangeReason>
     AUDIO_STREAM_DEVICE_CHANGE_REASON_TAIHE_MAP = {
     {OHOS::AudioStandard::AudioStreamDeviceChangeReason::UNKNOWN, AudioStreamDeviceChangeReason::key_t::REASON_UNKNOWN},
@@ -484,6 +494,22 @@ bool TaiheAudioEnum::IsLegalInputArgumentAudioEffectMode(int32_t audioEffectMode
     return result;
 }
 
+bool TaiheAudioEnum::IsLegalInputArgumentNoiseReductionMode(int32_t noiseReductionMode)
+{
+    bool result = false;
+    switch (noiseReductionMode) {
+        case OHOS::AudioStandard::NOISE_REDUCTION_MODE_FIDELITY:
+        case OHOS::AudioStandard::NOISE_REDUCTION_MODE_PURE_VOCALS:
+        case OHOS::AudioStandard::NOISE_REDUCTION_MODE_STANDARD:
+            result = true;
+            break;
+        default:
+            result = false;
+            break;
+    }
+    return result;
+}
+
 bool TaiheAudioEnum::IsLegalInputArgumentChannelBlendMode(int32_t blendMode)
 {
     bool result = false;
@@ -1544,6 +1570,18 @@ AudioEffectMode TaiheAudioEnum::ToTaiheAudioEffectMode(OHOS::AudioStandard::Audi
     return iter->second;
 }
 
+ohos::multimedia::audio::NoiseReductionMode TaiheAudioEnum::ToTaiheNoiseReductionMode(
+    OHOS::AudioStandard::NoiseReductionMode mode)
+{
+    auto iter = NOISE_REDUCTION_MODE_TAIHE_MAP.find(mode);
+    if (iter == NOISE_REDUCTION_MODE_TAIHE_MAP.end()) {
+        AUDIO_WARNING_LOG("ToTaiheNoiseReductionMode invalid mode: %{public}d", static_cast<int32_t>(mode));
+        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "ToTaiheNoiseReductionMode fail");
+        return ohos::multimedia::audio::NoiseReductionMode::key_t::FIDELITY;
+    }
+    return iter->second;
+}
+
 AudioStreamDeviceChangeReason TaiheAudioEnum::ToTaiheAudioStreamDeviceChangeReason(
     OHOS::AudioStandard::AudioStreamDeviceChangeReason reason)
 {
diff --git a/frameworks/taihe/common/taihe_audio_enum.h b/frameworks/taihe/common/taihe_audio_enum.h
index f271eee332d..3fc494f50a4 100644
--- a/frameworks/taihe/common/taihe_audio_enum.h
+++ b/frameworks/taihe/common/taihe_audio_enum.h
@@ -22,6 +22,7 @@
 #include "audio_log.h"
 #include "audio_effect.h"
 #include "audio_session_device_info.h"
+#include "audio_noise_reduction_types.h"
 
 namespace ANI::Audio {
 using namespace taihe;
@@ -107,6 +108,7 @@ public:
 
     static bool IsLegalInputArgumentInterruptMode(int32_t interruptMode);
     static bool IsLegalInputArgumentAudioEffectMode(int32_t audioEffectMode);
+    static bool IsLegalInputArgumentNoiseReductionMode(int32_t noiseReductionMode);
     static bool IsLegalInputArgumentChannelBlendMode(int32_t blendMode);
     static bool IsLegalCapturerType(int32_t type);
     static bool IsLegalInputArgumentVolType(int32_t inputType);
@@ -168,6 +170,8 @@ public:
     static AudioVolumeType ToTaiheAudioVolumeType(TaiheAudioEnum::AudioJsVolumeType type);
     static ohos::multimedia::audio::AudioRingMode ToTaiheAudioRingMode(OHOS::AudioStandard::AudioRingerMode mode);
     static AudioEffectMode ToTaiheAudioEffectMode(OHOS::AudioStandard::AudioEffectMode mode);
+    static ohos::multimedia::audio::NoiseReductionMode ToTaiheNoiseReductionMode(
+        OHOS::AudioStandard::NoiseReductionMode mode);
     static AudioStreamDeviceChangeReason ToTaiheAudioStreamDeviceChangeReason(
         OHOS::AudioStandard::AudioStreamDeviceChangeReason reason);
     static AudioChannelLayout ToTaiheAudioChannelLayout(OHOS::AudioStandard::AudioChannelLayout layout);
diff --git a/frameworks/taihe/idl/ohos.multimedia.audio.taihe b/frameworks/taihe/idl/ohos.multimedia.audio.taihe
index 34b1a17fdeb..349122bbf11 100644
--- a/frameworks/taihe/idl/ohos.multimedia.audio.taihe
+++ b/frameworks/taihe/idl/ohos.multimedia.audio.taihe
@@ -1083,7 +1083,11 @@ interface AudioCapturer {
 
   @gen_promise("setMuteHint")
   SetMuteHintSync(mute: bool): void;
+  SetNoiseReductionMode(noiseReductionMode: NoiseReductionMode): void;
 
+  GetNoiseReductionMode(): NoiseReductionMode;
+
+  GetSupportedNoiseReductionModes(): Array<NoiseReductionMode>;
   RequestPlaybackCaptureStart(callback: (data: PlaybackCaptureStartState) => void): void;
 
   GetCurrentInputDevices(): Array<AudioDeviceDescriptor>;
@@ -1267,6 +1271,12 @@ enum AudioEffectMode: i32 {
   EFFECT_DEFAULT = 1,
 }
 
+enum NoiseReductionMode: i32 {
+  FIDELITY = 0,
+  PURE_VOCALS = 1,
+  STANDARD = 2,
+}
+
 struct AudioSpatialDeviceState {
   address: String;
   isSpatializationSupported: bool;
diff --git a/interfaces/inner_api/native/audiocapturer/include/audio_capturer.h b/interfaces/inner_api/native/audiocapturer/include/audio_capturer.h
index a75c4cc94dc..d63bdf14072 100644
--- a/interfaces/inner_api/native/audiocapturer/include/audio_capturer.h
+++ b/interfaces/inner_api/native/audiocapturer/include/audio_capturer.h
@@ -23,6 +23,7 @@
 #include "microphone_descriptor.h"
 #include "timestamp.h"
 #include "audio_capturer_options.h"
+#include "audio_noise_reduction_types.h"
 
 namespace OHOS {
 namespace AudioStandard {
@@ -921,6 +922,24 @@ public:
 
     virtual uint32_t GetOverflowCount() const = 0;
 
+    virtual int32_t SetNoiseReductionMode(NoiseReductionMode mode)
+    {
+        (void)mode;
+        return ERR_NOT_SUPPORTED;
+    }
+
+    virtual int32_t GetNoiseReductionMode(NoiseReductionMode &mode)
+    {
+        (void)mode;
+        return ERR_NOT_SUPPORTED;
+    }
+
+    virtual int32_t GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode>& modes)
+    {
+        (void)modes;
+        return ERR_NOT_SUPPORTED;
+    }
+
     /**
      * @brief Select the input device for the current capturer.
      *
diff --git a/interfaces/inner_api/native/audiocommon/include/audio_pipe_types.h b/interfaces/inner_api/native/audiocommon/include/audio_pipe_types.h
index 44b8a726f3a..44039a1f599 100644
--- a/interfaces/inner_api/native/audiocommon/include/audio_pipe_types.h
+++ b/interfaces/inner_api/native/audiocommon/include/audio_pipe_types.h
@@ -83,6 +83,7 @@ enum AudioFlag : uint32_t {
     AUDIO_INPUT_FLAG_LIVE = 0x2000000,
     AUDIO_INPUT_FLAG_ONBOARD_MIC = 0x4000000,
     AUDIO_INPUT_FLAG_CAMCORDER = 0x8000000,
+    AUDIO_INPUT_FLAG_VOICE_MESSAGE = 0x40000000,
     AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT = 0x10000000,
     AUDIO_FLAG_MAX,
 };
diff --git a/interfaces/inner_api/native/audiocommon/include/audio_stream_info.h b/interfaces/inner_api/native/audiocommon/include/audio_stream_info.h
index 3b0e442b97a..1b3d02e22e2 100755
--- a/interfaces/inner_api/native/audiocommon/include/audio_stream_info.h
+++ b/interfaces/inner_api/native/audiocommon/include/audio_stream_info.h
@@ -284,6 +284,7 @@ enum AudioPipeType {
     PIPE_TYPE_IN_NORMAL_ONBOARD_MIC = 22,
     PIPE_TYPE_IN_CAMCORDER = 23,
     PIPE_TYPE_OUT_VOICE_CALL_ASSISTANT = 24,
+    PIPE_TYPE_IN_NORMAL_VOICE_MESSAGE = 25,
 };
 
 enum AudioPreloadType {
diff --git a/interfaces/kits/c/audio_capturer/native_audiocapturer.h b/interfaces/kits/c/audio_capturer/native_audiocapturer.h
index c7a9f529391..1f60acd77f5 100644
--- a/interfaces/kits/c/audio_capturer/native_audiocapturer.h
+++ b/interfaces/kits/c/audio_capturer/native_audiocapturer.h
@@ -39,6 +39,7 @@
 #define NATIVE_AUDIOCAPTURER_H
 
 #include <time.h>
+#include "native_audio_common.h"
 #include "native_audiostream_base.h"
 #include "native_audio_device_base.h"
 #include "native_audio_session_base.h"
@@ -427,6 +428,51 @@ typedef void (*OH_AudioCapturer_SensitiveRecordPermitCallback)(
     void* userData,
     bool isPermitted);
 
+/**
+ * @brief Sets noise reduction mode on current audio capturer.
+ *
+ * @param capturer Audio capturer created by OH_AudioStreamBuilder_GenerateCapturer().
+ * @param noiseReductionMode Noise reduction mode to set, refer to {@link OH_AudioNoiseReductionMode}.
+ * @return Function result code:
+ *         {@link AUDIOSTREAM_SUCCESS} If the execution is successful.
+ *         {@link AUDIOSTREAM_ERROR_INVALID_PARAM} The param of capturer is nullptr or noiseReductionMode is invalid.
+ *         {@link AUDIOSTREAM_ERROR_ILLEGAL_STATE} Stream is in running or released state.
+ * @since 26.0.0
+ */
+OH_AudioStream_Result OH_AudioCapturer_SetNoiseReductionMode(OH_AudioCapturer* capturer,
+    OH_AudioNoiseReductionMode noiseReductionMode);
+
+/**
+ * @brief Gets noise reduction mode on current audio capturer.
+ *
+ * @param capturer Audio capturer created by OH_AudioStreamBuilder_GenerateCapturer().
+ * @param noiseReductionMode Pointer to the variable where noise reduction mode will be stored,
+ *                           refer to {@link OH_AudioNoiseReductionMode}.
+ * @return Function result code:
+ *         {@link AUDIOSTREAM_SUCCESS} If the execution is successful.
+ *         {@link AUDIOSTREAM_ERROR_INVALID_PARAM} The param of capturer or noiseReductionMode is nullptr.
+ *         {@link AUDIOSTREAM_ERROR_ILLEGAL_STATE} Stream is in released state.
+ * @since 26.0.0
+ */
+OH_AudioStream_Result OH_AudioCapturer_GetNoiseReductionMode(OH_AudioCapturer* capturer,
+    OH_AudioNoiseReductionMode* noiseReductionMode);
+
+/**
+ * @brief Gets all supported noise reduction modes on current audio capturer.
+ *
+ * @param capturer Audio capturer created by OH_AudioStreamBuilder_GenerateCapturer().
+ * @param noiseReductionModes Pointer to receive the array of supported noise reduction modes.
+ *                            The array is managed by the system. The caller should not free this memory.
+ *                            Refer to {@link OH_AudioNoiseReductionMode}.
+ * @return Function result code:
+ *         {@link AUDIOSTREAM_SUCCESS} If the execution is successful.
+ *         {@link AUDIOSTREAM_ERROR_INVALID_PARAM} The param of capturer or noiseReductionModes is nullptr.
+ *         {@link AUDIOSTREAM_ERROR_ILLEGAL_STATE} Stream is in released state.
+ * @since 26.0.0
+ */
+OH_AudioStream_Result OH_AudioCapturer_GetSupportedNoiseReductionModes(OH_AudioCapturer* capturer,
+    OH_AudioNoiseReductionMode** noiseReductionModes);
+
 #ifdef __cplusplus
 }
 #endif
diff --git a/interfaces/kits/c/common/native_audio_common.h b/interfaces/kits/c/common/native_audio_common.h
index 883b7d94c31..5b2bdcef860 100644
--- a/interfaces/kits/c/common/native_audio_common.h
+++ b/interfaces/kits/c/common/native_audio_common.h
@@ -166,6 +166,34 @@ typedef enum {
     AUDIO_RINGER_MODE_NORMAL = 2,
 } OH_AudioRingerMode;
 
+/**
+* @brief Enumerates the noise reduction modes.
+*
+* @since 26.0.0
+*/
+typedef enum {
+    /**
+    * @brief Fidelity mode, no noise reduction.
+    *
+    * @since 26.0.0
+    */
+    AUDIO_NOISE_REDUCTION_MODE_FIDELITY = 0,
+ 
+    /**
+    * @brief Pure vocals mode, strong noise reduction.
+    *
+    * @since 26.0.0
+    */
+    AUDIO_NOISE_REDUCTION_MODE_PURE_VOCALS = 1,
+ 
+    /**
+    * @brief Standard mode, weak noise reduction.
+    *
+    * @since 26.0.0
+    */
+    AUDIO_NOISE_REDUCTION_MODE_STANDARD = 2,
+} OH_AudioNoiseReductionMode;
+
 #ifdef __cplusplus
 }
 #endif
diff --git a/services/audio_policy/common/include/audio_module_info.h b/services/audio_policy/common/include/audio_module_info.h
index 458fa5ad18e..0220b6788e2 100644
--- a/services/audio_policy/common/include/audio_module_info.h
+++ b/services/audio_policy/common/include/audio_module_info.h
@@ -84,6 +84,7 @@ static const char* PRIMARY_VOICE_RECOGNITION_MIC = "Built_in_voice_recognition";
 static const char* PRIMARY_RAW_AI_MIC = "Built_in_raw_ai";
 static const char* PRIMARY_INTERPHONE_MIC = "Built_in_interphone";
 static const char* PRIMARY_CAMCORDER = "Built_in_camcorder";
+static const char* PRIMARY_VOICE_MESSAGE_MIC = "Built_in_voice_message";
 static const char* PRIMARY_SPEAKER_PIPENAME = "primary_output";
 static const char* USB_SPEAKER_PIPENAME = "usb_arm_output";
 static const char* USB_MIC_PIPENAME = "usb_arm_input";
diff --git a/services/audio_policy/server/domain/device/src/audio_device_common.cpp b/services/audio_policy/server/domain/device/src/audio_device_common.cpp
index 4a7c82327e8..107ec8733a6 100755
--- a/services/audio_policy/server/domain/device/src/audio_device_common.cpp
+++ b/services/audio_policy/server/domain/device/src/audio_device_common.cpp
@@ -85,7 +85,8 @@ static const std::vector<std::string> SourceNames = {
     std::string(PRIMARY_RAW_AI_MIC),
     std::string(PRIMARY_INTERPHONE_MIC),
     std::string(PRIMARY_ONBOARD_MIC),
-    std::string(PRIMARY_CAMCORDER)
+    std::string(PRIMARY_CAMCORDER),
+    std::string(PRIMARY_VOICE_MESSAGE_MIC)
 };
 
 static bool IsDistributedOutput(const AudioDeviceDescriptor &desc)
diff --git a/services/audio_policy/server/domain/pipe/src/audio_concurrency_manager.cpp b/services/audio_policy/server/domain/pipe/src/audio_concurrency_manager.cpp
index c4c623b41f8..eb0946c38bb 100644
--- a/services/audio_policy/server/domain/pipe/src/audio_concurrency_manager.cpp
+++ b/services/audio_policy/server/domain/pipe/src/audio_concurrency_manager.cpp
@@ -44,6 +44,7 @@ static const std::vector<std::pair<AudioMode, uint32_t>> g_priorityOrder = {
     {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_AI},
     {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_LIVE},
     {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_ONBOARD_MIC},
+    {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_VOICE_MESSAGE},
     {AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_NORMAL},
 };
 
@@ -64,6 +65,7 @@ static const std::map<std::pair<AudioMode, uint32_t>, AudioPipeType> g_pipeTypeM
     {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_AI}, PIPE_TYPE_IN_NORMAL_AI},
     {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_LIVE}, PIPE_TYPE_IN_NORMAL_LIVE},
     {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_ONBOARD_MIC}, PIPE_TYPE_IN_NORMAL_ONBOARD_MIC},
+    {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_VOICE_MESSAGE}, PIPE_TYPE_IN_NORMAL_VOICE_MESSAGE},
     {{AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_NORMAL}, PIPE_TYPE_IN_NORMAL},
 };
 
diff --git a/services/audio_policy/server/domain/pipe/src/audio_definition_policy_utils.cpp b/services/audio_policy/server/domain/pipe/src/audio_definition_policy_utils.cpp
index f278b171c09..882d5af0cd4 100644
--- a/services/audio_policy/server/domain/pipe/src/audio_definition_policy_utils.cpp
+++ b/services/audio_policy/server/domain/pipe/src/audio_definition_policy_utils.cpp
@@ -185,6 +185,7 @@ std::unordered_map<std::string, AudioFlag> AudioDefinitionPolicyUtils::flagStrTo
     {"AUDIO_OUTPUT_FLAG_INTERPHONE", AUDIO_OUTPUT_FLAG_INTERPHONE},
     {"AUDIO_INPUT_FLAG_ONBOARD_MIC", AUDIO_INPUT_FLAG_ONBOARD_MIC},
     {"AUDIO_INPUT_FLAG_CAMCORDER", AUDIO_INPUT_FLAG_CAMCORDER},
+    {"AUDIO_INPUT_FLAG_VOICE_MESSAGE", AUDIO_INPUT_FLAG_VOICE_MESSAGE},
     {"AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT", AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT},
 };
 
diff --git a/services/audio_policy/server/domain/pipe/src/audio_pipe_manager.cpp b/services/audio_policy/server/domain/pipe/src/audio_pipe_manager.cpp
index 15f27be4bd1..ee6e3850083 100644
--- a/services/audio_policy/server/domain/pipe/src/audio_pipe_manager.cpp
+++ b/services/audio_policy/server/domain/pipe/src/audio_pipe_manager.cpp
@@ -484,7 +484,8 @@ bool AudioPipeManager::IsSpecialPipe(uint32_t routeFlag)
         (routeFlag & AUDIO_OUTPUT_FLAG_INTERPHONE) ||
         (routeFlag & AUDIO_INPUT_FLAG_ONBOARD_MIC) ||
         (routeFlag & AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT) ||
-        (routeFlag & AUDIO_INPUT_FLAG_CAMCORDER)) {
+        (routeFlag & AUDIO_INPUT_FLAG_CAMCORDER) ||
+        (routeFlag & AUDIO_INPUT_FLAG_VOICE_MESSAGE)) {
         AUDIO_INFO_LOG("Flag %{public}d", routeFlag);
         return true;
     }
diff --git a/services/audio_policy/server/domain/pipe/src/audio_pipe_selector.cpp b/services/audio_policy/server/domain/pipe/src/audio_pipe_selector.cpp
index bfea650d57c..a6ecfec965f 100644
--- a/services/audio_policy/server/domain/pipe/src/audio_pipe_selector.cpp
+++ b/services/audio_policy/server/domain/pipe/src/audio_pipe_selector.cpp
@@ -1557,6 +1557,8 @@ AudioPipeType AudioPipeSelector::GetInputPipeType(uint32_t flag)
         return PIPE_TYPE_IN_NORMAL_ONBOARD_MIC;
     } else if (flag & AUDIO_INPUT_FLAG_CAMCORDER) {
         return PIPE_TYPE_IN_CAMCORDER;
+    } else if (flag & AUDIO_INPUT_FLAG_VOICE_MESSAGE) {
+        return PIPE_TYPE_IN_NORMAL_VOICE_MESSAGE;
     }
     return PIPE_TYPE_IN_NORMAL;
 }
diff --git a/services/audio_policy/server/domain/volume/src/audio_adapter_manager.cpp b/services/audio_policy/server/domain/volume/src/audio_adapter_manager.cpp
index 63e297353ac..946ecfa6f9e 100755
--- a/services/audio_policy/server/domain/volume/src/audio_adapter_manager.cpp
+++ b/services/audio_policy/server/domain/volume/src/audio_adapter_manager.cpp
@@ -1629,6 +1629,10 @@ void AudioAdapterManager::GetSourceIdInfoAndIdType(
             idType = HDI_ID_TYPE_PRIMARY;
             idInfo = HDI_ID_INFO_CAMCORDER;
         }
+        if (pipeInfo->routeFlag_ & AUDIO_INPUT_FLAG_VOICE_MESSAGE) {
+            idType = HDI_ID_TYPE_PRIMARY;
+            idInfo = HDI_ID_INFO_VOICE_MESSAGE;
+        }
     } else if (pipeInfo->adapterName_ == "primary_extra_capturer") {
         idType = HDI_ID_TYPE_PRIMARY;
         idInfo = pipeInfo->moduleInfo_.name;
diff --git a/services/audio_policy/server/infra/config/parser/include/audio_concurrency_parser.h b/services/audio_policy/server/infra/config/parser/include/audio_concurrency_parser.h
index 796905720c7..9f00a2e0c96 100644
--- a/services/audio_policy/server/infra/config/parser/include/audio_concurrency_parser.h
+++ b/services/audio_policy/server/infra/config/parser/include/audio_concurrency_parser.h
@@ -77,6 +77,7 @@ private:
         {"primary in voice recognition", PIPE_TYPE_IN_NORMAL_VOICE_RECOGNITION},
         {"primary in raw ai", PIPE_TYPE_IN_NORMAL_RAW_AI},
         {"primary in onboard mic", PIPE_TYPE_IN_NORMAL_ONBOARD_MIC},
+        {"primary in voice message", PIPE_TYPE_IN_NORMAL_VOICE_MESSAGE},
         {"tts out", PIPE_TYPE_OUT_VOICE_CALL_ASSISTANT}
     };
 };
diff --git a/services/audio_policy/server/infra/datashare/src/audio_policy_utils.cpp b/services/audio_policy/server/infra/datashare/src/audio_policy_utils.cpp
index b2c2c8b00b7..1f79635efab 100644
--- a/services/audio_policy/server/infra/datashare/src/audio_policy_utils.cpp
+++ b/services/audio_policy/server/infra/datashare/src/audio_policy_utils.cpp
@@ -76,6 +76,7 @@ std::map<std::string, ClassType> AudioPolicyUtils::portStrToEnum = {
     {PRIMARY_INTERPHONE_MIC, TYPE_PRIMARY},
     {PRIMARY_ONBOARD_MIC, TYPE_PRIMARY},
     {PRIMARY_CAMCORDER, TYPE_PRIMARY},
+    {PRIMARY_VOICE_MESSAGE_MIC, TYPE_PRIMARY},
 };
 
 std::set<DeviceType> AudioPolicyUtils::needDeclaredDevices = {
@@ -95,6 +96,7 @@ static inline const char* ResolvePrimaryMicPort(uint32_t routeFlag)
         case AUDIO_INPUT_FLAG_RAW_AI:           return PRIMARY_RAW_AI_MIC;
         case AUDIO_INPUT_FLAG_ONBOARD_MIC:      return PRIMARY_ONBOARD_MIC;
         case AUDIO_INPUT_FLAG_CAMCORDER:        return PRIMARY_CAMCORDER;
+        case AUDIO_INPUT_FLAG_VOICE_MESSAGE:    return PRIMARY_VOICE_MESSAGE_MIC;
         case AUDIO_INPUT_FLAG_FAST:             return FAST_CAPTURER_SOURCE;
         case AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP: return FAST_VOIP_CAPTURER_SOURCE;
         case AUDIO_INPUT_FLAG_INTERPHONE:       return PRIMARY_INTERPHONE_MIC;
@@ -571,6 +573,7 @@ std::string AudioPolicyUtils::GetInputDeviceClassBySourcePortName(std::string so
         {PRIMARY_RAW_AI_MIC, PRIMARY_CLASS},
         {PRIMARY_ONBOARD_MIC, PRIMARY_CLASS},
         {PRIMARY_CAMCORDER, PRIMARY_CLASS},
+        {PRIMARY_VOICE_MESSAGE_MIC, PRIMARY_CLASS},
     };
     std::string deviceClass = INVALID_CLASS;
     if (sourcePortStrToClassStrMap_.count(sourcePortName) > 0) {
@@ -842,7 +845,7 @@ DeviceType AudioPolicyUtils::GetDeviceType(const std::string &deviceName)
         deviceName == PRIMARY_UNPROCESS_MIC || deviceName == PRIMARY_ULTRASONIC_MIC ||
         deviceName == PRIMARY_VOICE_RECOGNITION_MIC || deviceName == PRIMARY_RAW_AI_MIC ||
         deviceName == PRIMARY_INTERPHONE_MIC || deviceName == PRIMARY_LIVE_MIC || deviceName == PRIMARY_ONBOARD_MIC ||
-        deviceName == PRIMARY_CAMCORDER) {
+        deviceName == PRIMARY_CAMCORDER || deviceName == PRIMARY_VOICE_MESSAGE_MIC) {
         devType = DeviceType::DEVICE_TYPE_MIC;
     } else if (deviceName == "Built_in_wakeup") {
         devType = DeviceType::DEVICE_TYPE_WAKEUP;
diff --git a/services/audio_policy/server/service/service_main/src/audio_core_service.cpp b/services/audio_policy/server/service/service_main/src/audio_core_service.cpp
index 99e9e1895f2..f4062bdcae6 100644
--- a/services/audio_policy/server/service/service_main/src/audio_core_service.cpp
+++ b/services/audio_policy/server/service/service_main/src/audio_core_service.cpp
@@ -78,6 +78,8 @@ static const std::string PIPE_PRIMARY_INPUT_VOICE_RECOGNITION = "primary_input_v
 static constexpr uint32_t ONLY_SUPPORT_MMAP = 2;
 static constexpr uint32_t AUDIO_INPUT_ONBOARD_MIC_TYPE = 1 << 17;
 AudioInputType hdiAudioOnBoardMicInputType = static_cast<AudioInputType>(AUDIO_INPUT_ONBOARD_MIC_TYPE);
+static constexpr uint32_t AUDIO_INPUT_VOICE_MESSAGE_TYPE = 1 << 18;
+AudioInputType hdiAudioVoiceMessageInputType = static_cast<AudioInputType>(AUDIO_INPUT_VOICE_MESSAGE_TYPE);
 static const std::string PIPE_PRIMARY_INPUT_CAMCORDER = "primary_input_camcorder";
 static const std::unordered_set<SourceType> VOIP_PRIVACY_MUTE_TARGET_SOURCES = {
     SOURCE_TYPE_LIVE,
@@ -187,6 +189,7 @@ const std::map<SourceType, AudioInputType> FWKTYPE_TO_HDITYPE_MAP = {
     { SOURCE_TYPE_LIVE, AUDIO_INPUT_LIVE_TYPE },
     { SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT, AUDIO_INPUT_RAW_AI_TYPE},
     { SOURCE_TYPE_ONBOARD_MIC, hdiAudioOnBoardMicInputType},
+    { SOURCE_TYPE_VOICE_MESSAGE, hdiAudioVoiceMessageInputType},
 };
 
 static const std::unordered_set<SourceType> specialSourceTypeSet_ = {
@@ -4132,6 +4135,9 @@ void AudioCoreService::GetTargetSourceTypeAndMatchingFlag(SourceType source,
         case SOURCE_TYPE_LIVE:
             targetSource = SOURCE_TYPE_LIVE;
             break;
+        case SOURCE_TYPE_VOICE_MESSAGE:
+            targetSource = SOURCE_TYPE_VOICE_MESSAGE;
+            break;
         default:
             targetSource = SOURCE_TYPE_MIC;
             break;
diff --git a/services/audio_policy/server/service/service_main/src/audio_core_service_private.cpp b/services/audio_policy/server/service/service_main/src/audio_core_service_private.cpp
index fc860da9dcd..c96ade6380c 100644
--- a/services/audio_policy/server/service/service_main/src/audio_core_service_private.cpp
+++ b/services/audio_policy/server/service/service_main/src/audio_core_service_private.cpp
@@ -133,7 +133,8 @@ static const std::vector<std::string> SourceNames = {
     std::string(PRIMARY_RAW_AI_MIC),
     std::string(PRIMARY_INTERPHONE_MIC),
     std::string(PRIMARY_ONBOARD_MIC),
-    std::string(PRIMARY_CAMCORDER)
+    std::string(PRIMARY_CAMCORDER),
+    std::string(PRIMARY_VOICE_MESSAGE_MIC)
 };
 
 std::string AudioCoreService::GetEncryptAddr(const std::string &addr)
@@ -2256,7 +2257,8 @@ uint32_t AudioCoreService::OpenNewAudioPortAndRoute(std::shared_ptr<AudioPipeInf
             (pipeInfo->routeFlag_ != AUDIO_INPUT_FLAG_VOICE_RECOGNITION) &&
             (pipeInfo->routeFlag_ != AUDIO_INPUT_FLAG_RAW_AI) &&
             (pipeInfo->routeFlag_ != AUDIO_INPUT_FLAG_INTERPHONE) &&
-            (pipeInfo->routeFlag_ != AUDIO_INPUT_FLAG_ONBOARD_MIC))) {
+            (pipeInfo->routeFlag_ != AUDIO_INPUT_FLAG_ONBOARD_MIC) &&
+            (pipeInfo->routeFlag_ != AUDIO_INPUT_FLAG_VOICE_MESSAGE))) {
             audioPolicyManager_.SetDeviceActive(audioActiveDevice_.GetCurrentInputDeviceType(GetRealUid(streamDesc)),
                 pipeInfo->moduleInfo_.name, true, INPUT_DEVICES_FLAG);
         }
diff --git a/services/audio_service/server/src/audio_server.cpp b/services/audio_service/server/src/audio_server.cpp
index a95377ceb48..677443c8493 100644
--- a/services/audio_service/server/src/audio_server.cpp
+++ b/services/audio_service/server/src/audio_server.cpp
@@ -1274,7 +1274,13 @@ int32_t AudioServer::SetAudioParameter(const std::string &key, const std::string
     CHECK_AND_RETURN_RET_LOG(AudioParameterValueCheck(key, value), ERR_INVALID_PARAM,
         "AudioParameterValueCheck fail! key: %{public}s, value: %{public}s", key.c_str(), value.c_str());
 
-    if (key != "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG") {
+    if (key == "record_denoise_mode") {
+        AudioServer::audioParameters[key] = value;
+        std::string halName = "primary";
+        HdiAdapterManager::GetInstance().SetAudioParameter(HDI_DEVICE_MANAGER_TYPE_LOCAL, halName,
+            AudioParamKey::NONE, "", "record_denoise_mode=" + value);
+        return SUCCESS;
+    } else if (key != "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG") {
         bool ret = VerifyClientPermission(MODIFY_AUDIO_SETTINGS_PERMISSION);
         CHECK_AND_RETURN_RET_LOG(ret, ERR_PERMISSION_DENIED, "MODIFY_AUDIO_SETTINGS permission denied");
     } else {
@@ -1523,6 +1529,15 @@ const std::string AudioServer::GetAudioParameterInner(const std::string &key)
     AudioXCollie audioXCollie("GetAudioParameter", TIME_OUT_SECONDS,
          nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
 
+    if (key == "record_denoise_mode_support") {
+        auto result = HdiAdapterManager::GetInstance().GetAudioParameterByKey(
+            HDI_DEVICE_MANAGER_TYPE_LOCAL, "record_denoise_mode_support");
+        if (result.has_value() && !result.value().empty()) {
+            return result.value();
+        }
+        return "";
+    }
+    
     auto result = HdiAdapterManager::GetInstance().GetAudioParameterByKey(HDI_DEVICE_MANAGER_TYPE_LOCAL, key);
     if (result.has_value()) {
         return result.value();
-- 
2.51.2.huawei.11


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
#ifndef LOG_TAG
#define LOG_TAG "NapiAudioCapturer"
#endif

#include "napi_audio_capturer.h"
#include <new>
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "xpower_event_js.h"
#endif
#endif
#include "audio_errors.h"
#include "audio_system_manager.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "napi_audio_enum.h"
#include "napi_dfx_utils.h"
#include "napi_audio_capturer_callbacks.h"
#include "napi_capturer_position_callback.h"
#include "napi_capturer_period_position_callback.h"
#include "napi_audio_capturer_read_data_callback.h"
#include "napi_audio_capturer_device_change_callback.h"
#include "napi_audio_capturer_info_change_callback.h"

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_capturerConstructor = nullptr;
std::unique_ptr<AudioCapturerOptions> NapiAudioCapturer::sCapturerOptions_ = nullptr;
mutex NapiAudioCapturer::createMutex_;
int32_t NapiAudioCapturer::isConstructSuccess_ = SUCCESS;
std::atomic<bool> NapiAudioCapturer::firstReadCalled_{false};
std::atomic<bool> NapiAudioCapturer::firstRegReadCbCalled_{false};

namespace {
class NapiPlaybackCaptureStartStateCallback : public AudioCapturerOnPlaybackCaptureStartCallback {
public:
    void OnPlaybackCaptureStartResult(PlaybackCaptureStartState state) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = state;
        hasResult_ = true;
    }

    PlaybackCaptureStartState GetState()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }

    bool HasResult()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return hasResult_;
    }

private:
    std::mutex mutex_;
    PlaybackCaptureStartState state_ = START_STATE_FAILED;
    bool hasResult_ = false;
};

struct PlaybackCaptureStartAsyncContext {
    napi_ref callbackRef = nullptr;
    napi_ref selfRef = nullptr;
    napi_async_work work = nullptr;
    NapiAudioCapturer *native = nullptr;
    PlaybackCaptureStartState playbackCaptureStartState = START_STATE_FAILED;
};

void ReleasePlaybackCaptureStartAsyncContext(napi_env env, PlaybackCaptureStartAsyncContext *context)
{
    if (context == nullptr) {
        return;
    }
    if (context->callbackRef != nullptr) {
        napi_delete_reference(env, context->callbackRef);
    }
    if (context->selfRef != nullptr) {
        napi_delete_reference(env, context->selfRef);
    }
    if (context->work != nullptr) {
        napi_delete_async_work(env, context->work);
    }
    delete context;
}

PlaybackCaptureStartState HandlePlaybackCaptureStartResult(AudioCapturer *audioCapturer,
    PlaybackCaptureStartState state)
{
    if (state == START_STATE_SUCCESS) {
        if (audioCapturer != nullptr && !audioCapturer->Start()) {
            AUDIO_ERR_LOG("Start playback capture stream failed after authority granted");
            return START_STATE_FAILED;
        }
        return START_STATE_SUCCESS;
    }
    if (state == START_STATE_NOT_AUTHORIZED) {
        AUDIO_ERR_LOG("Playback capture start failed: not authorized");
        return START_STATE_NOT_AUTHORIZED;
    }
    AUDIO_ERR_LOG("Playback capture start failed: unknown error");
    return START_STATE_FAILED;
}

void ExecutePlaybackCaptureStart(PlaybackCaptureStartAsyncContext *context)
{
    CHECK_AND_RETURN_LOG(context != nullptr, "context is nullptr");
    ObjectRefMap objectGuard(context->native);
    auto *napiAudioCapturer = objectGuard.GetPtr();
    CHECK_AND_RETURN_LOG(napiAudioCapturer != nullptr, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_LOG(napiAudioCapturer->audioCapturer_ != nullptr, "audioCapturer_ is nullptr");

    auto playbackCaptureStartCallback = std::make_shared<NapiPlaybackCaptureStartStateCallback>();
    if (playbackCaptureStartCallback == nullptr) {
        context->playbackCaptureStartState = START_STATE_FAILED;
        return;
    }

    napiAudioCapturer->audioCapturer_->SetPlaybackCaptureStartStateCallback(playbackCaptureStartCallback);
    int32_t ret = napiAudioCapturer->audioCapturer_->StartPlaybackCapture();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("StartPlaybackCapture failed, ret=%{public}d", ret);
        napiAudioCapturer->audioCapturer_->SetPlaybackCaptureStartStateCallback(nullptr);
        context->playbackCaptureStartState = START_STATE_FAILED;
        return;
    }

    if (!playbackCaptureStartCallback->HasResult()) {
        AUDIO_ERR_LOG("Playback capture start result missing");
        context->playbackCaptureStartState = START_STATE_FAILED;
    } else {
        context->playbackCaptureStartState = HandlePlaybackCaptureStartResult(
            napiAudioCapturer->audioCapturer_.get(), playbackCaptureStartCallback->GetState());
    }
    napiAudioCapturer->audioCapturer_->SetPlaybackCaptureStartStateCallback(nullptr);
}

void CallPlaybackCaptureStartJsCallback(napi_env env, PlaybackCaptureStartAsyncContext *context)
{
    CHECK_AND_RETURN_LOG(context != nullptr, "context is nullptr");
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");

    napi_value callback = nullptr;
    napi_status status = napi_get_reference_value(env, context->callbackRef, &callback);
    if (status != napi_ok || callback == nullptr) {
        AUDIO_ERR_LOG("get callback reference failed");
    } else {
        napi_value args[ARGS_ONE] = { nullptr };
        status = NapiParamUtils::SetValueInt32(
            env, static_cast<int32_t>(context->playbackCaptureStartState), args[PARAM0]);
        if (status != napi_ok || args[PARAM0] == nullptr) {
            AUDIO_ERR_LOG("create requestPlaybackCaptureStart callback arg failed");
        } else {
            napi_value callbackResult = nullptr;
            status = napi_call_function(env, nullptr, callback, ARGS_ONE, args, &callbackResult);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("requestPlaybackCaptureStart callback failed");
            }
        }
    }
    napi_close_handle_scope(env, scope);
}

int32_t CreatePlaybackCaptureStartContext(napi_env env, napi_value self, napi_value callback,
    NapiAudioCapturer *napiAudioCapturer, PlaybackCaptureStartAsyncContext **context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, NAPI_ERR_SYSTEM, "context is nullptr");
    auto *asyncContext = new (std::nothrow) PlaybackCaptureStartAsyncContext();
    CHECK_AND_RETURN_RET_LOG(asyncContext != nullptr, NAPI_ERR_NO_MEMORY,
        "RequestPlaybackCaptureStart failed : no memory");
    asyncContext->native = napiAudioCapturer;

    napi_status status = napi_create_reference(env, callback, REFERENCE_CREATION_COUNT, &asyncContext->callbackRef);
    if (status != napi_ok || asyncContext->callbackRef == nullptr) {
        delete asyncContext;
        return NAPI_ERR_SYSTEM;
    }
    status = napi_create_reference(env, self, REFERENCE_CREATION_COUNT, &asyncContext->selfRef);
    if (status != napi_ok || asyncContext->selfRef == nullptr) {
        ReleasePlaybackCaptureStartAsyncContext(env, asyncContext);
        return NAPI_ERR_SYSTEM;
    }

    *context = asyncContext;
    return SUCCESS;
}

int32_t EnqueuePlaybackCaptureStartWork(napi_env env, PlaybackCaptureStartAsyncContext *context)
{
    napi_value resource = nullptr;
    napi_status status = napi_create_string_utf8(env, "RequestPlaybackCaptureStart", NAPI_AUTO_LENGTH, &resource);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NAPI_ERR_SYSTEM, "create async resource failed");
    status = napi_create_async_work(env, nullptr, resource,
        [](napi_env, void *data) {
            ExecutePlaybackCaptureStart(reinterpret_cast<PlaybackCaptureStartAsyncContext *>(data));
        },
        [](napi_env env, napi_status, void *data) {
            auto *ctx = reinterpret_cast<PlaybackCaptureStartAsyncContext *>(data);
            CallPlaybackCaptureStartJsCallback(env, ctx);
            ReleasePlaybackCaptureStartAsyncContext(env, ctx);
        },
        reinterpret_cast<void *>(context), &context->work);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NAPI_ERR_SYSTEM, "create async work failed");
    status = napi_queue_async_work_with_qos(env, context->work, napi_qos_user_initiated);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NAPI_ERR_SYSTEM, "queue async work failed");
    return SUCCESS;
}
} // namespace

NapiAudioCapturer::NapiAudioCapturer()
    : audioCapturer_(nullptr),  sourceType_(SOURCE_TYPE_MIC), env_(nullptr) {}

NapiAudioCapturer::~NapiAudioCapturer()
{
    if (audioCapturer_ != nullptr) {
        bool ret = audioCapturer_->Release();
        CHECK_AND_RETURN_LOG(ret, "AudioCapturer release fail");
        audioCapturer_ = nullptr;
        AUDIO_INFO_LOG("Proactively release audioCapturer");
    }
}

void NapiAudioCapturer::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject == nullptr) {
        AUDIO_WARNING_LOG("Native object is null");
        return;
    }
    auto obj = static_cast<NapiAudioCapturer *>(nativeObject);
    ObjectRefMap<NapiAudioCapturer>::DecreaseRef(obj);
    AUDIO_INFO_LOG("Decrease obj count");
}

napi_status NapiAudioCapturer::InitAudioCapturer(napi_env env, napi_value &constructor)
{
    AUDIO_DEBUG_LOG("NapiAudioCapturer::InitAudioCapturer()");
    napi_property_descriptor audio_capturer_properties[] = {
        DECLARE_NAPI_FUNCTION("getCapturerInfo", GetCapturerInfo),
        DECLARE_NAPI_FUNCTION("getCapturerInfoSync", GetCapturerInfoSync),
        DECLARE_NAPI_FUNCTION("getStreamInfo", GetStreamInfo),
        DECLARE_NAPI_FUNCTION("getStreamInfoSync", GetStreamInfoSync),
        DECLARE_NAPI_FUNCTION("start", Start),
        DECLARE_NAPI_FUNCTION("read", Read),
        DECLARE_NAPI_FUNCTION("getAudioTime", GetAudioTime),
        DECLARE_NAPI_FUNCTION("getAudioTimeSync", GetAudioTimeSync),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("getBufferSize", GetBufferSize),
        DECLARE_NAPI_FUNCTION("getBufferSizeSync", GetBufferSizeSync),
        DECLARE_NAPI_FUNCTION("getAudioStreamId", GetAudioStreamId),
        DECLARE_NAPI_FUNCTION("getAudioStreamIdSync", GetAudioStreamIdSync),
        DECLARE_NAPI_FUNCTION("getCurrentInputDevices", GetCurrentInputDevices),
        DECLARE_NAPI_FUNCTION("getCurrentAudioCapturerChangeInfo", GetCurrentAudioCapturerChangeInfo),
        DECLARE_NAPI_FUNCTION("getCurrentMicrophones", GetCurrentMicrophones),
        DECLARE_NAPI_FUNCTION("getOverflowCount", GetOverflowCount),
        DECLARE_NAPI_FUNCTION("setInputDeviceToAccessory", SetInputDeviceToAccessory),
        DECLARE_NAPI_FUNCTION("getOverflowCountSync", GetOverflowCountSync),
        DECLARE_NAPI_FUNCTION("getAudioTimestampInfo", GetAudioTimestampInfo),
        DECLARE_NAPI_FUNCTION("getAudioTimestampInfoSync", GetAudioTimestampInfoSync),
        DECLARE_NAPI_FUNCTION("setWillMuteWhenInterrupted", SetWillMuteWhenInterrupted),
        DECLARE_NAPI_FUNCTION("setMuteHint", SetMuteHint),
        DECLARE_NAPI_FUNCTION("setNoiseReductionMode", SetNoiseReductionMode),
        DECLARE_NAPI_FUNCTION("getNoiseReductionMode", GetNoiseReductionMode),
        DECLARE_NAPI_FUNCTION("getSupportedNoiseReductionModes", GetSupportedNoiseReductionModes),
        DECLARE_NAPI_FUNCTION("requestPlaybackCaptureStart", RequestPlaybackCaptureStart),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("onReadMicInData", OnReadMicInData),
        DECLARE_NAPI_FUNCTION("offReadMicInData", OffReadMicInData),
        DECLARE_NAPI_FUNCTION("setIndependentAudioSessionStrategy", SetIndependentAudioSessionStrategy),
        DECLARE_NAPI_GETTER("state", GetState),
    };

    napi_status status = napi_define_class(env, NAPI_AUDIO_CAPTURER_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr, sizeof(audio_capturer_properties) / sizeof(audio_capturer_properties[PARAM0]),
        audio_capturer_properties, &constructor);
    return status;
}

napi_value NapiAudioCapturer::Init(napi_env env, napi_value exports)
{
    napi_value constructor;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createAudioCapturer", CreateAudioCapturer),
        DECLARE_NAPI_STATIC_FUNCTION("createAudioCapturerSync", CreateAudioCapturerSync),
        DECLARE_NAPI_STATIC_FUNCTION("createMicInAudioCapturer", CreateMicInAudioCapturer)
    };

    napi_status status = InitAudioCapturer(env, constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_class fail");
    status = napi_create_reference(env, constructor, REFERENCE_CREATION_COUNT, &g_capturerConstructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
    status = napi_set_named_property(env, exports, NAPI_AUDIO_CAPTURER_CLASS_NAME.c_str(), constructor);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
    status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "Failure in NapiAudioCapturer::Init()");

    return exports;
}

unique_ptr<NapiAudioCapturer> NapiAudioCapturer::CreateAudioCapturerNativeObject(napi_env env)
{
    unique_ptr<NapiAudioCapturer> napiCapturer = make_unique<NapiAudioCapturer>();
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, nullptr, "No memory");

    napiCapturer->env_ = env;
    napiCapturer->sourceType_ = sCapturerOptions_->capturerInfo.sourceType;

    AudioCapturerOptions capturerOptions = *sCapturerOptions_;
    napiCapturer->isModernInnerCapturer_ = capturerOptions.playbackCaptureConfig.isModernInnerCapturer;

    /* NapiAudioCapturer not support other capturerFlags, only support flag 0 */
    if (capturerOptions.capturerInfo.capturerFlags != 0) {
        capturerOptions.capturerInfo.capturerFlags = 0;
    }
    capturerOptions.capturerInfo.recorderType = RECORDER_TYPE_ARKTS_AUDIO_RECORDER;
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    napiCapturer->audioCapturer_ = AudioCapturer::CreateCapturer(capturerOptions);
#else
    std::string cacheDir = "/data/storage/el2/base/temp";
    napiCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions, cacheDir);
#endif
    if (napiCapturer->audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("Capturer Create failed");
        NapiAudioCapturer::isConstructSuccess_ = NAPI_ERR_SYSTEM;
        napiCapturer.release();
        return nullptr;
    }
    ObjectRefMap<NapiAudioCapturer>::Insert(napiCapturer.get());

    if (napiCapturer->audioCapturer_ != nullptr && napiCapturer->callbackNapi_ == nullptr) {
        napiCapturer->callbackNapi_ = std::make_shared<NapiAudioCapturerCallback>(env);
        CHECK_AND_RETURN_RET_LOG(napiCapturer->callbackNapi_ != nullptr, napiCapturer, "No memory");
        int32_t ret = napiCapturer->audioCapturer_->SetCapturerCallback(napiCapturer->callbackNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, napiCapturer, "Construct SetCapturerCallback failed");
    }
    return napiCapturer;
}

napi_value NapiAudioCapturer::GetCallback(size_t argc, napi_value *argv)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    return callback;
}

template <typename T>
static void GetCapturerNapiCallback(napi_value callback, const std::string &cbName,
    std::list<std::shared_ptr<NapiAudioCapturerCallbackInner>> audioCapturerCallbacks, std::shared_ptr<T> *cb)
{
    if (audioCapturerCallbacks.size() == 0) {
        AUDIO_ERR_LOG("no callback to get");
        return;
    }
    for (auto &iter:audioCapturerCallbacks) {
        if (!iter->CheckIfTargetCallbackName(cbName)) {
            continue;
        }
        std::shared_ptr<T> temp = std::static_pointer_cast<T>(iter);
        if (temp->ContainSameJsCallbackInner(cbName, callback)) {
            *cb = temp;
            return;
        }
    }
}

template <typename T>
static void UnregisterAudioCapturerSingletonCallbackTemplate(napi_env env, napi_value callback,
    const std::string &cbName, std::shared_ptr<T> cb,
    std::function<int32_t(std::shared_ptr<T> callbackPtr, napi_value callback)> removeFunction = nullptr)
{
    if (callback != nullptr) {
        CHECK_AND_RETURN_LOG(cb->ContainSameJsCallbackInner(cbName, callback), "napi_get_cb_info failed");
    }

    cb->RemoveCallbackReference(cbName, env, callback);

    if (removeFunction == nullptr) {
        return;
    }
    int32_t ret = removeFunction(cb, callback);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Unset of Capturer callback failed");
    return;
}

template<typename T>
static void UnregisterAudioCapturerCallbackTemplate(napi_env env, napi_value callback,
    NapiAudioCapturer *napiCapturer, const std::string &cbName,
    std::function<int32_t(std::shared_ptr<T> callbackPtr, napi_value callback)> removeFunction)
{
    if (callback != nullptr) {
        std::shared_ptr<T> cb = nullptr;
        GetCapturerNapiCallback(callback, cbName, napiCapturer->audioCapturerCallbacks_, &cb);
        CHECK_AND_RETURN_LOG(cb != nullptr, "CapturerNapiCallback is null");
        UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
        return;
    }

    auto isPresent = [&env, &callback, &cbName, &removeFunction]
        (std::shared_ptr<NapiAudioCapturerCallbackInner> &iter) {
            if (!iter->CheckIfTargetCallbackName(cbName)) {
                return false;
            }
            std::shared_ptr<T> cb = std::static_pointer_cast<T>(iter);
            UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
            return true;
        };
    napiCapturer->audioCapturerCallbacks_.remove_if(isPresent);
    AUDIO_DEBUG_LOG("UnregisterAudioCapturerCallback success");
}

napi_value NapiAudioCapturer::Construct(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argCount = ARGS_TWO;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "failed to napi_get_cb_info");

    unique_ptr<NapiAudioCapturer> napiCapturer = CreateAudioCapturerNativeObject(env);
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, result, "failed to CreateAudioCapturerNativeObject");
    status = napi_wrap(env, thisVar, static_cast<void*>(napiCapturer.get()),
        NapiAudioCapturer::Destructor, nullptr, nullptr);
    if (status != napi_ok) {
        ObjectRefMap<NapiAudioCapturer>::Erase(napiCapturer.get());
        return result;
    }

    napiCapturer.release();
    return thisVar;
}

bool NapiAudioCapturer::CheckContextStatus(std::shared_ptr<AudioCapturerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "CHECK_AND_RETURN_RET_LOG");
    if (context->native == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

bool NapiAudioCapturer::CheckAudioCapturerStatus(NapiAudioCapturer *napi,
    std::shared_ptr<AudioCapturerAsyncContext> context)
{
    CHECK_AND_RETURN_RET_LOG(napi != nullptr, false, "CHECK_AND_RETURN_RET_LOG");
    if (napi->audioCapturer_ == nullptr) {
        context->SignError(NAPI_ERR_SYSTEM);
        return false;
    }
    return true;
}

NapiAudioCapturer* NapiAudioCapturer::GetParamWithSync(const napi_env &env, napi_callback_info info,
    size_t &argc, napi_value *args)
{
    napi_status status;
    NapiAudioCapturer *napiAudioCapturer = nullptr;
    napi_value jsThis = nullptr;

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
        "GetParamWithSync fail to napi_get_cb_info");

    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiAudioCapturer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG((napiAudioCapturer != nullptr) && (napiAudioCapturer->audioCapturer_ != nullptr),
        napiAudioCapturer, "GetParamWithSync fail to napi_unwrap");

    return napiAudioCapturer;
}

napi_value NapiAudioCapturer::CreateAudioCapturerWrapper(napi_env env, const AudioCapturerOptions capturerOptions)
{
    lock_guard<mutex> lock(createMutex_);
    napi_value result = nullptr;
    napi_value constructor = nullptr;

    napi_status status = napi_get_reference_value(env, g_capturerConstructor, &constructor);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_get_reference_value failed");
        goto fail;
    }
    if (sCapturerOptions_ != nullptr) {
        sCapturerOptions_.release();
    }
    sCapturerOptions_ = make_unique<AudioCapturerOptions>();
    CHECK_AND_RETURN_RET_LOG(sCapturerOptions_ != nullptr, result, "sCapturerOptions create failed");
    *sCapturerOptions_ = capturerOptions;
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("napi_new_instance failed");
        goto fail;
    }
    return result;

fail:
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioCapturer::CreateAudioCapturer(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("CreateAudioCapturer failed : no memory");
        NapiAudioError::ThrowError(env, "CreateAudioCapturer failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetCapturerOptions(env, &context->capturerOptions, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "CreateAudioCapturer failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto complete = [env, context](napi_value &output) {
        output = CreateAudioCapturerWrapper(env, context->capturerOptions);
        if (NapiAudioCapturer::isConstructSuccess_ != SUCCESS) {
            context->SignError(NapiAudioCapturer::isConstructSuccess_);
            NapiAudioCapturer::isConstructSuccess_ = SUCCESS;
        }
    };

    return NapiAsyncWork::Enqueue(env, context, "CreateAudioCapturer", nullptr, complete);
}


napi_value NapiAudioCapturer::CreateMicInAudioCapturer(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("CreateMicInAudioCapturer failed : no memory");
        NapiAudioError::ThrowError(env, "CreateMicInAudioCapturer failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetCapturerOptions(env, &context->capturerOptions, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "CreateMicInAudioCapturer failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto complete = [env, context](napi_value &output) {
        output = CreateAudioCapturerWrapper(env, context->capturerOptions);
        if (NapiAudioCapturer::isConstructSuccess_ != SUCCESS) {
            context->SignError(NapiAudioCapturer::isConstructSuccess_);
            NapiAudioCapturer::isConstructSuccess_ = SUCCESS;
        }
    };

    return NapiAsyncWork::Enqueue(env, context, "CreateMicInAudioCapturer", nullptr, complete);
}

napi_value NapiAudioCapturer::CreateAudioCapturerSync(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = {};
    napi_status status = NapiParamUtils::GetParam(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG((status == napi_ok) && (argc == ARGS_ONE),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "mandatory parameters are left unspecified"), "invaild param");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_object, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of options must be number"),
        "invaild valueType");

    AudioCapturerOptions capturerOptions;
    CHECK_AND_RETURN_RET_LOG(NapiParamUtils::GetCapturerOptions(env, &capturerOptions, argv[PARAM0]) == napi_ok,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of options must be interface AudioCapturerOptions"),
        "get captureOptions failed");

    return NapiAudioCapturer::CreateAudioCapturerWrapper(env, capturerOptions);
}

napi_value NapiAudioCapturer::GetCapturerInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetCapturerInfo(context->capturerInfo);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetCapturerInfo(env, context->capturerInfo, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetCapturerInfo", executor, complete);
}

napi_value NapiAudioCapturer::GetCapturerInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID), "argsCount invaild");

    AudioCapturerInfo capturerInfo;
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");
    int32_t ret = napiAudioCapturer->audioCapturer_->GetCapturerInfo(capturerInfo);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetCapturerInfoSync failure!");
    NapiParamUtils::SetCapturerInfo(env, capturerInfo, result);

    return result;
}

napi_value NapiAudioCapturer::GetStreamInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetStreamInfo(context->streamInfo);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetStreamInfo(env, context->streamInfo, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetStreamInfo", executor, complete);
}

napi_value NapiAudioCapturer::GetStreamInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    AudioStreamInfo streamInfo;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetStreamInfo(streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetStreamInfo failure!");

    NapiParamUtils::SetStreamInfo(env, streamInfo, result);
    return result;
}

napi_value NapiAudioCapturer::Start(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        // Playback capture created by modern inner capture api forbidden to start.
        // It should start by RequestPlaybackCaptureStart.
        if (napiAudioCapturer->sourceType_ == SOURCE_TYPE_PLAYBACK_CAPTURE &&
            napiAudioCapturer->isModernInnerCapturer_) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
            return;
        }
        context->intValue = napiAudioCapturer->audioCapturer_->StartWithError();
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM, AudioStandard::ErrMessage(context->intValue));
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
#ifdef FEATURE_HIVIEW_ENABLE
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "STREAM_CHANGE", "SRC=Audio");
#endif
#endif
    return NapiAsyncWork::Enqueue(env, context, "Start", executor, complete);
}

napi_status NapiAudioCapturer::ReadFromNative(shared_ptr<AudioCapturerAsyncContext> context)
{
    napi_status status = napi_generic_failure;
    CHECK_AND_RETURN_RET_LOG(CheckContextStatus(context), napi_generic_failure, "context object state is error.");
    auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
    ObjectRefMap objectGuard(obj);
    auto *napiAudioCapturer = objectGuard.GetPtr();
    CHECK_AND_RETURN_RET_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context), napi_generic_failure,
        "context object state is error.");
    uint32_t userSize = context->userSize;
    uint8_t *buffer = new uint8_t[userSize];
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, status, "buffer malloc failed,no memery");
    int32_t bytesRead = 0;
    while (static_cast<uint32_t>(bytesRead) < context->userSize) {
        int32_t len = napiAudioCapturer->audioCapturer_->Read(*(buffer + bytesRead),
            userSize - bytesRead, true);
        if (len >= 0) {
            bytesRead += len;
        } else {
            bytesRead = len;
            break;
        }
    }
    if (bytesRead <= 0) {
        delete [] buffer;
        return status;
    }
    context->bytesRead = static_cast<size_t>(bytesRead);
    context->buffer = buffer;
    status = napi_ok;
    return status;
}

napi_value NapiAudioCapturer::SetInputDeviceToAccessory(napi_env env, napi_callback_info info)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySelfPermission(),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_PERMISSION_DENIED), "No system permission");
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, "SetInputDeviceToAccessory failed : no memory",
            NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->SetInputDevice(DEVICE_TYPE_ACCESSORY);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetInputDeviceToAccessory", executor, complete);
}

napi_value NapiAudioCapturer::Read(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

#ifndef IOS_PLATFORM
    if ((!firstReadCalled_.exchange(true, std::memory_order_relaxed)) &&
        (getpid() == gettid())) {
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        if (CheckAudioCapturerStatus(napiAudioCapturer, context) == true) {
            AudioCapturerInfo capturerInfo = {};
            napiAudioCapturer->audioCapturer_->GetCapturerInfo(capturerInfo);
            napiAudioCapturer->audioCapturer_->SetInMainThreadState(true);
            int32_t appUid = static_cast<int32_t>(getuid());
            NapiDfxUtils::ReportAudioMainThreadEvent(appUid, NapiDfxUtils::SteamDirection::capture,
                capturerInfo.sourceType, NapiDfxUtils::MainThreadCallFunc::read);
        }
    }
#endif

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueUInt32(env, context->userSize, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "GetValueUInt32 userSize failed",
            NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueBoolean(env, context->isBlocking, argv[PARAM1]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "GetValueUInt32 userSize failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        context->status = ReadFromNative(context);
        if (context->status != napi_ok) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::CreateArrayBuffer(env, context->bytesRead, context->buffer, output);
        delete [] context->buffer;
        context->buffer = nullptr;
    };

    return NapiAsyncWork::Enqueue(env, context, "Read", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioTime(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        Timestamp timestamp;
        if (napiAudioCapturer->audioCapturer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC)) {
            const uint64_t secToNanosecond = 1000000000;
            context->time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
                static_cast<uint64_t>(timestamp.time.tv_sec) * secToNanosecond;
        } else {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueInt64(env, static_cast<int64_t>(context->time), output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetAudioTime", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioTimeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    Timestamp timestamp;
    bool ret = napiAudioCapturer->audioCapturer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC);
    CHECK_AND_RETURN_RET_LOG(ret, result, "GetAudioTime failure!");
    const uint64_t secToNanosecond = 1000000000;
    uint64_t time = static_cast<uint64_t>(timestamp.time.tv_nsec) +
        static_cast<uint64_t>(timestamp.time.tv_sec) * secToNanosecond;

    NapiParamUtils::SetValueInt64(env, static_cast<int64_t>(time), result);
    return result;
}

napi_value NapiAudioCapturer::Stop(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->StopWithError();
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM, AudioStandard::ErrMessage(context->intValue));
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };

    return NapiAsyncWork::Enqueue(env, context, "Stop", executor, complete);
}

napi_value NapiAudioCapturer::Release(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->ReleaseWithError();
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM, AudioStandard::ErrMessage(context->intValue));
        }
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };

    return NapiAsyncWork::Enqueue(env, context, "Release", executor, complete);
}

napi_value NapiAudioCapturer::GetBufferSize(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetBufferSize(context->bufferSize);
        if (context->intValue != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->bufferSize, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetBufferSize", executor, complete);
}

napi_value NapiAudioCapturer::GetBufferSizeSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    size_t bufferSize = 0;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetBufferSize(bufferSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetBufferSize failure!");

    NapiParamUtils::SetValueUInt32(env, bufferSize, result);
    return result;
}

napi_value NapiAudioCapturer::GetAudioStreamId(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->intValue = napiAudioCapturer->audioCapturer_->GetAudioStreamId(context->audioStreamId);
        if (context->intValue  == ERR_ILLEGAL_STATE) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
        } else if (context->intValue  == ERR_INVALID_INDEX) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->audioStreamId, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetAudioStreamId", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioStreamIdSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    uint32_t audioStreamId;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetAudioStreamId(audioStreamId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetAudioStreamId failure!");

    NapiParamUtils::SetValueUInt32(env, audioStreamId, result);
    return result;
}

napi_value NapiAudioCapturer::GetCurrentInputDevices(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret = napiAudioCapturer->audioCapturer_->GetCurrentInputDevices(deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetCurrentInputDevices failure!");

    NapiParamUtils::SetValueDeviceInfo(env, deviceInfo, result);
    return result;
}

napi_value NapiAudioCapturer::GetCurrentAudioCapturerChangeInfo(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    AudioCapturerChangeInfo capturerInfo;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetCurrentCapturerChangeInfo(capturerInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetCurrentCapturerChangeInfo failure!");

    NapiParamUtils::SetAudioCapturerChangeInfoDescriptors(env, capturerInfo, result);
    return result;
}

napi_value NapiAudioCapturer::GetCurrentMicrophones(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    vector<sptr<MicrophoneDescriptor>> micDescs;
    micDescs = napiAudioCapturer->audioCapturer_->GetCurrentMicrophones();

    NapiParamUtils::SetMicrophoneDescriptors(env, micDescs, result);
    return result;
}

napi_value NapiAudioCapturer::GetState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    if (argc > PARAM0) {
        NapiAudioError::ThrowError(env, NAPI_ERR_INPUT_INVALID);
        napi_get_undefined(env, &result);
        return result;
    }

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    int32_t capturerState = napiAudioCapturer->audioCapturer_->GetStatus();

    napi_status status = NapiParamUtils::SetValueInt32(env, capturerState, result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "SetValueInt32 capturerState fail");
    return result;
}

napi_value NapiAudioCapturer::GetOverflowCount(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        context->overflowCount = napiAudioCapturer->audioCapturer_->GetOverflowCount();
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetValueUInt32(env, context->overflowCount, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetOverflowCount", executor, complete);
}

napi_value NapiAudioCapturer::SetWillMuteWhenInterrupted(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("failed: no memory");
        NapiAudioError::ThrowError(env, "SetWillMuteWhenInterrupted failed : no memory", NAPI_ERR_ILLEGAL_STATE);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments",
            NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->muteWhenInterrupted, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get muteWhenInterrupted failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "AudioCapturer state is error.");
        CHECK_AND_RETURN_LOG(napiAudioCapturer->audioCapturer_ != nullptr, "audioCapturer_ is nullptr");
        context->intValue = napiAudioCapturer->audioCapturer_->SetInterruptStrategy(context->muteWhenInterrupted ?
                InterruptStrategy::MUTE : InterruptStrategy::DEFAULT);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetWillMuteWhenInterrupted failed",
            NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetWillMuteWhenInterrupted", executor, complete);
}

napi_value NapiAudioCapturer::SetMuteHint(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("failed: no memory");
        NapiAudioError::ThrowError(env, "SetMuteHint failed : no memory", NAPI_ERR_ILLEGAL_STATE);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value *argv) {
        NAPI_CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INPUT_INVALID);
        context->status = NapiParamUtils::GetValueBoolean(env, context->muteHint, argv[PARAM0]);
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get muteHint failed",
            NAPI_ERR_INPUT_INVALID);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "AudioCapturer state is error.");
        CHECK_AND_RETURN_LOG(napiAudioCapturer->audioCapturer_ != nullptr, "audioCapturer_ is nullptr");
        context->intValue = napiAudioCapturer->audioCapturer_->SetMuteHint(context->muteHint);
        if (context->intValue == ERROR_ILLEGAL_STATE || context->intValue == ERR_ILLEGAL_STATE) {
            context->SignError(NAPI_ERR_ILLEGAL_STATE);
            return;
        }
        NAPI_CHECK_ARGS_RETURN_VOID(context, context->intValue == SUCCESS, "SetMuteHint failed", NAPI_ERR_SYSTEM);
    };

    auto complete = [env](napi_value &output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetMuteHint", executor, complete);
}

napi_value NapiAudioCapturer::SetNoiseReductionMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_ONE;
    napi_value args[ARGS_ONE] = {};

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, args);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "invalid arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args[PARAM0], &valueType);
    CHECK_AND_RETURN_RET_LOG(valueType == napi_number, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of noiseReductionMode must be number"),
        "invalid valueType");

    int32_t noiseReductionMode = 0;
    NapiParamUtils::GetValueInt32(env, noiseReductionMode, args[PARAM0]);
    CHECK_AND_RETURN_RET_LOG(NapiAudioEnum::IsLegalInputArgumentNoiseReductionMode(noiseReductionMode),
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of noiseReductionMode must be enum NoiseReductionMode"),
        "get noiseReductionMode failed");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    int32_t ret = napiAudioCapturer->audioCapturer_->SetNoiseReductionMode(
        static_cast<NoiseReductionMode>(noiseReductionMode));
    if (ret == ERR_INVALID_PARAM || ret == ERROR_INVALID_PARAM) {
        return NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
        "SetNoiseReductionMode failed");
    return NapiParamUtils::GetUndefinedValue(env);
}

napi_value NapiAudioCapturer::GetNoiseReductionMode(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID), "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    NoiseReductionMode noiseReductionMode = NOISE_REDUCTION_MODE_FIDELITY;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetNoiseReductionMode(noiseReductionMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
        "GetNoiseReductionMode failed");

    NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(noiseReductionMode), result);
    return result;
}

napi_value NapiAudioCapturer::GetSupportedNoiseReductionModes(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID), "invalid arguments");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    std::vector<NoiseReductionMode> noiseReductionModes;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetSupportedNoiseReductionModes(noiseReductionModes);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
        "GetSupportedNoiseReductionModes failed");

    napi_status status = napi_create_array_with_length(env, noiseReductionModes.size(), &result);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "napi_create_array_with_length failed");
    for (size_t i = 0; i < noiseReductionModes.size(); ++i) {
        napi_value jsMode = nullptr;
        status = NapiParamUtils::SetValueInt32(env, static_cast<int32_t>(noiseReductionModes[i]), jsMode);
        CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
            "SetValueInt32 failed");
        status = napi_set_element(env, result, i, jsMode);
        CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
            "napi_set_element failed");
    }
    return result;
}

napi_value NapiAudioCapturer::RequestPlaybackCaptureStart(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = { nullptr };
    napi_value self = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &self, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && self != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, "napi_get_cb_info failed", NAPI_ERR_INPUT_INVALID),
        "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE,
        NapiAudioError::ThrowErrorAndReturn(env, "invalid arguments count", NAPI_ERR_INPUT_INVALID),
        "invalid arguments count");

    napi_valuetype type = napi_undefined;
    status = napi_typeof(env, argv[PARAM0], &type);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && type == napi_function,
        NapiAudioError::ThrowErrorAndReturn(env, "callback must be a function", NAPI_ERR_INPUT_INVALID),
        "callback must be a function");

    NapiAudioCapturer *napiAudioCapturer = nullptr;
    status = napi_unwrap(env, self, reinterpret_cast<void **>(&napiAudioCapturer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && napiAudioCapturer != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, "napi_unwrap failed", NAPI_ERR_SYSTEM), "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, "audioCapturer_ is nullptr", NAPI_ERR_SYSTEM),
        "audioCapturer_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->sourceType_ == SOURCE_TYPE_PLAYBACK_CAPTURE &&
        napiAudioCapturer->isModernInnerCapturer_,
        NapiAudioError::ThrowErrorAndReturn(env,
            "not a playback capture stream created by playbackCaptureMode", NAPI_ERR_ILLEGAL_STATE),
        "not a playback capture stream created by playbackCaptureMode");

    PlaybackCaptureStartAsyncContext *context = nullptr;
    int32_t ret = CreatePlaybackCaptureStartContext(env, self, argv[PARAM0], napiAudioCapturer, &context);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
        NapiAudioError::ThrowErrorAndReturn(env, "create context failed", ret), "create context failed");
    ret = EnqueuePlaybackCaptureStartWork(env, context);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
        (ReleasePlaybackCaptureStartAsyncContext(env, context),
            NapiAudioError::ThrowErrorAndReturn(env, "queue async work failed", ret)),
        "queue async work failed");
    return NapiParamUtils::GetUndefinedValue(env);
}

napi_value NapiAudioCapturer::GetOverflowCountSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    uint32_t overflowCount = napiAudioCapturer->audioCapturer_->GetOverflowCount();

    NapiParamUtils::SetValueUInt32(env, overflowCount, result);
    return result;
}

napi_value NapiAudioCapturer::GetAudioTimestampInfo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<AudioCapturerAsyncContext>();
    if (context == nullptr) {
        NapiAudioError::ThrowError(env, NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }
    context->GetCbInfo(env, info);

    auto executor = [context]() {
        CHECK_AND_RETURN_LOG(CheckContextStatus(context), "context object state is error.");
        auto obj = reinterpret_cast<NapiAudioCapturer*>(context->native);
        ObjectRefMap objectGuard(obj);
        auto *napiAudioCapturer = objectGuard.GetPtr();
        CHECK_AND_RETURN_LOG(CheckAudioCapturerStatus(napiAudioCapturer, context),
            "context object state is error.");
        int32_t ret = napiAudioCapturer->audioCapturer_->GetAudioTimestampInfo(context->timeStamp,
            Timestamp::Timestampbase::MONOTONIC);
        if (ret != SUCCESS) {
            context->SignError(NAPI_ERR_SYSTEM);
        }
    };

    auto complete = [env, context](napi_value &output) {
        NapiParamUtils::SetTimeStampInfo(env, context->timeStamp, output);
    };

    return NapiAsyncWork::Enqueue(env, context, "GetAudioTimestampInfo", executor, complete);
}

napi_value NapiAudioCapturer::GetAudioTimestampInfoSync(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = PARAM0;

    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, nullptr);
    CHECK_AND_RETURN_RET_LOG(argc == PARAM0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID), "invaild param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");

    Timestamp timeStamp;
    int32_t ret = napiAudioCapturer->audioCapturer_->GetAudioTimestampInfo(timeStamp,
        Timestamp::Timestampbase::MONOTONIC);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "GetAudioTimestampInfo failure!");
    NapiParamUtils::SetTimeStampInfo(env, timeStamp, result);
    return result;
}

napi_value NapiAudioCapturer::On(napi_env env, napi_callback_info info)
{
    const size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_THREE;

    napi_value argv[requireArgc + PARAM1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc >= requireArgc, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argc requeset failed");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"),
        "eventType invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("NapiAudioCapturer: On callbackName: %{public}s", callbackName.c_str());

    napi_valuetype handler = napi_undefined;
    if (argc == requireArgc) {
        napi_typeof(env, argv[PARAM1], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function"), "handler invaild");
    } else {
        napi_valuetype paramArg1 = napi_undefined;
        napi_typeof(env, argv[PARAM1], &paramArg1);
        napi_valuetype expectedValType = napi_number;  // Default. Reset it with 'callbackName' if check, if required.
        CHECK_AND_RETURN_RET_LOG(paramArg1 == expectedValType,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of frame must be number"), "paramArg1 invaild");

        napi_typeof(env, argv[PARAM2], &handler);
        CHECK_AND_RETURN_RET_LOG(handler == napi_function,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
            "incorrect parameter types: The type of callback must be function"), "handler invaild");
    }

    return RegisterCallback(env, jsThis, argv, callbackName);
}

napi_value NapiAudioCapturer::OnReadMicInData(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argc request failed");

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, argv[PARAM0], &handler);
    CHECK_AND_RETURN_RET_LOG(handler == napi_function,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of callback must be function"), "handler invalid");

    napi_value cbArgs[ARGS_TWO] = { nullptr, nullptr };
    cbArgs[PARAM1] = argv[PARAM0];
    return RegisterCallback(env, jsThis, cbArgs, READ_MICIN_DATA_CALLBACK_NAME);
}

napi_value NapiAudioCapturer::RegisterCallback(napi_env env, napi_value jsThis,
    napi_value *argv, const std::string &cbName)
{
    NapiAudioCapturer *napiCapturer = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiCapturer));

    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiCapturer->audioCapturer_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "audioCapturer_ is nullptr");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    if (!cbName.compare(STATE_CHANGE_CALLBACK_NAME) ||
        !cbName.compare(AUDIO_INTERRUPT_CALLBACK_NAME)) {
        result = RegisterCapturerCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(MARK_REACH_CALLBACK_NAME)) {
        result = RegisterPositionCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(PERIOD_REACH_CALLBACK_NAME)) {
        result = RegisterPeriodPositionCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(INPUTDEVICE_CHANGE_CALLBACK_NAME)) {
        RegisterAudioCapturerDeviceChangeCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(AUDIO_CAPTURER_CHANGE_CALLBACK_NAME)) {
        RegisterAudioCapturerInfoChangeCallback(env, argv, cbName, napiCapturer);
    } else if (!cbName.compare(READ_DATA_CALLBACK_NAME) || !cbName.compare(READ_MICIN_DATA_CALLBACK_NAME)) {
        RegisterCapturerReadDataCallback(env, argv, cbName, napiCapturer);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"), "audioCapturer_ is nullptr");
    }

    return result;
}

napi_value NapiAudioCapturer::RegisterCapturerCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_RET_LOG(napiCapturer->callbackNapi_ != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "napiCapturer is nullptr");

    std::shared_ptr<NapiAudioCapturerCallback> cb =
        std::static_pointer_cast<NapiAudioCapturerCallback>(napiCapturer->callbackNapi_);
    cb->SaveCallbackReference(cbName, argv[PARAM1]);
    if (cbName == INTERRUPT_CALLBACK_NAME || cbName == AUDIO_INTERRUPT_CALLBACK_NAME) {
        if (!cb->GetInterruptTsfnFlag()) {
            cb->CreateInterruptTsfn(env);
        }
    } else if (cbName == STATE_CHANGE_CALLBACK_NAME) {
        if (!cb->GetStateChangeTsfnFlag()) {
            cb->CreateStateChangeTsfn(env);
        }
    }

    if (!cbName.compare(STATE_CHANGE_CALLBACK_NAME)) {
        CapturerState state = napiCapturer->audioCapturer_->GetStatus();
        if (state == CAPTURER_PREPARED) {
            napiCapturer->callbackNapi_->OnStateChange(state);
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioCapturer::RegisterPositionCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    int64_t markPosition = 0;
    NapiParamUtils::GetValueInt64(env, markPosition, argv[PARAM1]);

    AUDIO_INFO_LOG("NapiAudioCapturer:RegisterPositionCallback start!");
    if (markPosition > 0) {
        napiCapturer->positionCbNapi_ = std::make_shared<NapiCapturerPositionCallback>(env);
        CHECK_AND_RETURN_RET_LOG(napiCapturer->positionCbNapi_ != nullptr,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "positionCbNapi_ is nullptr");
        int32_t ret = napiCapturer->audioCapturer_->SetCapturerPositionCallback(markPosition,
            napiCapturer->positionCbNapi_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
            NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
            "SetCapturerPositionCallback failed");

        std::shared_ptr<NapiCapturerPositionCallback> cb =
            std::static_pointer_cast<NapiCapturerPositionCallback>(napiCapturer->positionCbNapi_);
        cb->SaveCallbackReference(cbName, argv[PARAM2]);
        if (!cb->GetCapturePositionFlag()) {
            cb->CreateCapturePositionTsfn(env);
        }
    } else {
        AUDIO_ERR_LOG("NapiAudioCapturer: Mark Position value not supported!!");
        CHECK_AND_RETURN_RET_LOG(false, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of frame is not supported"), "invailed callback");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiAudioCapturer::RegisterPeriodPositionCallback(napi_env env, napi_value *argv, const std::string &cbName,
    NapiAudioCapturer *napiCapturer)
{
    int64_t frameCount = 0;
    napi_get_value_int64(env, argv[PARAM1], &frameCount);

    CHECK_AND_RETURN_RET_LOG(frameCount > 0, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "parameter verification failed: The param of frame is not supported"), "frameCount value not supported");

    CHECK_AND_RETURN_RET_LOG(napiCapturer->periodPositionCbNapi_ == nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE),
        "periodReach already subscribed.");

    std::shared_ptr<NapiCapturerPeriodPositionCallback> cb = std::make_shared<NapiCapturerPeriodPositionCallback>(env);
    CHECK_AND_RETURN_RET_LOG(cb != nullptr,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_NO_MEMORY), "periodPositionCbNapi_ id nullptr");
    napiCapturer->periodPositionCbNapi_ = cb;

    int32_t ret = napiCapturer->audioCapturer_->SetCapturerPeriodPositionCallback(frameCount, cb);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_SYSTEM),
        "SetCapturerPeriodPositionCallback failed");

    cb->SaveCallbackReference(cbName, argv[PARAM2]);
    cb->CreatePeriodPositionTsfn(env);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

void NapiAudioCapturer::RegisterAudioCapturerDeviceChangeCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    std::shared_ptr<NapiAudioCapturerDeviceChangeCallback> cb = nullptr;
    GetCapturerNapiCallback(argv[PARAM1], cbName, napiCapturer->audioCapturerCallbacks_, &cb);
    CHECK_AND_RETURN_LOG(cb == nullptr, "Do not register same capturer device callback!");

    cb = std::make_shared<NapiAudioCapturerDeviceChangeCallback>(env);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(cbName, argv[PARAM1]);
    cb->CreateCaptureDeviceChangeTsfn(env);
    int32_t ret =
        napiCapturer->audioCapturer_->SetAudioCapturerDeviceChangeCallback(cb);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Registering of capturer device change callback failed");

    napiCapturer->audioCapturerCallbacks_.push_back(cb);

    AUDIO_DEBUG_LOG("RegisterAudioCapturerDeviceChangeCallback is successful");
}

void NapiAudioCapturer::RegisterAudioCapturerInfoChangeCallback(napi_env env, napi_value *argv,
    const std::string &cbName, NapiAudioCapturer *napiCapturer)
{
    std::shared_ptr<NapiAudioCapturerInfoChangeCallback> cb = nullptr;
    GetCapturerNapiCallback(argv[PARAM1], cbName, napiCapturer->audioCapturerCallbacks_, &cb);
    CHECK_AND_RETURN_LOG(cb == nullptr, "Do not register same capturer info change callback!");
    cb = std::make_shared<NapiAudioCapturerInfoChangeCallback>(env);

    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    cb->SaveCallbackReference(cbName, argv[PARAM1]);
    cb->CreateCaptureInfoChangeTsfn(env);
    int32_t ret =
        napiCapturer->audioCapturer_->SetAudioCapturerInfoChangeCallback(cb);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Registering of capturer info change callback failed");

    napiCapturer->audioCapturerCallbacks_.push_back(cb);

    AUDIO_DEBUG_LOG("RegisterAudioCapturerInfoChangeCallback is successful");
}

void NapiAudioCapturer::RegisterCapturerReadDataCallback(napi_env env, napi_value *argv, const std::string &cbName,
    NapiAudioCapturer *napiCapturer)
{
    if (napiCapturer->capturerReadDataCallbackNapi_ == nullptr) {
        napiCapturer->capturerReadDataCallbackNapi_ = std::make_shared<NapiCapturerReadDataCallback>(env, napiCapturer);
        napiCapturer->audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
        CHECK_AND_RETURN_LOG(napiCapturer->capturerReadDataCallbackNapi_ != nullptr, "readDataNapi_ is nullptr");
        int32_t ret = napiCapturer->audioCapturer_->
            SetCapturerReadCallback(napiCapturer->capturerReadDataCallbackNapi_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "SetCapturerCallback failed");
        std::shared_ptr<NapiCapturerReadDataCallback> cb =
            std::static_pointer_cast<NapiCapturerReadDataCallback>(napiCapturer->capturerReadDataCallbackNapi_);
        cb->CreateReadDataTsfn(env);
    }

    std::shared_ptr<NapiCapturerReadDataCallback> cb =
        std::static_pointer_cast<NapiCapturerReadDataCallback>(napiCapturer->capturerReadDataCallbackNapi_);
    CHECK_AND_RETURN_LOG(!cb->HasCallbackReference(cbName), "%{public}s already subscribed.", cbName.c_str());
    cb->AddCallbackReference(cbName, argv[PARAM1]);

#ifndef IOS_PLATFORM
    if ((!firstRegReadCbCalled_.exchange(true, std::memory_order_relaxed)) &&
        (getpid() == gettid())) {
        AudioCapturerInfo capturerInfo = {};
        napiCapturer->audioCapturer_->GetCapturerInfo(capturerInfo);
        napiCapturer->audioCapturer_->SetInMainThreadState(true);
        int32_t appUid = static_cast<int32_t>(getuid());
        NapiDfxUtils::ReportAudioMainThreadEvent(appUid, NapiDfxUtils::SteamDirection::capture,
            capturerInfo.sourceType, NapiDfxUtils::MainThreadCallFunc::readCb);
    }
#endif

    AUDIO_INFO_LOG("Register Callback is successful");
}

napi_value NapiAudioCapturer::Off(napi_env env, napi_callback_info info)
{
    const size_t minArgCount = ARGS_ONE;
    size_t argc = ARGS_TWO;

    napi_value argv[minArgCount + 1] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc >= minArgCount, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argc invaild");

    napi_valuetype eventType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &eventType);
    CHECK_AND_RETURN_RET_LOG(eventType == napi_string, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "incorrect parameter types: The type of eventType must be string"),
        "eventType invaild");

    std::string callbackName = NapiParamUtils::GetStringArgument(env, argv[PARAM0]);
    AUDIO_DEBUG_LOG("NapiAudioCapturer: Off callbackName: %{public}s", callbackName.c_str());

    return UnregisterCallback(env, jsThis, argc, argv, callbackName);
}

napi_value NapiAudioCapturer::OffReadMicInData(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &jsThis, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_get_cb_info failed");
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_ONE, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INPUT_INVALID, "mandatory parameters are left unspecified"), "argc request failed");

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, argv[PARAM0], &handler);
    CHECK_AND_RETURN_RET_LOG(handler == napi_function,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INPUT_INVALID,
        "incorrect parameter types: The type of callback must be function"), "handler invalid");

    napi_value cbArgs[ARGS_TWO] = { nullptr, nullptr };
    cbArgs[PARAM1] = argv[PARAM0];
    return UnregisterCallback(env, jsThis, ARGS_TWO, cbArgs, READ_MICIN_DATA_CALLBACK_NAME);
}

napi_value NapiAudioCapturer::UnregisterCallback(napi_env env, napi_value jsThis, size_t argc, napi_value *argv,
    const std::string &cbName)
{
    NapiAudioCapturer *napiCapturer = nullptr;
    napi_status status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&napiCapturer));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_SYSTEM), "napi_unwrap failed");
    CHECK_AND_RETURN_RET_LOG(napiCapturer != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "napiCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiCapturer->audioCapturer_ != nullptr, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_NO_MEMORY), "audioCapturer_ is nullptr");

    if (!cbName.compare(MARK_REACH_CALLBACK_NAME)) {
        UnregisterCapturerPositionCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(PERIOD_REACH_CALLBACK_NAME)) {
        UnregisterCapturerPeriodPositionCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(STATE_CHANGE_CALLBACK_NAME) ||
        !cbName.compare(AUDIO_INTERRUPT_CALLBACK_NAME)) {
        UnregisterCapturerCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(INPUTDEVICE_CHANGE_CALLBACK_NAME)) {
        UnregisterAudioCapturerDeviceChangeCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(AUDIO_CAPTURER_CHANGE_CALLBACK_NAME)) {
        UnregisterAudioCapturerInfoChangeCallback(env, argc, cbName, argv, napiCapturer);
    } else if (!cbName.compare(READ_DATA_CALLBACK_NAME) || !cbName.compare(READ_MICIN_DATA_CALLBACK_NAME)) {
        UnregisterCapturerReadDataCallback(env, argc, cbName, argv, napiCapturer);
    } else {
        bool unknownCallback = true;
        CHECK_AND_RETURN_RET_LOG(!unknownCallback, NapiAudioError::ThrowErrorAndReturn(env,
            NAPI_ERR_INVALID_PARAM,
            "parameter verification failed: The param of type is not supported"),
            "NAPI_ERR_UNSUPPORTED cbName");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

void NapiAudioCapturer::UnregisterCapturerPeriodPositionCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_LOG(napiCapturer->periodPositionCbNapi_ != nullptr, "capturerCallbackNapi is nullptr");

    std::shared_ptr<NapiCapturerPeriodPositionCallback> cb =
        std::static_pointer_cast<NapiCapturerPeriodPositionCallback>(napiCapturer->periodPositionCbNapi_);
    std::function<int32_t(std::shared_ptr<NapiCapturerPeriodPositionCallback> callbackPtr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiCapturerPeriodPositionCallback> callbackPtr, napi_value callback) {
            napiCapturer->audioCapturer_->UnsetCapturerPeriodPositionCallback();
            napiCapturer->periodPositionCbNapi_ = nullptr;
            return SUCCESS;
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
    AUDIO_INFO_LOG("UnregisterCapturerPeriodPositionCallback is successful");
}

void NapiAudioCapturer::UnregisterCapturerPositionCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_LOG(napiCapturer->positionCbNapi_ != nullptr, "capturerCallbackNapi is nullptr");

    std::shared_ptr<NapiCapturerPositionCallback> cb =
        std::static_pointer_cast<NapiCapturerPositionCallback>(napiCapturer->positionCbNapi_);
    std::function<int32_t(std::shared_ptr<NapiCapturerPositionCallback> callbackPtr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiCapturerPositionCallback> callbackPtr, napi_value callback) {
            napiCapturer->audioCapturer_->UnsetCapturerPositionCallback();
            napiCapturer->positionCbNapi_ = nullptr;
            return SUCCESS;
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb, removeFunction);
    AUDIO_INFO_LOG("UnregisterCapturerPositionCallback is successful");
}

void NapiAudioCapturer::UnregisterCapturerCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    CHECK_AND_RETURN_LOG(napiCapturer->callbackNapi_ != nullptr, "capturerCallbackNapi is nullptr");
    std::shared_ptr<NapiAudioCapturerCallback> cb =
        std::static_pointer_cast<NapiAudioCapturerCallback>(napiCapturer->callbackNapi_);
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerSingletonCallbackTemplate(env, callback, cbName, cb);
    AUDIO_INFO_LOG("Unregister CapturerCallback is successful");
}

void NapiAudioCapturer::UnregisterAudioCapturerDeviceChangeCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    std::function<int32_t(std::shared_ptr<NapiAudioCapturerDeviceChangeCallback> callbackptr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiAudioCapturerDeviceChangeCallback> callbackPtr, napi_value callback) {
            return napiCapturer->audioCapturer_->RemoveAudioCapturerDeviceChangeCallback(callbackPtr);
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerCallbackTemplate(env, callback, napiCapturer, cbName, removeFunction);

    AUDIO_INFO_LOG("UnregisterCapturerDeviceChangeCallback is successful");
}

void NapiAudioCapturer::UnregisterAudioCapturerInfoChangeCallback(napi_env env, size_t argc,
    const std::string &cbName, napi_value *argv, NapiAudioCapturer *napiCapturer)
{
    std::function<int32_t(std::shared_ptr<NapiAudioCapturerInfoChangeCallback> callbackPtr,
        napi_value callback)> removeFunction =
        [&napiCapturer] (std::shared_ptr<NapiAudioCapturerInfoChangeCallback> callbackPtr, napi_value callback) {
            return napiCapturer->audioCapturer_->RemoveAudioCapturerInfoChangeCallback(callbackPtr);
        };
    auto callback = GetCallback(argc, argv);
    UnregisterAudioCapturerCallbackTemplate(env, callback, napiCapturer, cbName, removeFunction);
    AUDIO_DEBUG_LOG("UnregisterAudioCapturerInfoChangeCallback is successful");
}

void NapiAudioCapturer::UnregisterCapturerReadDataCallback(napi_env env, size_t argc, const std::string &cbName,
    napi_value *argv,
    NapiAudioCapturer *napiCapturer)
{
    napi_value callback = nullptr;

    if (argc == ARGS_TWO) {
        callback = argv[PARAM1];
    }
    CHECK_AND_RETURN_LOG(napiCapturer->capturerReadDataCallbackNapi_ != nullptr,
        "napiCapturerReadDataCallback is nullptr, return");

    std::shared_ptr<NapiCapturerReadDataCallback> cb =
        std::static_pointer_cast<NapiCapturerReadDataCallback>(napiCapturer->capturerReadDataCallbackNapi_);
    cb->RemoveCallbackReference(env, callback, cbName);
    if (!cb->HasAnyCallbackReference()) {
        napiCapturer->capturerReadDataCallbackNapi_ = nullptr;
    }

    AUDIO_INFO_LOG("Unregister Callback is successful");
}

napi_value NapiAudioCapturer::SetIndependentAudioSessionStrategy(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {};
    auto *napiAudioCapturer = GetParamWithSync(env, info, argc, argv);
    CHECK_AND_RETURN_RET_LOG(argc == ARGS_TWO, NapiAudioError::ThrowErrorAndReturn(env,
        NAPI_ERR_INVALID_PARAM, "mandatory parameters are left unspecified"), "argcCount invaild");

    AudioSessionStrategy audioSessionStrategy;
    napi_status status = NapiParamUtils::GetAudioSessionStrategy(env, audioSessionStrategy, argv[PARAM0]);
    CHECK_AND_RETURN_RET_LOG((status == napi_ok), NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "param1 verification failed: audioSessionStrategy wrong value"),
        "valueType invalid");

    uint32_t behavior;
    status = NapiParamUtils::GetValueUInt32(env, behavior, argv[PARAM1]);
    CHECK_AND_RETURN_RET_LOG((status == napi_ok), NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "param2 verification failed: behavior wrong value"),
        "valueType invalid");
    CHECK_AND_RETURN_RET_LOG(
        (behavior & ~static_cast<uint32_t>(AudioSessionBehaviorFlags::MASK_VALID_BEHAVIOR)) == 0,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_INVALID_PARAM,
        "Parameter verification failed. behavior param is out of range"), "invaild behavior param");

    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer != nullptr, result, "napiAudioCapturer is nullptr");
    CHECK_AND_RETURN_RET_LOG(napiAudioCapturer->audioCapturer_ != nullptr, result, "audioCapturer_ is nullptr");
    int32_t ret =
        napiAudioCapturer->audioCapturer_->SetIndependentAudioSessionStrategy(audioSessionStrategy, behavior);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS,
        NapiAudioError::ThrowErrorAndReturn(env, NAPI_ERR_ILLEGAL_STATE), "SetIndependentAudioSessionStrategy Failed");
    return result;
}
} // namespace AudioStandard
} // namespace OHOS
