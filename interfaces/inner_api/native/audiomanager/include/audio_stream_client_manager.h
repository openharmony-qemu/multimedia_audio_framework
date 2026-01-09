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

#ifndef AUDIO_STREAM_CLIENT_MANAGER_H
#define AUDIO_STREAM_CLIENT_MANAGER_H

#include "audio_stream_types.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
class AudioStreamClientManager {
public:
    static AudioStreamClientManager &GetInstance();

    /**
    * @brief set stream volume by sessionId.
    *
    * @param sessionId stream sessionId.
    * @param volume return stream volume.
    * @return Returns {@link SUCCESS} if the operation is successfully.
    * @test
    */
    int32_t GetVolumeBySessionId(const uint32_t &sessionId, float &volume);

    /**
     * @brief registers the renderer data transfer callback listener
     *
     * @param param {@link DataTransferMonitorParam}
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t RegisterRendererDataTransferCallback(const DataTransferMonitorParam &param,
        const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback);

    /**
     * @brief Unregisters the renderer data transfer callback listener
     *
     * @param param {@link DataTransferMonitorParam}
     * @return Returns {@link SUCCESS} if callback unregistration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t UnregisterRendererDataTransferCallback(
        const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback);
private:
    AudioStreamClientManager() = default;
    ~AudioStreamClientManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_CLIENT_MANAGER_H
