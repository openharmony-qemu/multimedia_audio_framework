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
#ifndef NAPI_AUDIO_DEBUGGING_MANAGER_H
#define NAPI_AUDIO_DEBUGGING_MANAGER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_debug_manager.h"

namespace OHOS {
namespace AudioStandard {
inline const std::string AUDIO_DEBUGGING_MGR_NAPI_CLASS_NAME = "AudioDebuggingManager";

/**
 * @brief NAPI wrapper for AudioDebuggingManager, providing audio runtime snapshot
 * functionality for debugging and troubleshooting.
 *
 * AudioDebuggingManager provides many functions for developer to get the
 * information about audio system runtime info.
 *
 * @since 26.0.0
 */
class NapiAudioDebuggingManager {
public:
    NapiAudioDebuggingManager();
    ~NapiAudioDebuggingManager();

    /**
     * @brief Initialize the NapiAudioDebuggingManager class and define JS properties.
     *
     * @param env NAPI environment handle.
     * @param exports JS exports object to attach the class constructor.
     * @return napi_value The class constructor.
     * @since 26.0.0
     */
    static napi_value Init(napi_env env, napi_value exports);

    /**
     * @brief Create a JS AudioDebuggingManager wrapper instance.
     *
     * @param env NAPI environment handle.
     * @return napi_value The created JS AudioDebuggingManager instance.
     * @since 26.0.0
     */
    static napi_value CreateDebuggingManagerWrapper(napi_env env);

private:
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    static napi_value Construct(napi_env env, napi_callback_info info);
    static NapiAudioDebuggingManager *GetParamWithSync(const napi_env &env, napi_callback_info info,
        size_t &argc, napi_value *args);

    /**
     * @brief Prints full audio runtime snapshot for target app.
     *
     * The snapshot will contain the stream, pipe, volume and device information.
     * Note that the information details and format may vary from different version,
     * it can only be used for manual debugging.
     *
     * @param env NAPI environment handle.
     * @param info NAPI callback info containing arguments.
     * @return napi_value Undefined.
     * @since 26.0.0
     */
    static napi_value PrintAppInfo(napi_env env, napi_callback_info info);

    /**
     * @brief Prints full audio runtime snapshot for target audio renderer instance.
     *
     * The snapshot will contain the stream, pipe, volume and device information.
     * Note that the information details and format may vary from different version,
     * it can only be used for manual debugging.
     *
     * @param env NAPI environment handle.
     * @param info NAPI callback info containing renderer and fd arguments.
     * @return napi_value Undefined.
     * @since 26.0.0
     */
    static napi_value PrintRendererInfo(napi_env env, napi_callback_info info);

    /**
     * @brief Prints full audio runtime snapshot for target audio capturer instance.
     *
     * The snapshot will contain the stream, pipe, volume and device information.
     * Note that the information details and format may vary from different version,
     * it can only be used for manual debugging.
     *
     * @param env NAPI environment handle.
     * @param info NAPI callback info containing capturer and fd arguments.
     * @return napi_value Undefined.
     * @since 26.0.0
     */
    static napi_value PrintCapturerInfo(napi_env env, napi_callback_info info);

    /**
     * @brief Prints full audio runtime snapshot for target audio loopback instance.
     *
     * The snapshot will contain the loopback status, device and effect information.
     * Note that the information details and format may vary from different version,
     * it can only be used for manual debugging.
     *
     * @param env NAPI environment handle.
     * @param info NAPI callback info containing loopback and fd arguments.
     * @return napi_value Undefined.
     * @since 26.0.0
     */
    static napi_value PrintLoopbackInfo(napi_env env, napi_callback_info info);

    /**
     * @brief Prints full audio runtime snapshot for target audio session manager instance.
     *
     * The snapshot will contain the session status, scene, strategy and device information.
     * Note that the information details and format may vary from different version,
     * it can only be used for manual debugging.
     *
     * @param env NAPI environment handle.
     * @param info NAPI callback info containing session and fd arguments.
     * @return napi_value Undefined.
     * @since 26.0.0
     */
    static napi_value PrintSessionInfo(napi_env env, napi_callback_info info);

    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_AUDIO_DEBUGGING_MANAGER_H
